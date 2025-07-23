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

#include <kernel.h>
#include <stdio.h> 
#include <string.h> 
#include <sifcmd.h>
#include <sifrpc.h>
#include <loadfile.h>  
#include <libhdd.h> 

#include "../irx/modules.h"
#include "hdd.h"

// hdd stuff
// can hold up to 256 partition.. should be enough for now ;)
int 		 hddFsNb;
t_hddFilesystem  hddFsList[MAX_PARTITIONS] __attribute__((aligned(64)));
t_hddInfo        hddInfo;

int hddAvailable = 0;

// private fct
void hddPowerOffCallback(void *arg);
int  hddLoadModules(void);


/*
 * setup the hdd
 * return -1 if no usable hdd found
 */
int hddSetup()
{	
	 // load hddmodules
  	if (hddLoadModules() < 0)
  	  // HDD NOT AVAILABLE
   	  return -1;
	printf("load Hdd\n");
	hddPreparePoweroff();
	printf("Hdd off\n");
	hddSetUserPoweroffCallback(hddPowerOffCallback, NULL);
	printf("Hdd callback\n"); 
	
	// check hdd
	if ((hddCheckPresent() < 0) || (hddCheckFormatted() < 0))
	  // HDD NOT AVAILABLE
   	  return -1;
	else
	{	
	  // HDD FORMATTED & AVAILABLE  
	  // get some info on the HDD
	  printf("Hdd Infos:\n");
	  hddGetInfo(&hddInfo); 
	  printf("Hdd Size   : %dMB \n",hddInfo.hddSize);
	  printf("Free Space : %dMB \n",hddInfo.hddFree);
	  // check available FS/Partition
	  hddFsNb =  hddGetFilesystemList(hddFsList, MAX_PARTITIONS);
	  printf("%d Active Partitions\n", hddFsNb);
	  // no partition mounted
	  hddStatus = HDD_DEV_UNMOUNTED;
	  // we use only pfs0:"
	  strcpy(hddDevName,"pfs0:");
	  
	  hddAvailable = 1;
	  return 1;
	}
}

/*
 * return HDD status
 * 0 not available
 */
int isHddAvailable()
{
	return hddAvailable;
}

/*
 * Load hdd modules
 * calles by hddSetup()
 */
int hddLoadModules()
{	
	int ret, mod_ret;
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";
	
	// search ps2dev9 irx 
    	ret = SifSearchModuleByName("ps2dev9");
    	//and unload it if found
    	if (ret != 0) SifUnloadModule(ret);
    	// this fix issue with already loaded dev9 version that may be different
    	
	ret = SifExecModuleBuffer(&ps2dev9Irx, size_ps2dev9Irx, 0, NULL, &mod_ret);
	if (ret < 0) return -1;
	
	ret = SifExecModuleBuffer(&ps2atadIrx, size_ps2atadIrx, 0, NULL, &mod_ret);
	if (mod_ret > 0) 
	{
	  printf("No HDD present !\n"); 
	  return -1; // no HDD
	  
	} 

	ret = SifExecModuleBuffer(&ps2hddIrx, size_ps2hddIrx, sizeof(hddarg), hddarg, &mod_ret);
	if (ret < 0) return -1;
	ret = SifExecModuleBuffer(&ps2fsIrx, size_ps2fsIrx, sizeof(pfsarg), pfsarg, &mod_ret);
	if (ret < 0) return -1;
	ret = SifExecModuleBuffer(&poweroffIrx, size_poweroffIrx , 0, NULL, &mod_ret);
	// loaded
	return ret;
} 

/*
 * Callback fct for poweroff
 * calles by hddSetup()
 */
void hddPowerOffCallback(void *arg)
{
	;// actually doing nothing !
} 

/*
 * Get a Directory content
 * put result in dentBuf[]
 * return nb of entries found
 */
int hddGetdir(const char *name, const char *extensions, t_hddDirEntry dentBuf[], int maxEnt)
{

	iox_dirent_t thisDir;
	int count = 0, dirFd, rv;
	char openString[1024];

	snprintf(openString, 1024, "%s%s", hddDevName, name);

	printf("hddIO getdir. openString = %s\n", openString);

	dirFd = fileXioDopen(openString);
	if(dirFd < 0)
		return dirFd;

	rv = fileXioDread(dirFd, &thisDir);
	while((rv > 0) && (count < maxEnt))
	{
		if(strcmp(thisDir.name, "."))
		{
			if(	(hddTocEntryCompare(thisDir.name, extensions)) ||
				(FIO_S_ISDIR(thisDir.stat.mode)))
			{

				strncpy(dentBuf[count].name, thisDir.name, 256);
				dentBuf[count].name[255] = '\0'; // for safety
				dentBuf[count].attrib = 0;
				if(FIO_S_ISDIR(thisDir.stat.mode))
					dentBuf[count].attrib |= HDD_ATTRIB_DIR;
				if(FIO_S_ISLNK(thisDir.stat.mode))
					dentBuf[count].attrib |= HDD_ATTRIB_SYMLINK;
				dentBuf[count].size = thisDir.stat.size;

				count++;
			}
		}
		rv = fileXioDread(dirFd, &thisDir);
	}

	fileXioDclose(dirFd);

	printf("hddIO getdir returning.. count = %d\n", count);

	return count;
}

/*
 * Parse entry for given extensions
 * called by hddGetdir()
 * return 0 if not found
 */
int hddTocEntryCompare(char* filename, const char* extensions)
{
	static char ext_list[129];
	char* token;
	char* ext_point;

	if(!extensions)
		return 1;
	
	strncpy(ext_list,extensions,128);
	ext_list[128]=0;

	token = strtok( ext_list, " ," );
	while( token != NULL )
	{
		// if 'token' matches extension of 'filename'
		// then return a match
		ext_point = strrchr(filename,'.');
		if(ext_point == NULL) return 0;

		if(strcasecmp(ext_point, token) == 0)
			return 1;
		
		/* Get next token: */
		token = strtok( NULL, " ," );
	}
	
	// If not match found then return FALSE
	return 0;	
}
