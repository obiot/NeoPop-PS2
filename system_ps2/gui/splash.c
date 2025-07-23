/*
  splash.c -- Splash screen
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
#include "../gs/gfxpipe.h"
#include "../gs/gs.h"
#include "../gs/hw.h"
#include "../neopop_ps2.h"
#include "splash.h"
#include "ps2print.h" 

#include "../data/splash.h"

int y_offset = 0;

// LOADING SCREEN
void splash_loading()
{
   
   // Clear the screen (with ZBuffer Disabled)
   gp_disablezbuf(&thegp);
   gp_frect(&thegp,0,0,320<<4,machine_def.vdph<<4,0,GS_SET_RGBA(0,0,0,0x80));
   gp_enablezbuf(&thegp); 
   
   if (machine_def.fps_rate == 50)
   	y_offset=16;
   else
   	y_offset=0;
   
	
   gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT32, splash, 320, 224);
	
   gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT32, 0, 0, 0);

   gp_texrect(&thegp, 				  // gfxpipe	
   	      0, y_offset<<4, 			  // x1,y1
	      0, 0, 				  // u1,v1
	      (splash_w)<<4, (splash_h+y_offset)<<4,// x2,y2
	      splash_w<<4, splash_h<<4,		  // u2,v2
	      10, 			          // z
	      GS_SET_RGBA(255, 255, 255, 200) // color
	      );

   // TOP TEXT
   gp_gouradrect(&thegp,(56)<<4, (22+y_offset)<<4,GS_SET_RGBA(153,153, 153, 100), (264)<<4, (42+y_offset)<<4, GS_SET_RGBA(153,153, 153, 100), 11);
   gp_linerect(&thegp,  (56)<<4, (22+y_offset)<<4, 			        (264)<<4, (42+y_offset)<<4, 12, GS_SET_RGBA(255, 255, 255, 128)); 
	
   //164
   textCpixel(0,320,28+y_offset, GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,TITLE_TXT);	

   // BOTTOM TEXT	
   gp_gouradrect(&thegp,(56)<<4, (178+y_offset)<<4,GS_SET_RGBA(153,153, 153, 100), (264)<<4, (198+y_offset)<<4, GS_SET_RGBA(153,153, 153, 100), 11);
   gp_linerect(&thegp,  (56)<<4, (178+y_offset)<<4, 			           (264)<<4, (198+y_offset)<<4, 12, GS_SET_RGBA(255, 255, 255, 128)); 
	
   //184
   textCpixel(0,320,184+y_offset, GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,"Loading...");
		

   gp_hardflush(&thegp);
	
   ps2_vsync();
	
   ps2_switch_buffers();

}

// TINY CREDIT SCREEN
void splash_credit()
{
   int cmpt=0; 
	
   // Clear the screen (with ZBuffer Disabled)
   gp_disablezbuf(&thegp);
   gp_frect(&thegp,0,0,320<<4,machine_def.vdph<<4,0,GS_SET_RGBA(0,0,0,0x80));
   gp_enablezbuf(&thegp); 
   
   if (machine_def.fps_rate == 50)
   	y_offset=16;
   else
   	y_offset=0;
   
   // upload background pictures
   gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT32, splash, 320, 224);
   
   while(1) 
   {
	cmpt++;
	if(cmpt>81)cmpt=0; 
	
	gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT32, 0, 0, 0);

   	gp_texrect(&thegp, 				  // gfxpipe	
   	           0, y_offset<<4, 			  // x1,y1
	           0, 0, 				  // u1,v1
	           (splash_w)<<4, (splash_h+y_offset)<<4, // x2,y2
	            splash_w<<4, splash_h<<4,		  // u2,v2
	           10, 			          	  // z
	           GS_SET_RGBA(255, 255, 255, 200) 	  // color
	           );
	
	// TOP TEXT
	gp_gouradrect(&thegp,(56)<<4, (22+y_offset)<<4,GS_SET_RGBA(153,153, 153, 100), (264)<<4, (42+y_offset)<<4, GS_SET_RGBA(153,153, 153, 100), 11);
        gp_linerect(&thegp,  (56)<<4, (22+y_offset)<<4, 			        (264)<<4, (42+y_offset)<<4, 12, GS_SET_RGBA(255, 255, 255, 128)); 
	
	//164
        textCpixel(0,320,28+y_offset, GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,TITLE_TXT);	
	
	
	// BOTTOM TEXT
        gp_gouradrect(&thegp,(56)<<4, (168+y_offset)<<4,GS_SET_RGBA(153,153, 153, 100), (264)<<4, (216+y_offset)<<4, GS_SET_RGBA(153,153, 153, 100), 11);
        gp_linerect(&thegp,  (56)<<4, (168+y_offset)<<4, 			        (264)<<4, (216+y_offset)<<4, 12, GS_SET_RGBA(255, 255, 255, 128)); 
	
        
        //184
        textCpixel(0,320,172+y_offset, GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,CREDIT1_TXT);
        textCpixel(0,320,184+y_offset, GS_SET_RGBA(255, 255, 255, 255)   ,0,0,12,CREDIT2_TXT);
        
		
	//210
	if(cmpt<31)
		textCpixel(0,320,204+y_offset,GS_SET_RGBA(255, 255, 255, 255)	     ,0,0,12," - Press X to continue - ");
        else if (cmpt<64)
        	textCpixel(0,320,204+y_offset,GS_SET_RGBA(255, 255, 255, (63 - cmpt)),0,0,12," - Press X to continue - "); 
	
   	// exit if cross button pressed
   	if (system_input_isButtonPressed (PAD_CROSS)) {  
   		ps2_vsync();
   		break; 
   	}
  
  	// else "flush" next frame
        gp_hardflush(&thegp);
	
        ps2_vsync();
	
   	ps2_switch_buffers();
   	
    
   
     } // end while  
}

