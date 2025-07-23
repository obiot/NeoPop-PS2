/*
  system_ps2.c -- PS2 Specific function
  
  Copyright (C) 2004 Evilo

  This file is part of NeoPop-PS2, a NeoGeo Pocket emulator
  The author can be contacted at <evilo13_at_gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <loadfile.h>  
#include <iopcontrol.h>
#include <iopheap.h>
#include <libcdvd.h> 
#include <libmc.h> 
#include <fileio.h>  
#include <sbv_patches.h>

#include "system_ps2.h"
#include "cdrom/cdvd.h" 
#include "cdrom/cdvd_rpc.h"
#include "sound/sjpcm.h"

#include "gui/splash.h"



#include "data/ngpc_bkg.h" 
#include "data/ngpc_bkg_dbl.h"

#include "neopop_ps2.h"

int 	boot_mode;

//u16   *framebuffer;//[256*256]  __attribute__((aligned(64)));



extern int fps;

#define VERSION1 "NEOPOP/PS2 "
#define VERSION2_MAJOR 0
#define VERSION2_MINOR 1 

int whichdrawbuf = 0; 

extern unsigned vsync_freq;

//extern s16 *mxsndbuf2;

char 		path_prefix[128]  __attribute__((aligned(64))) = "cdrom0:\\"; 
char 		display_fps[12]  __attribute__((aligned(64)));

void	(*ps2_render_screen)(void); 

// default emulator settings
struct_neopopSettings neopopSettings  = 
{
		(VERSION2_MAJOR * 10 + VERSION2_MINOR),
		0,		// Filter: Nearest
		1,		// SOUND: ON
		0,		// FULLSCREEN: windowed
		0,		// showFPS OFF
		0,		// RFU
		0,		// SAVE: OFF
		83, 40,		// PAL x/y offset
		83, 25		// NTSC x/y offset
}; 

// default machine settings
struct_machine machine_def = {0,0,0,0,0,0,0,0,0}; // initialized later 

/*  ps2_init: Sets up a video mode, allocates memory
 */

void ps2_init() 
{
	
  SifInitRpc(0);
  printf("rpc init\n");
  
  //init fio
  fioInit();

  // load modules
  ps2_loadModules(S_TYPE);  
  
  // init CDROM
  CDVD_Init();
  // init MC
  mcInit(MC_TYPE_MC);
  
	 
}

// reboot the IOP
void ps2_rebootIOP()
{
	// IOP reboot
	SifInitRpc(0);
   	SifExitIopHeap();
   	SifLoadFileExit();
   	SifExitRpc();
   	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
   	//while (!SifIopSync()) ;
}


/*  setDisplay
 */

void ps2_setDisplay() 
{
  	
	DmaReset();

	if(pal_ntsc() == GS_PAL) // PAL
	{
		GS_InitGraph(GS_PAL,GS_NONINTERLACE);
		machine_def.vdph = 256;
		machine_def.snd_sample = (960 >> 1);
		machine_def.fps_rate = 50;
		machine_def.dispx = neopopSettings.dispXPAL;
		machine_def.dispy = neopopSettings.dispYPAL;
		vsync_freq = GETTIME_FREQ_PAL;
	} 
	else // NTSC
	{
		GS_InitGraph(GS_NTSC,GS_NONINTERLACE);
		machine_def.vdph = 224;
		machine_def.snd_sample = (800 >> 1);
		machine_def.fps_rate = 60;
		machine_def.dispx = neopopSettings.dispXNTSC;
		machine_def.dispy = neopopSettings.dispYNTSC;
		vsync_freq = GETTIME_FREQ_NTSC;
	}
	
	// beurk..
	machine_def.x1_offset = ((PS2_SCREEN_WIDTH - FULLSCREEN_DPW)>>1) << 4 ;
	machine_def.x2_offset = (PS2_SCREEN_WIDTH-((PS2_SCREEN_WIDTH-FULLSCREEN_DPW)>>1)) << 4; 
	
	machine_def.y1_offset = ((machine_def.vdph-FULLSCREEN_DPH)>>1) << 4 ;
	machine_def.y2_offset = (machine_def.vdph-((machine_def.vdph-FULLSCREEN_DPH)>>1)) << 4; 
	
	// set video mode
	GS_SetDispMode(machine_def.dispx,machine_def.dispy,320,machine_def.vdph); 
	
	// init pipe
	GS_SetEnv(320, machine_def.vdph, 0, 0x080000, GS_PSMCT32, 0x100000, GS_PSMZ32);
	install_VRstart_handler();
	createGfxPipe(&thegp, 0x080000);
	

	
	
	// and point to correct rendering code
	if (neopopSettings.fullscreen==0) // windowed
		ps2_render_screen=ps2_render_windowed; 
	else if (neopopSettings.fullscreen==1)
		ps2_render_screen=ps2_render_double; 
	else //(neopopSettings.fullscreen==2)
		ps2_render_screen=ps2_render_fullscreen; 
		
	ps2_system_uploadbackground(neopopSettings.fullscreen);
	
	// set default filter mode
	gp_setFilterMethod(neopopSettings.renderFilter);
		
	
	// Clear the screen (with ZBuffer Disabled)
	gp_disablezbuf(&thegp);
	gp_frect(&thegp,0,0,320<<4,machine_def.vdph<<4,0,GS_SET_RGBA(0,0,0,0x80));
	gp_enablezbuf(&thegp); 
	
	// display the loading screen
	splash_loading();

}

// init the path prefix varialbe
void setBootPath(int argc, char *argv[])
{
   // detect host : strip elf from path
   if (argc == 0)  // argc == 0 usually means naplink..
   {
        strcpy (path_prefix,"host:");
   }
   else if (argc>=1)
   {
       char *p;
        if ((p = strrchr(argv[0], '/'))!=NULL) {
	      snprintf(path_prefix, sizeof(path_prefix), "%s", argv[0]);
	      p = strrchr(path_prefix, '/');
	      if (p!=NULL)
	        p[1]='\0';
	    } else if ((p = strrchr(argv[0], '\\'))!=NULL) {
	      snprintf(path_prefix, sizeof(path_prefix), "%s", argv[0]);
	      p = strrchr(path_prefix, '\\');
	      if (p!=NULL)
	        p[1]='\0';
	    } else if ((p = strchr(argv[0], ':'))!=NULL) {
	      snprintf(path_prefix, sizeof(path_prefix), "%s", argv[0]);
	      p = strchr(path_prefix, ':');
	      if (p!=NULL)
	        p[1]='\0';
	    }
   } 
}



// return the device from which the emu is booted
int ps2_getBootDevice(char *path) 
{
   
   boot_mode = BOOT_HO;
	
   if (!strncmp(path_prefix, "cdrom", strlen("cdrom"))) {
        	printf("Booting from cd\n");
        	boot_mode = BOOT_CD;
   } else if(!strncmp(path_prefix, "mc", strlen("mc"))) {
        	printf("Booting from mc\n");
        	boot_mode = BOOT_MC;
   } else if(!strncmp(path_prefix, "host", strlen("host"))) {
	        printf("Booting from host\n");
	        boot_mode = BOOT_HO; 
   }  else if(!strncmp(path_prefix, "hdd", strlen("hdd"))) { //exist?
	        printf("Booting from hdd\n");
	        boot_mode = BOOT_HD;
   }  else if(!strncmp(path_prefix, "pfs", strlen("pfs"))) {
	        printf("Booting from hdd\n");
	        boot_mode = BOOT_HD;
   }	
   
   return boot_mode;
}

//upload background pictures for windowed/double mode
void ps2_system_uploadbackground(int fullscreen)
{
  if (fullscreen==0)
	//upload background pictures for "windowed" mode
	gp_uploadTexture(&thegp, BACK_TEX, 512, 0, 0, GS_PSMCT32, ngpc_bkg, ngpc_bkg_w, ngpc_bkg_h);
  else if (fullscreen==1)
  	//upload background pictures for "double" mode
	gp_uploadTexture(&thegp, BACK_TEX, 512, 0, 0, GS_PSMCT32, ngpc_bkg_dbl, ngpc_bkg_dbl_w, ngpc_bkg_dbl_h);
}

// render screen in fullscreen
void ps2_render_fullscreen()
{
 	
	//upload 16 bit framebuffer
 	gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT16, cfb, BUF_WIDTH, BUF_HEIDTH);
	gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT16, 0, 0, 0);


	gp_texrect(&thegp, 			// gfxpipe	
		   0,  0, // x1,y1
		   0,    0, 			// u1,v1
		   PS2_SCREEN_WIDTH<<4, machine_def.vdph<<4,	// x2 (320<<4) ,y2
		   (BUF_WIDTH)<<4, BUF_HEIDTH<<4, // u2 (320<<4), v2(224<<4)
		   10, 				// z
		   GS_SET_RGBA(255, 255, 255, 200) // color
		  );

	if (neopopSettings.showFPS)
	{
		sprintf(display_fps,"FPS %d/%d",fps,machine_def.fps_rate);
		textpixel(10,5,GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,display_fps);
	}		
	gp_hardflush(&thegp);

}

// render screen in double
void ps2_render_double()
{
     	
     	// set background texture
	gp_setTex(&thegp, BACK_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT32, 0, 0, 0);

	gp_texrect(&thegp, 			   // gfxpipe	
		   0,    0, 			   // x1,y1
		   0,    0, 			   // u1,v1
		   ngpc_bkg_dbl_w<<4, ngpc_bkg_dbl_h<<4,   // x2 (320<<4) ,y2
		   ngpc_bkg_dbl_w<<4, ngpc_bkg_dbl_h<<4,   // u2 (320<<4), v2(224<<4)
		   10, 				   // z
		   GS_SET_RGBA(255, 255, 255, 200) // color
		  );

	
	//upload 16 bit framebuffer
 	gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT16, cfb, BUF_WIDTH, BUF_HEIDTH);
	gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT16, 0, 0, 0);

	gp_texrect(&thegp, 			// gfxpipe	
		   53<<4,    10<<4, 		// x1,y1
		   0,    0, 			// u1,v1
		   (53+FULLSCREEN_DPW)<<4, (10+FULLSCREEN_DPH)<<4,		// x2 (320<<4) ,y2
		   BUF_WIDTH<<4, BUF_HEIDTH<<4, // u2 (320<<4), v2(224<<4)
		   11, 				// z
		   GS_SET_RGBA(255, 255, 255, 255) // color
		  );
	
	
	if (neopopSettings.showFPS)
	{
		sprintf(display_fps,"FPS %d/%d",fps,machine_def.fps_rate);
		textpixel(10,5,GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,display_fps);
	}		
	gp_hardflush(&thegp);
	
}

// render screen in original taste
void ps2_render_windowed()
{
     	
     	// set background texture
	gp_setTex(&thegp, BACK_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT32, 0, 0, 0);

	gp_texrect(&thegp, 			   // gfxpipe	
		   0,    0, 			   // x1,y1
		   0,    0, 			   // u1,v1
		   ngpc_bkg_w<<4, ngpc_bkg_h<<4,   // x2 (320<<4) ,y2
		   ngpc_bkg_w<<4, ngpc_bkg_h<<4,   // u2 (320<<4), v2(224<<4)
		   10, 				   // z
		   GS_SET_RGBA(255, 255, 255, 200) // color
		  );

	
	//upload 16 bit framebuffer
 	gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT16, cfb, BUF_WIDTH, BUF_HEIDTH);
	gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT16, 0, 0, 0);

	gp_texrect(&thegp, 			// gfxpipe	
		   78<<4,    46<<4, 		// x1,y1
		   0,    0, 			// u1,v1
		   (78+BUF_WIDTH)<<4, (46+BUF_HEIDTH)<<4,		// x2 (320<<4) ,y2
		   BUF_WIDTH<<4, BUF_HEIDTH<<4, // u2 (320<<4), v2(224<<4)
		   11, 				// z
		   GS_SET_RGBA(255, 255, 255, 255) // color
		  );
	
	
	if (neopopSettings.showFPS)
	{
		sprintf(display_fps,"FPS %d/%d",fps,machine_def.fps_rate);
		textpixel(10,5,GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,display_fps);
	}		
	gp_hardflush(&thegp);
	
}


void ps2_vsync()
{
	WaitForNextVRstart(1);
}

void ps2_switch_buffers()
{
	// Update drawing and display enviroments.
    	GS_SetCrtFB(whichdrawbuf);
    	whichdrawbuf ^= 1;
    	GS_SetDrawFB(whichdrawbuf); 
}


/*
 * loadModules()
 */
void ps2_loadModules(int module_type)
{
    int ret;
        
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    
            
    if (module_type == S_TYPE)    
    {
    	SifLoadModule("rom0:SIO2MAN", 0, NULL);
    	SifLoadModule("rom0:PADMAN", 0, NULL);
    	SifLoadModule("rom0:MCMAN", 0, NULL);  	
    	SifLoadModule("rom0:MCSERV", 0, NULL); 
    }   
    else // XModules : be sure to link with -lpadx 
    {
        SifLoadModule("rom0:XSIO2MAN", 0, NULL);
    	SifLoadModule("rom0:XPADMAN", 0, NULL);
    	SifLoadModule("rom0:XMCMAN", 0, NULL);
    	SifLoadModule("rom0:XMCSERV", 0, NULL);
    }   
    
    SifExecModuleBuffer(&cdvdIrx, size_cdvdIrx, 0, NULL, &ret);
    SifExecModuleBuffer(&isjpcmIrx, size_isjpcmIrx, 0, NULL, &ret);
    SifExecModuleBuffer(&iomanXIrx, size_iomanXIrx, 0, NULL, &ret);
    SifExecModuleBuffer(&fileXioIrx, size_fileXioIrx, 0, NULL, &ret); 
    
} 
