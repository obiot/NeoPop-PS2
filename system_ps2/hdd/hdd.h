/*
  hdd.c -- HDD specific functions
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

#ifndef __HDD_H__
#define __HDD_H__

// PS2DRV includes
#include <libhdd.h> 
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fileXio_rpc.h>


#define	HDD_FLAG_WRITE			0x01

#define HDD_ATTRIB_DIR			0x01
#define HDD_ATTRIB_SYMLINK		0x02

#define HDD_STATE_ERROR			0x01

#define HDD_DEV_MOUNTED			0x01
#define HDD_DEV_UNMOUNTED		0x00

#define MAX_PARTITIONS 256


typedef struct 
{
	char name[256];
	u32 attrib;
	u32 size;
} t_hddDirEntry;


// Device name (pfs0,pfs1,pfs2)
char hddDevName[32];

// Current state of device/mount
u32 hddStatus;

extern int 		hddFsNb;
extern int 		hddDevId;
extern t_hddFilesystem  hddFsList[MAX_PARTITIONS] __attribute__((aligned(64)));
extern t_hddInfo        hddInfo;

int hddSetup(void);
int isHddAvailable(void);
int hddGetdir(const char *name, const char *extensions, t_hddDirEntry dentBuf[], int maxEnt);
int hddTocEntryCompare(char* filename, const char* extensions);

#endif
