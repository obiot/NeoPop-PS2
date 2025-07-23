/*
  NeoPop-PS2.h -- common header file
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


#ifndef _NEOPOP_PS2_H_
#define _NEOPOP_PS2_H_ 

#include <tamtypes.h>
#include <sifcmd.h>
#include <sifrpc.h> 
#include <loadfile.h> 
#include <fileio.h>
#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <libcdvd.h>
#include <libpad.h>
#include <libmc.h> 
#include <math.h>

#include "neopop.h"

#include "irx/modules.h" 
#include "sound/sjpcm.h"
#include "misc/timer.h"
#include "gui/ps2print.h"
#include "gui/ps2romselect.h"
#include "cdrom/cdvd_rpc.h"
#include "hdd/hdd.h"
#include "gs/gfxpipe.h"
#include "gs/gs.h"
#include "gs/hw.h" 

// PS2DRV includes
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fileXio_rpc.h>

#include "system_ps2.h"


/* convert 4bit colour component to 8bit */
#define CONV4TO8(x)	(((x)<<4)|(x))
/* convert 4bit colour component to 5bit */
#define RGB555(R,G,B) ((((u16)(B))<<10)|(((u16)(G))<<5)|((u16)(R)))



extern int videoBufferAvailable;

extern _s16 *sound_buffer;

extern int frame_counter;

extern unsigned vsync_freq;


extern _u16 rgb_lookup[16*16*16];


enum comms_mode {
    COMMS_NONE,
    COMMS_SERVER,
    COMMS_CLIENT,

    COMMS_LAST
};


//#define NGP_FPS 59.95			/* frames per second */
#define NGP_FPS 50			/* frames per second */


#define DEFAULT_SAMPLERATE	24000	/* default sample rate */

#define PAUSED_LOCAL	1
#define PAUSED_REMOTE	2


#define mP1UP    0x01  	/* up */
#define mP1DOWN  0x02  	/* down */
#define mP1LEFT  0x04  	/* left */
#define mP1RIGHT 0x08 	/* right */
#define mP1A 	 0x10  	/* button a */
#define mP1B 	 0x20	/* button b */
#define mP1C 	 0x40  	/* button c */

#define JOYPORT_ADDR	0x6F82

#define PADDLE_1	0




BOOL system_comms_connect(void);
void system_comms_pause(BOOL);

BOOL system_graphics_init(void);
void system_graphics_update(void);
void system_graphics_blit(void);

void system_input_init(void);
void system_input_update(void);
void system_input_waitforX(void);
int  system_input_isButtonPressed(u32 button);

BOOL system_io_file_exist(char *filename);

char *system_make_file_name(const char *, const char *, int);

void system_rom_changed(void);
BOOL system_rom_load(char *);
void system_rom_unload(void);

void system_sound_callback(void *, _u8 *, int);
void system_sound_chipreset(void);
BOOL system_sound_init(void);
void system_sound_shutdown(void);
void system_sound_silence(void);
void system_sound_update(void);

void system_state_load(void);
void system_state_save(void);

extern int paused;
extern int have_sound;
extern int samplerate;
extern char *state_dir;
extern char *flash_dir;
extern char *screenshot_dir;
extern int use_rom_name;
extern int state_slot;
extern int fs_mode;
extern int do_exit;

#endif //_NEOPOP-PS2_H_
