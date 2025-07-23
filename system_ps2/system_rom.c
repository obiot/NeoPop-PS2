/*
  system_rom.c -- ROM loading support
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

#include <string.h>

#include "neopop_ps2.h"

static int fileMode =	FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP |
			FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;



static BOOL rom_load(char *filename)
{
    //stat st;
    int fd;

    // detect rom size & allocate mem
   if (strstr(filename,"pfs") != NULL) // LOAD FROM HDD
   {
    
      fd = fileXioOpen(filename, O_RDONLY, fileMode);
      if (fd < 0) 
        return FALSE;
      
      rom.length = fileXioLseek (fd,0,SEEK_END);
      rom.data = (_u8*)memalign(64, rom.length);
      fileXioLseek(fd, 0, SEEK_SET);
      fileXioRead(fd, rom.data, rom.length);
      fileXioClose(fd);
   }
   else // use fioXxxxx
   {
    
      fd = fioOpen(filename, O_RDONLY);
      if (fd < 0) 
       return FALSE;
 
     
      rom.length = fioLseek(fd,0,SEEK_END);
      rom.data = (_u8*)memalign(64, rom.length);
      fioLseek(fd, 0, SEEK_SET);
      fioRead(fd, rom.data, rom.length);
      fioClose(fd);
   }
   printf("%s size:%d \n",filename, rom.length);
    
   return TRUE;


}

void system_rom_changed(void)
{
    char title[128];

    (void)snprintf(title, sizeof(title), PROGRAM_NAME " - %s", rom.name);

}
    
BOOL system_rom_load(char *filename)
{
    char *fn;
    BOOL ret;

    /* Remove old ROM from memory */
    system_rom_unload();

    ret = rom_load(filename);

    if (ret == FALSE)
	return FALSE;

    memset(rom.filename, 0, sizeof(rom.filename));
    if ((fn=strrchr(filename, '/')) == NULL)
	fn = filename;
    else
	*fn++ = '\0';

    /* don't copy extension */
    strncpy(rom.filename, fn, min(sizeof(rom.filename), strlen(fn)-4));

    rom_loaded();
    system_rom_changed();

    return TRUE;
}

void system_rom_unload(void)
{
    rom_unload();
}
