/*
  ps2romselect.c -- rom browser
  
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

#ifndef __PS2ROMSELECT_H__
#define __PS2ROMSELECT_H__

#define SCREEN_W 320
#define SCREEN_H 256
#define NWIDTH SCREEN_W
#define NHEIGHT SCREEN_H
#define TXTMIN 0
#define TXTMAX 320


#define CD_LABEL    "[ CDROM ]"
#define MC_LABEL    "[ MEMORY CARD 0 ]"
#define HDD_LABEL   "[ HDD ]"
#define HOST_LABEL  "[ HOST ]"
#define BACK_STRING ".."


#define BROWSE_CD 0
#define BROWSE_MC 1
#define BROWSE_HO 2
#define BROWSE_HD 3


#define Z_BOX0		2
#define Z_BOX1		3
#define Z_BOX2		4
#define Z_LIST		5
#define Z_SELECT	6
#define Z_SCROLLBG	7
#define Z_SCROLL	8
#define Z_SCROLL_M	9

#define ARRAY_ENTRIES	256


#define MAX_TEST_DISP 27

// define lef & margin for the list display
#define BROWSER_MARGIN 0

#define LEFT_OFFSET	16
#define RIGTH_OFFSET	180

#define TOP_OFFSET	70
#define BOTTOM_OFFSET	(2+142+64)

#define LINE_SPACE	18


struct ROMdata 
{
	char name[256]; // These values should be more than enough
	char filename[256];
	char displayname[256];
} __attribute__((aligned(64)));



void initBrowser(int, char*, char*);
void display_dir(void);
char *browser_main(void);
int  menu_update_input(void);
void initromdata(void);
int  init_mc(int mc_type);
void chooseDirectory();
void processRomScan(char rom3[120]);
void setDefBrowsingDevice(int);


#endif

