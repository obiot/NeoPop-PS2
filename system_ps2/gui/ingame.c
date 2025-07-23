/*
  ingame.c -- Ingame menu
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

#include <tamtypes.h>
#include <string.h>
#include <stdio.h>
#include <libpad.h>
#include <libmc.h>
#include "../cdrom/cdvd.h"
#include "../cdrom/cdvd_rpc.h"
#include "../sound/sjpcm.h"
#include "ingame.h"
#include "../gs/gfxpipe.h"
#include "../gs/gs.h"
#include "../gs/hw.h"
#include "../neopop_ps2.h"
#include "ps2print.h" 

extern int fps;
extern char display_fps[12]  __attribute__((aligned(64)));

char display_frameskip[16]  __attribute__((aligned(64)));

extern u32 ngpc_bkg_w, ngpc_bkg_h;
extern u32 ngpc_bkg_dbl_w, ngpc_bkg_dbl_h;

extern _u8 system_frameskip_key;

void IngameMenu()
{
	struct padButtonStatus pad1;
	int pad1_data = 0;
	int old_pad = 0;
	int new_pad;
	int ret;
	int selection = 0;
        static int ypos[8] = {59<<4,77<<4,95<<4,113<<4,131<<4,149<<4,167<<4,185<<4};
        int center_x,center_y;


	if (neopopSettings.soundOn)
	   SjPCM_Pause();
	//disableVSyncCallbacks();
	
	center_x = 0;
	center_y = 114;



	while(1) 
	{
		// draw last frame
		if (neopopSettings.fullscreen==0) // original
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
			
		}
		else if (neopopSettings.fullscreen==1) // double
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
	
		}
		else // fullscreen
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
			
		}	
		if (neopopSettings.showFPS)
		{
			sprintf(display_fps,"FPS %d/%d",fps,machine_def.fps_rate);
			textpixel(10,5,GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,display_fps);
		}		

	    	//gp_gouradrect(&thegp, 0, ((vdph-dph)>>1)<<4, GS_SET_RGBA(0, 0, 0, 128), 320<<4, (vdph-((vdph-dph)>>1))<<4, GS_SET_RGBA(0, 0, 0,128 ), 11);
    	    	
    	    	// Shade neocd display
	    	gp_frect(&thegp, 0, 0, 320<<4, 256<<4, 12, GS_SET_RGBA(0, 0, 0, 64));

		//gp_gouradrect(&thegp,(96-16)<<4,54<<4,GS_SET_RGBA(0x00, 0x00, 0x40, 128), (320-96+16)<<4,211<<4, GS_SET_RGBA(0x40,0x40, 0x80, 128), 13);
		gp_gouradrect(&thegp,(96-16)<<4,54<<4,GS_SET_RGBA(0x00,0x00, 0x20, 100), (320-96+16)<<4,211<<4, GS_SET_RGBA(0x00,0x00, 0x20, 100), 13);
		gp_linerect(&thegp, (95-16)<<4, 54<<4, (320-96+16)<<4, 211<<4, 14, GS_SET_RGBA(255, 255, 255, 128));
	
		TextOutC2(0<<4,320<<4,ypos[0]," - Resume - ",15);
		
		

		if (neopopSettings.fullscreen == 0)
		   TextOutC2(0<<4,320<<4,ypos[1],"Display : Original",15);
		else if (neopopSettings.fullscreen == 1)
		  TextOutC2(0<<4,320<<4,ypos[1],"Display : Double",15);
		else // (oswanSettings.fullscreen == 2)
		  TextOutC2(0<<4,320<<4,ypos[1],"Display : Fullscreen",15);
		
		if (neopopSettings.renderFilter==0)
			TextOutC2(0<<4,320<<4,ypos[2],"Filter : Nearest",15);	
		else 
			TextOutC2(0<<4,320<<4,ypos[2],"Filter : Linear",15);	
			
		if (neopopSettings.showFPS==0)			
	        	TextOutC2(0<<4,320<<4,ypos[3],"FPS Counter : Off",15);
		else
			TextOutC2(0<<4,320<<4,ypos[3],"FPS Counter : On",15);	

				
		if (system_frameskip_key==1)
		   TextOutC2(0<<4,320<<4,ypos[4],"Frameskip : Off",15);
		else
		   TextOutC2(0<<4,320<<4,ypos[4],"Frameskip : On",15);
		/*
		{
		   sprintf(display_frameskip,"Frameskip : %d", (system_frameskip_key -1));		
		   TextOutC2(0<<4,320<<4,ypos[4],display_frameskip,15);
		}
		*/
		
		
		if (neopopSettings.soundOn==0)			
	        	TextOutC2(0<<4,320<<4,ypos[5],"Sound : Off",15);
		else
			TextOutC2(0<<4,320<<4,ypos[5],"Sound : On",15);
		
				
		TextOutC2(0<<4,320<<4,ypos[6],"Reset emulation",15);
	
		TextOutC2(0<<4,320<<4,ypos[7],"- Return to Browser -",15);
			
		
	
		
		gp_frect(&thegp, (95-16)<<4, ypos[selection], (320-95+16)<<4, ypos[selection] + (16<<4), 16, GS_SET_RGBA(123, 255, 255, 40));

		gp_hardflush(&thegp);
		WaitForNextVRstart(1);
    		GS_SetCrtFB(whichdrawbuf);
	    	whichdrawbuf ^= 1;
	    	GS_SetDrawFB(whichdrawbuf);

		//if(padGetState(0, 0) == PAD_STATE_STABLE) 
		while (((ret=padGetState(0, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?
		//{
			padRead(0, 0, &pad1);
			//pad1_data = 0xffff ^ ((pad1.btns[0] << 8) | pad1.btns[1]);
			pad1_data = 0xffff ^ pad1.btns;

			if((pad1.mode >> 4) == 0x07) 
			{
				if(pad1.ljoy_v < 64) pad1_data |= PAD_UP;
				else if(pad1.ljoy_v > 192) pad1_data |= PAD_DOWN;
			}
		//}
		new_pad = pad1_data & ~old_pad;
  		old_pad = pad1_data;

		//if((pad1_data & PAD_L2)&&
    	   	//   (pad1_data & PAD_R2))   break; // quit menu
		
		if(pad1_data & PAD_SELECT) // Screen positioning
		{
             		
			if((pad1_data & PAD_UP) && machine_def.dispy) machine_def.dispy--;
			if(pad1_data & PAD_DOWN) machine_def.dispy++;
			if((pad1_data & PAD_LEFT) && machine_def.dispx) machine_def.dispx--;
			if(pad1_data & PAD_RIGHT) machine_def.dispx++;
			
			if (machine_def.vdph == 256) // PAL MODE
			{
				neopopSettings.dispXPAL=machine_def.dispx;
				neopopSettings.dispYPAL=machine_def.dispy;
			}	
			else
			{
				neopopSettings.dispXNTSC=machine_def.dispx;
				neopopSettings.dispYNTSC=machine_def.dispy;
			}
			GS_SetDispMode(machine_def.dispx,machine_def.dispy,320,machine_def.vdph);
			continue;
			
		}

     		if((new_pad & PAD_UP) && (selection > -1))
     		{
	     	  	if(selection>0)
           			selection--;
          		else selection=7; 
          		
        	}
        
		if((new_pad & PAD_DOWN) && (selection < 8))
		{
		  	if(selection<7)          
            			selection++;
          		else selection=0;   
          		
        	}


		if(new_pad & PAD_CROSS) 
		{
			if(selection == 0) break;
			
			if(selection == 1) // fullscreen
			{
				neopopSettings.fullscreen++;
				if (neopopSettings.fullscreen>2)
				  neopopSettings.fullscreen=0;
				
				switch (neopopSettings.fullscreen)	  
				{
					case 0 : // orignal
						 ps2_render_screen=ps2_render_windowed;
						 break;
						 
					case 1 : //double
						 ps2_render_screen=ps2_render_double; 
						 break;
					
					case 2 : //fullscreen
						 ps2_render_screen=ps2_render_fullscreen; 
						 break;
						 
					default : break; // we should never get here !
				
				}	
				// re-upload rigth background
				ps2_system_uploadbackground(neopopSettings.fullscreen);
				
				// if fullscreen, linear interpolation
				neopopSettings.renderFilter = (neopopSettings.fullscreen>0);
				gp_setFilterMethod(neopopSettings.renderFilter);
				/*
				if (neopopSettings.fullscreen)
					ps2_render_screen=ps2_render_fullscreen; 
				else
					ps2_render_screen=ps2_render_windowed; 
				*/
			
			}	
			
			if(selection == 2) // video filter
			{
			
				neopopSettings.renderFilter ^= 1;
				gp_setFilterMethod(neopopSettings.renderFilter);
			
			}	
			
			
			if(selection == 3) // fps counter
			{
				neopopSettings.showFPS ^= 1;
			}
						
			// frameskip
			if(selection == 4) 
			{
				system_frameskip_key++;
				if (system_frameskip_key>2) system_frameskip_key=1;
			}

			if(selection == 5)  // sound on/off
			{
				neopopSettings.soundOn ^= 1;
				have_sound = neopopSettings.soundOn;
			}

			if(selection == 6) // Soft reset
			{
	          		reset();
             			break;
			}
			
			if(selection == 7) // browser
			{
				 // exit emulation
				 // return to browser
				 do_exit=1;
				 break;
			}
		}
	}

	// Wait till X has stopped been pressed.
	while(1) 
	{
		while (((ret=padGetState(0, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?
  		//if(padGetState(0, 0) == PAD_STATE_STABLE) 
		//{
			padRead(0, 0, &pad1); // port, slot, buttons
			//pad1_data = 0xffff ^ ((pad1.btns[0] << 8) | pad1.btns[1]);
			pad1_data = 0xffff ^ pad1.btns;
		//}
		if(!(pad1_data & PAD_CROSS)) break;
	}
	
	// Clear the screen (with ZBuffer Disabled)
	gp_disablezbuf(&thegp);
	gp_frect(&thegp,0,0,320<<4,256<<4,0,GS_SET_RGBA(0,0,0,0x80));
	gp_enablezbuf(&thegp);
	
	//gp_setFilterMethod(oswanSettings.renderFilter);
	
	if (neopopSettings.soundOn)
		SjPCM_Play();
	
	// get out of here !
}


