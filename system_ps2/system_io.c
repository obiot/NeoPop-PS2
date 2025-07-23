/*
  system_io.c -- read/write flash files 
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

static int fileMode =	FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP |
			FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;


static BOOL read_file_to_buffer(char *filename, _u8 *buffer, _u32 len)
{
	int fd;
	
	if (strstr(filename,"pfs") != NULL) // LOAD FROM HDD
   	{
	   fd = fileXioOpen(filename, O_RDONLY, fileMode);
	   if (fd==-1) return FALSE;
	   //read into buffer
	   printf("%s s:%x \n",filename,len);
	   fileXioRead(fd, buffer, len);
	   fileXioClose(fd);
	}
   	else // use fioXxxxx
   	{
   	   fd = fioOpen(filename, O_RDONLY);
	   if (fd==-1) return FALSE;
	   //read into buffer
	   printf("%s s:%x \n",filename,len);
	   fioRead(fd, buffer, len);
	   fioClose(fd);
	}
	return TRUE;
}


static BOOL write_file_from_buffer(char *filename, _u8 *buffer, _u32 len)
{
   return TRUE;
}


static BOOL validate_dir(const char *path)
{
   
    return TRUE;
}

void system_state_load(void)
{

    return;
}

void system_state_save(void)
{
    return;
}

BOOL system_io_file_exist(char *filename)
{
	int fd;
	
	if (strstr(filename,"pfs") != NULL) // LOAD FROM HDD
   	{
	   fd = fileXioOpen(filename, O_RDONLY, fileMode);
	   if (fd==-1) return FALSE;
	   fileXioClose(fd);
	}
   	else // use fioXxxxx
   	{
   	   fd = fioOpen(filename, O_RDONLY);
	   if (fd==-1) return FALSE;
	   fioClose(fd);
	}
	return TRUE;
}


BOOL system_io_rom_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL system_io_flash_read(_u8* buffer, _u32 len)
{
    //return read_file_to_buffer(NULL, buffer, len); not supported
    return FALSE;
}

BOOL system_io_flash_write(_u8* buffer, _u32 len)
{
    //return write_file_from_buffer(NULL, buffer, len); not supported
    return TRUE;
}

BOOL system_io_state_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL system_io_state_write(char *filename, _u8 *buffer, _u32 len)
{
    return write_file_from_buffer(filename, buffer, len);
}

char * system_make_file_name(const char *dir, const char *ext, int writing)
{
   return NULL;
}

