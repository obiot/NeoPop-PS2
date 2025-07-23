/*
  system_sound.c -- sound support functions
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

/* desired output sample rate */
int samplerate = DEFAULT_SAMPLERATE;		


_s16 *sound_buffer;
_s16 internal_sound_buffer[2][960]  __attribute__((aligned(64)))  __attribute__ ((section (".bss")));

int wichsoundbuf=0;



void system_sound_chipreset(void)
{

    sound_init(samplerate);
    return;
}

BOOL system_sound_init(void)
{
    
  sound_init(samplerate);
  
  int i;


  for(i=0;i<960;i++)
  {
    internal_sound_buffer[0][i]=0;
    internal_sound_buffer[1][i]=0;
  }
  
  if(SjPCM_Init(1) < 0)
  {
  	printf("SjPCM Bind failed!!");    
  	return FALSE;
  }
  
  SjPCM_Clearbuff();

  if(have_sound) 
	SjPCM_Play();
	
  sound_buffer = internal_sound_buffer[wichsoundbuf];
  //wichsoundbuf  ^= 1;

  return TRUE;
}


void system_sound_shutdown(void)
{

    free(sound_buffer);
    sound_buffer = NULL;

    return;
}

void system_sound_silence(void)
{

    if (mute == TRUE)
	return;
 
    //SjPCM_Pause();
    return;
}


void inline system_sound_update()
{

   sound_update((_s16 *)&sound_buffer[0], machine_def.snd_sample);
   SjPCM_Enqueue(sound_buffer, sound_buffer, machine_def.snd_sample, 0);
   
   //sound_buffer = internal_sound_buffer[wichsoundbuf];
   // switch the sound buffer
   //wichsoundbuf  ^= 1;
   

}
