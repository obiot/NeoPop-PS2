/*
  system_main.c -- main program
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



#include "neopop_ps2.h"
#include "system_ps2.h"

#include "gui/splash.h"

int frameskip_counter;

_u8 system_frameskip_key;
int do_exit = 0;
int have_sound;

int videoBufferAvailable=0;

int frame_counter = 0;


int fps = 0;
int rom_exist;
extern int hostlist; 


char ngp_rom_path[256] __attribute__((aligned(64)));


// needed by the core
void system_message(char *vaMessage, ...)
{
    /*
    va_list vl;

    va_start(vl, vaMessage);
    vprintf(vaMessage, vl);
    va_end(vl);
    printf("\n");
    */
    ; // empty function just to make the core source happy !
}



// vblank
void system_VBL(void)
{
 
   static unsigned lastime, mytime =0, elapsed, mycounter;
    
   lastime=mytime;
   mytime=timer_gettime();  
   elapsed=mytime-lastime;
   
   if(elapsed>0xffff)elapsed=elapsed-0xffff0000; 
     
   mycounter += elapsed;
   frame_counter++;
   if(mycounter >= vsync_freq)
   {
    fps=frame_counter;
    //if ((fps<49)&& (system_frameskip_key<12))system_frameskip_key++;
    frame_counter=0;
    mycounter=0;   
   }      
   
   if (++frameskip_counter >= system_frameskip_key) 
   {
	system_graphics_update();
	frameskip_counter = 0;
	videoBufferAvailable=1;
    } 
    
   if (have_sound)  
   {  	
      system_sound_update();
   }
   
   system_input_update();
   
   
   return;
}

int main(int argc, char *argv[])
{
    char *start_state;
  
   
    // init path_prefix system variable
    setBootPath(argc, argv);
    
    // set default opened item for the browser
    setDefBrowsingDevice( ps2_getBootDevice(path_prefix) );
    
    // state NULL
    start_state = NULL;

    /* some defaults, to be changed by getopt args */
    /* auto-select colour mode */
    system_colour = COLOURMODE_AUTO;
    /* default to English as language for now */
    language_english = TRUE;
    /* default to sound off */
    mute = FALSE;
    /* default frameski */
    system_frameskip_key = 2; // skip 1 frame of two

    // ngp_rom_path clean-up
    memset(ngp_rom_path, 0, 1024);
        
    // init system
    ps2_init();

    /* install BIOS */
    if (bios_install() == FALSE) {
	printf("cannot install BIOS\n");
	for(;;);
    }

    system_graphics_init();
  
    // display the loading screen
    splash_loading();
	
    // init sound
    system_input_init();
      
    // init sound
    if (mute == FALSE && system_sound_init() == FALSE) {
	printf("cannot init sound\n");
	mute = TRUE;
    }
    mute = FALSE;
    have_sound = !mute;
        
    // init timer
    timer_init();
    
    // init the browser
    initBrowser(boot_mode, "romlist.ini", ".ngp,.ngc,.npc");
    
    //display splash & credits
    splash_credit();
    
    // display directory listing
    display_dir();

    do // MAIN LOOP
    {
    	 do_exit=0;
     	 rom_exist=0;
     	 
     	 // browse roms
  	 while (!rom_exist)
  	 {
  	 	if(hostlist)
  	 	{
  	 	  strcpy(ngp_rom_path, path_prefix);
  	 	  strcat(ngp_rom_path, browser_main());
  	 	}
  	 	else
  	 	  strcpy(ngp_rom_path,browser_main());
     		
     		if(strstr(ngp_rom_path,BACK_STRING) != NULL)
           	  display_dir();          
          	  		
           	else if (system_io_file_exist(ngp_rom_path)==TRUE)
           	  rom_exist=1;
         }

  	 // display the loading screen
         splash_loading();
         
         // should not fail...
  	 system_rom_load(ngp_rom_path);
  	 // stop the CD from spinning
         CDVD_Stop(); 


	 // reset machine
	 reset();
	     
	 // load state if available
	 if (start_state != NULL)
	   state_restore(start_state);
	    
	 // start emulation
	 printf("start emulation...\n ");
	 if(have_sound) 
	   SjPCM_Play();
	 
	 // main emulation loop
	 do 
	 {
	    	    // emulate :)
		    emulate();  
		
	 } while (!do_exit);
    
    	 // pause sound
	 if(have_sound) 
	 {
	    SjPCM_Pause();
	    SjPCM_Clearbuff();
	 }
    
    } while (1); // uhm.. never exit ?
        
    system_rom_unload();
    system_sound_shutdown();

    return 0;
}



