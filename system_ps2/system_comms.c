/*
  system_comms.c -- comm port support functions
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


/* #define COMMS_DEBUG */

#include "neopop_ps2.h"


/* interlink defaults  */
int comms_mode   = COMMS_NONE;
char *comms_host = NULL;
int comms_port   = 7846;

void comms_write_message(int, _u8);

#define COMMS_DATA	0
#define COMMS_PAUSE	1




BOOL system_comms_connect(void)
{
   printf("no comms support\n");
   return FALSE;
}

void system_comms_pause(BOOL pause)
{
    return;
}

BOOL system_comms_poll(_u8* buffer)
{
    return FALSE;
}

BOOL system_comms_read(_u8* buffer)
{
    return FALSE;
}

void system_comms_write(_u8 data)
{
    return;
}
