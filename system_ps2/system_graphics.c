/*
  system_graphics.c -- graphics support functions
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


/* lookup table for color display surface pixels */
_u16 rgb_lookup[16*16*16]   __attribute__((aligned(64)));

/* lookup table for mono display surface pixels */
const _u16 __attribute__((aligned(64)))
    mono_lookup[8] = {	0x0000,	// 000b => 00000b (C5)
			0x1084,	// 001b => 00100b (C5)
			0x2108,	// 010b => 01000b (C5)
			0x318C,	// 011b => 01100b (C5)
			0x4210,	// 100b => 10000b (C5)
			0x5294,	// 101b => 10100b (C5)
			0x6318,	// 110b => 11000b (C5)
			0x739C	// 111b => 11100b (C5)
		      } ;

// 2 backbuffer
_u16 backbuffer[2][256*256]  __attribute__((aligned(64)));

// identify wich back buffer to use
int whichbackbuf = 0; 

// zbuffer (used by gfx)
_u8 *zbuffer   __attribute__((aligned(64)));




/* init graphics system & variables */
BOOL system_graphics_init(void)
{
    u16 i;
    u16 r, g, b;

    for (i=0; i<16*16*16; i++) 
    {
	r = (i&0xf)<< 1;
	g = ((i>>4)&0xf)<< 1;
	b = ((i>>8)&0xf)<< 1;

	/* RGB555 display format */
	rgb_lookup[i] = RGB555(r,g,b);
    }
	
    // front buffer point to 1st backbuffer
    cfb = backbuffer[whichbackbuf];
    //whichbackbuf  ^= 1;
    
    ps2_setDisplay();
    
    // put zbuffer into 16kb scratch pad @ 70002000
    zbuffer = (_u8 *)(0x70002000); 
        
    // vsync callback
    addVSyncCallback(&system_graphics_blit);
       
    return TRUE;
}



inline void system_graphics_update(void)
{
    // send backbuffer to vram
    ps2_render_screen();
        
    // switch back buffers
    whichbackbuf  ^= 1;
    cfb = backbuffer[whichbackbuf];
    
    
}

// called by the vsync interrupt
void system_graphics_blit(void)
{
    if (videoBufferAvailable==1)
    {
	   ps2_switch_buffers();
	   videoBufferAvailable=0;
    } 
    asm __volatile__ ("ei");
}

