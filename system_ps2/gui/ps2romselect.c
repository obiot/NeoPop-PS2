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

/*
 * 
 * Original code took from infones by 7not6
 * Added HDD Support and a bit of clean up
 */

#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <fileio.h>
#include <sifrpc.h>
#include <malloc.h>
#include <stdio.h>
#include <libpad.h>
#include <libmc.h>
#include "../neopop_ps2.h"
#include "../system_ps2.h"
#include "../cdrom/cdvd.h"
#include "../cdrom/cdvd_rpc.h"
#include "../hdd/hdd.h"
#include "../gs/hw.h"
#include "../gs/gs.h"
#include "../gs/gfxpipe.h"

#include "ps2print.h" 
#include "ps2romselect.h"

#include "../data/browser.h"

int num_roms;
int selection;
int frame_position;
int frame_selection;
int choosedir=1; 


// default open item is the CD
int CDMCFLAG = BROWSE_CD;   


//extern int hostlist; 
int  hostlist=0;

// text is displayed centered if set to 1
int  centered=0;

// rom array
struct ROMdata *romdata;

// mc variables
static int mc_Type, mc_Free, mc_Format;

// device entries
static mcTable mcDir[ARRAY_ENTRIES] __attribute__((aligned(64)));
static t_hddDirEntry hddDirEntries[ARRAY_ENTRIES] __attribute__((aligned(64))); 
struct TocEntry myTocEntries[ARRAY_ENTRIES] __attribute__((aligned(64)));

// devices current path
static char cdpath[128] __attribute__((aligned(64))) ="/";
static char mcpath[128] __attribute__((aligned(64))) ="/*";
static char hddpath[128] __attribute__((aligned(64))) ="\0";

static char rom2[120] __attribute__((aligned(64)));

extern char path_prefix[128] __attribute__((aligned(64))); 

char dummyname[256]  __attribute__((aligned(64)));
char * romname;

char rom_extensions[24];

// private function
void loadHostlist(int fd);
void setListEntry(int index, char *entry);


//------------------------------------------------------------------------------------


/*
 * init the browser
 */
void initBrowser(int bootmode, char *host_listname, char *extensions)
{
   int fd;
   
   // store rom extensions
   strcpy(rom_extensions, extensions);
     
   // initialize buffers and arraies
   romdata = memalign(64,(sizeof(struct ROMdata) * ARRAY_ENTRIES));
   memset((u8 *)romdata,0,sizeof(struct ROMdata) * ARRAY_ENTRIES);
   
   // set to 1 to center display
   centered = 0;
   
   // default init value
   choosedir = 0;
   hostlist = 0;
   

   // check romlist
   if ((bootmode == BOOT_HO) && (host_listname!=NULL))
   {
   	char host_path[256]  __attribute__((aligned(64)));
   	
   	strcpy(host_path, path_prefix);
        strcat(host_path, "romlist.ini");
   
   	//test for hostlist presence
   	fd = fioOpen(host_path, O_RDONLY);
   	if(fd < 0) 
   	{
	   printf("%s not found.\n",host_path);
	   setDefBrowsingDevice(BROWSE_MC); // no hostlist available
	   hostlist = 0;
   	}  
   	else
   	{
   	   loadHostlist(fd);
   	   fioClose(fd);
   	   hostlist = 1;
   	}
   }
   
   // if no hostlist init "normal" browser
   if (hostlist == 0 )
   {
      // init all hdd stuff

      hddSetup();
    }





} // end initBrowser

//------------------------------------------------------------------------------------

/*
 * set the default device to browse
 */
void setDefBrowsingDevice(int device)
{
	CDMCFLAG = device;
}
//------------------------------------------------------------------------------------

/*
 * Display Available Directory on
 * the current device
 */ 
void display_dir()
{
	
	if (hostlist)
	  return;
	  
 	num_roms = 0;
	while(num_roms<=1)
	{
	
	  debut:
	
	    selection = 0;
	    frame_selection = 0;
	    frame_position=0;
	    
	 
	    choosedir=1; 
	       
	    initromdata();
	    
      	    chooseDirectory();
	    
	    sprintf(rom2,"x");
		
	    strcpy(rom2,browser_main());
	   
	    if(strstr(rom2,MC_LABEL ) != NULL){
	           printf(" BROWSE MC0: \n");
	           CDMCFLAG=BROWSE_MC;goto debut;          
	    }
	    else if(strstr(rom2,CD_LABEL ) != NULL)
	    {
	           printf(" BROWSE CDROM: \n");
	           if (CDMCFLAG!=BROWSE_CD)
	             CDVD_FlushCache();
	           CDMCFLAG=BROWSE_CD;goto debut;           
	    }  
	    else if(strstr(rom2,HDD_LABEL ) != NULL){
	           printf(" BROWSE HD: \n");
	           CDMCFLAG=BROWSE_HD;goto debut;           
	    }  
	    
	    selection = 0;
	    frame_selection = 0;
	    frame_position=0;   
	    
	    printf("scanning rom \n");
	    processRomScan(rom2);
	  
	    if(num_roms==1) printf("No roms detected\n");
	    // A MSG BOX SHOULD BE DISPLAYED HERE
	 
	}//fin while
 	choosedir=0;  	
} // end display_dir
//------------------------------------------------------------------------------------

/*
 * display the browser
 * with all available files
 */
char * browser_main()
{
	int i,y;
	int first_time = 2;
	int retpad;
	
	// Clear the screen (with ZBuffer Disabled)
	gp_disablezbuf(&thegp);
	gp_frect(&thegp,0,0,320<<4,machine_def.vdph<<4,0,GS_SET_RGBA(0,0,0,0x80));
	gp_enablezbuf(&thegp); 
	  
	while(1) 
	{

		retpad = menu_update_input();
		
		// a key was pressed or first time refresh
		if((retpad != 0)||(first_time)) 
		{ 
				
    
	     		// Draw browser image
	     		gp_uploadTexture(&thegp, NEOPOP_TEX, 512, 0, 0, GS_PSMCT32, browser, 320, 256);
		
			gp_setTex(&thegp, NEOPOP_TEX, 512, GS_TEX_SIZE_512, GS_TEX_SIZE_256, GS_PSMCT32, 0, 0, 0);
	
			gp_texrect(&thegp, 			// gfxpipe	
			  	 	0,    0, // x1,y1
				   	0,    0, // u1,v1
			   		(320)<<4, (256)<<4,	// x2 (320<<4) ,y2
			   		(320)<<4, (256)<<4, // u2 (320<<4), v2(224<<4)
			   		Z_BOX0, 				// z
			   		GS_SET_RGBA(255, 255, 255, 200) // color
			  );
	     		
			// Draw Scroll Box
		   	gp_gouradrect(&thegp, (BROWSER_MARGIN+LEFT_OFFSET)<<4, TOP_OFFSET<<4,GS_SET_RGBA(0x00, 0x00, 0x40, 64), (RIGTH_OFFSET-BROWSER_MARGIN)<<4, (BOTTOM_OFFSET)<<4,  GS_SET_RGBA(0x40,0x40, 0x80, 10), Z_BOX1);
		
			// Draw text in scroll box
			y = (TOP_OFFSET-3)<<4;
			for(i=frame_position;i<(frame_position+8);i++) 
			{
			  // this is a hack for the hostlist function
			  // should be cleaned-up...
  			  if(hostlist)

			  {	
			  	// avoid crash when it see ".."
			  	if (strlen(romdata[i].filename)>2)
			  	{
			  	  // do some clean-up on the filename
			  	  strcpy(dummyname, romdata[i].filename);
			  	  // keep FILENAME only
			  	  // assuming that FILE format is rom\FILENAME.XXX
	 	  	  	  strtok( dummyname, "\\.");
			  	  romname = strtok( NULL, "\\.");
			  	  // trunc string if too long
			  	  if (strlen(romname) > MAX_TEST_DISP) romname[MAX_TEST_DISP]='\0';
			  	}
			  	else 
			  	  romname = romdata[i].displayname;
			  }
			  else 
			  {
			    romname = romdata[i].displayname;
			  }
			   
			  
			  
			  if(((TOP_OFFSET-3)+(frame_selection*LINE_SPACE))<<4==y)
			  {
			    if(i<num_roms)
			    {	
	                 	if (centered)
	                 	 textCpixel(0,320,(y>>4)+4,GS_SET_RGBA(255, 255, 255, 255),0,0,Z_SCROLL,(char *)romname);
	                 	else
	                 	 textpixel((BROWSER_MARGIN+LEFT_OFFSET),(y>>4)+4,GS_SET_RGBA(255, 255, 255, 255),0,0,Z_SCROLL,(char *)romname);
	            	    }
			  }
			  else
			  {
			    if(i<num_roms)
			    {
	                 	if (centered)
	                 	  textCpixel(0,320,(y>>4)+4,GS_SET_RGBA(200, 200, 200, 128),0,0,Z_LIST,(char *)romname);
	                 	else
	                 	  textpixel((BROWSER_MARGIN+LEFT_OFFSET),(y>>4)+4,GS_SET_RGBA(200, 200, 200, 128),0,0,Z_LIST,(char *)romname);
	            	    }   
	          	  }
	            	  y += LINE_SPACE<<4;
		        }  
		   
	
			gp_frect(&thegp, (LEFT_OFFSET+BROWSER_MARGIN)<<4, ((TOP_OFFSET-1)+(frame_selection*LINE_SPACE))<<4,(RIGTH_OFFSET-BROWSER_MARGIN)<<4, (-3+76+7+(frame_selection*LINE_SPACE))<<4, Z_SELECT, GS_SET_RGBA(32, 81, 124, 50));
			gp_hardflush(&thegp);
			
			ps2_vsync();
	
			// Update drawing and display enviroments.
	    		ps2_switch_buffers();
	    		
	 		if((retpad == PAD_CROSS)&&(!first_time)) // X was pressed
				break;
				
			if (first_time) first_time--;
			
	  	 }
	  	 else
	  	 {
	  	 	//Just wait for next vblank
	  	 	//don't draw nothing !
	  	 	ps2_vsync();
	  	 }
	}
	
    	if(hostlist)
    	  return (char *)&romdata[selection].filename;
    
    	if(choosedir==1)
    	  return (char *)&romdata[selection].name;
    	else 
    	  return (char *)&romdata[selection].filename;  
    	  
} // end browser_main
//------------------------------------------------------------------------------------

/* 
 * paddle event in the browser
 */
int menu_update_input()
{
	static struct padButtonStatus lpad1; // just in case
	static int padcountdown = 0;
	static int pad_held_down = 0;
	static int lpad1_data = 0;
	
	int ret;
	
	// ------- controller 1 ---------------------------------------
   	while (((ret=padGetState(0, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?
        
        
	if(padRead(0, 0, &lpad1)!=0)
	{
		lpad1_data = 0xffff ^lpad1.btns;

		if((lpad1.mode >> 4) == 0x07) {
			if(lpad1.ljoy_v < 64) lpad1_data |= PAD_UP;
			else if(lpad1.ljoy_v > 192) lpad1_data |= PAD_DOWN;

			if(lpad1.ljoy_h < 64) lpad1_data |= PAD_LEFT;
			else if(lpad1.ljoy_h > 192) lpad1_data |= PAD_RIGHT;
		}
	}

	if(lpad1_data & PAD_CROSS)
	{        
	        while(1) 
	        {
			  if(padGetState(0, 0) == PAD_STATE_STABLE) {
				padRead(0, 0, &lpad1); // port, slot, buttons
				//lpad1_data = 0xffff ^ ((lpad1.btns[0] << 8) | lpad1.btns[1]);
				lpad1_data = 0xffff ^ lpad1.btns;
			  }
			  if(!(lpad1_data & PAD_CROSS)) break;
		}    
        	return PAD_CROSS;
        }

	if(padcountdown) padcountdown--;

	if((lpad1_data & PAD_DOWN) && (padcountdown==0) && (selection!=(num_roms-1))) 
	{
		selection++;

		if(frame_selection<7) frame_selection++;

		//if the pad has been held down for a certain amount of time, give padcountdown
		//a lower value, in effect making the scrolling of the text faster
		if(pad_held_down++<4) padcountdown=10;
		else padcountdown=4;

		//move the display frame if necessary
		if(selection>(frame_position+7)) frame_position++;

		return PAD_DOWN;

	}
	else if((lpad1_data & PAD_UP) && (padcountdown==0) && (selection>0)) {
		selection--;

		if(frame_selection>0)frame_selection--;

		if(pad_held_down++<4) padcountdown=10;
		else padcountdown = 4;

		if(selection<frame_position) frame_position--;

		return PAD_UP;
	}
	else if((lpad1_data & PAD_LEFT) && (padcountdown==0) && (frame_position>8)) {
		frame_position -= 8;
		selection -= 8;

		if(pad_held_down++<4) padcountdown=10;
		else padcountdown = 4;

		return PAD_LEFT;
	}
	else if((lpad1_data & PAD_RIGHT) && (padcountdown==0) && (frame_position<(num_roms-16))) {
		frame_position += 8;
		selection += 8;

		if(pad_held_down++<4) padcountdown=10;
		else padcountdown = 4;

		return PAD_RIGHT;
	}

	//if up or are NOT being pressed, reset the pad_held_down flag
	if(!(lpad1_data & (PAD_UP | PAD_DOWN))) pad_held_down = 0;

	return 0;
}
//------------------------------------------------------------------------------------

void initromdata()
{
	int i; 

 	for(i=ARRAY_ENTRIES; i--;)
 	{	
 		setListEntry(i,""); 
    	}
}
//------------------------------------------------------------------------------------

int init_mc(int mc_type)
{

 	int ret;
    
        
	if(mcInit(mc_type) < 0) 
	{
		printf("Failed to initialize memcard server!\n");
		SleepThread();
	}
	printf("mc init\n");

	// Since this is the first call, -1 should be returned.
	mcGetInfo(0, 0, &mc_Type, &mc_Free, &mc_Format); 
	mcSync(MC_WAIT, NULL, &ret);
	
	// Assuming that the same memory card is connected, this should return 0
	mcGetInfo(0 ,0,&mc_Type,&mc_Free,&mc_Format);
	mcSync(MC_WAIT, NULL, &ret);
	printf("MC Type: %d Free: %d Format: %d\n\n", mc_Type, mc_Free, mc_Format);
	
  return (int)(mc_Free*1000);
}
//------------------------------------------------------------------------------------

void chooseDirectory()
{   
  int i,ret;
  
  num_roms=0;
 
  if(CDMCFLAG==BROWSE_CD)
  {

    setListEntry(num_roms,MC_LABEL); 
    num_roms++;	
    
    if (isHddAvailable())
    {
    	setListEntry(num_roms,HDD_LABEL); 
    	num_roms++;	
    }
    
    
    while(CDVD_DiskReady(CdBlock)==CdNotReady);
	ret = CDVD_GetDir(cdpath, (char*)NULL, CDVD_GET_DIRS_ONLY, myTocEntries, ARRAY_ENTRIES, (char*)NULL);
    
    	printf("Retrieved %d directory entries\n",ret);

	for (i = 0;i<ret;i++)
	{
    		/*
             	printf("Dir name: %s\tLBA = %d\tSize = %d\n",
				myTocEntries[i].filename,
				myTocEntries[i].fileLBA,
				myTocEntries[i].fileSize); 
		*/
                sprintf(romdata[num_roms].filename,"cdfs:/%s",myTocEntries[i].filename);
                strcpy(romdata[num_roms].displayname, romdata[num_roms].filename);	
                strcpy(romdata[num_roms].name,myTocEntries[i].filename);//romdata[num_roms].filename
                //printf("%s - %s \n",romdata[num_roms].name, mcDir[i].name);			    
                num_roms++;	 
	}
	
 }
 else if(CDMCFLAG==BROWSE_MC)
 { 
    // stop the CD from spinning if not browsing it !
    CDVD_Stop(); 

    mcGetDir(0, 0, mcpath, 0, ARRAY_ENTRIES - 10, mcDir);
    mcSync(0, NULL, &ret);

    setListEntry(num_roms,CD_LABEL); 
    num_roms++;	
    
    if (isHddAvailable())
    {
    	setListEntry(num_roms,HDD_LABEL); 	
    	num_roms++;	
    }

    for(i=0; i < ret; i++)
	{
		if(mcDir[i].attrFile & MC_ATTR_SUBDIR)
		{
  	           	sprintf(romdata[num_roms].filename,"mc0:%s",mcDir[i].name);
                	strcpy(romdata[num_roms].name,mcDir[i].name);//romdata[num_roms].filename
                	strcpy(romdata[num_roms].displayname, romdata[num_roms].filename);	
                	//printf("%s - %s \n",romdata[num_roms].name, mcDir[i].name);			    
                	num_roms++;	
        	}
		
	}
	
 }
 else if (isHddAvailable())
 {	
 	
 	// stop the CD from spinning if not browsing it !
    	CDVD_Stop(); 
    	
    	setListEntry(num_roms,MC_LABEL); 
    	num_roms++;	
    	setListEntry(num_roms,CD_LABEL); 
    	num_roms++;
    	
	for (i=0;i<hddFsNb;i++)
	{
		// don't show HDLoader Partition
		if (strstr(hddFsList[i].name,"PP.HDL")==NULL)
		{
			//printf("Found : %s\n",hddFsList[count].name);
			sprintf(romdata[num_roms].filename,"%s%s",hddDevName,hddFsList[i].name);
                	strcpy(romdata[num_roms].name,hddFsList[i].name);
                	strcpy(romdata[num_roms].displayname, romdata[num_roms].filename);	
                	num_roms++;	
                }
	}
 	
 }
 
  
}
//------------------------------------------------------------------------------------

void processRomScan(char rom3[120])
{ 
    int i,ret;
    static char tname[128]  __attribute__((aligned(64)));
    
    num_roms=0;
    
    setListEntry(num_roms,BACK_STRING); 
    num_roms++;	
  
    if(CDMCFLAG==BROWSE_CD)
    {
	// CDROM
    	strcpy(tname,rom3);
    	strcat(tname,"/"); 
  
    
    	while(CDVD_DiskReady(CdBlock)==CdNotReady);
	ret = CDVD_GetDir(tname, rom_extensions , CDVD_GET_FILES_ONLY, myTocEntries, ARRAY_ENTRIES, tname);

	printf("retrieve %d file entries\n",ret);

	for (i = 0;i<ret;i++)
	{
                sprintf(romdata[num_roms].filename,"cdfs:/%s/%s",rom3,myTocEntries[i].filename);
                strcpy(romdata[num_roms].displayname, myTocEntries[i].filename);
                if (strlen(romdata[num_roms].displayname) > MAX_TEST_DISP) romdata[num_roms].displayname[MAX_TEST_DISP]='\0';
                strcpy(romdata[num_roms].name,myTocEntries[i].filename);
                num_roms++;	 
              	
		
	}
    
    	printf ("ROMS DETECTED: %d\n",num_roms);
    	
 
    }
    else if(CDMCFLAG==BROWSE_MC)
    {       
	char rom_extension2[24];
	char * rom_ext;
	
    	// backup the rom extension
    	strcpy(rom_extension2, rom_extensions);
    	
    	// search on MC for all extensions
    	rom_ext = strtok( rom_extension2, ",");
    	while (rom_ext != NULL)
    	{       
    	  
    	  strcpy(tname,rom3);
    	  strcat(tname,"/*");
    	  strcat(tname,rom_ext); 
   
	  mcGetDir(0, 0, tname, 0, ARRAY_ENTRIES - 10, mcDir);
	  mcSync(0, NULL, &ret);
	  printf("\nmcGetDir returned %d\nListing of %s directory on memory card:\n", ret,tname);
    		
    	  for(i=0; i < ret; i++)
	  {  
     		if(mcDir[i].attrFile & MC_ATTR_SUBDIR);
		//	printf("[DIR] %d %s\n",i,mcDir[i].name);
		else
		{
		     	sprintf(romdata[num_roms].filename,"mc0:%s/%s",rom3,mcDir[i].name);
		     	strcpy(romdata[num_roms].displayname, mcDir[i].name);
                	strncpy(romdata[num_roms].name,mcDir[i].name,20);//romdata[num_roms].filename	
                	printf("%s - %d bytes %s\n", mcDir[i].name, mcDir[i].fileSizeByte,romdata[num_roms].filename);			    
                	num_roms++;	     	 		     
		}
	  }		
    	 
    	  //printf("extension %s\n",rom_ext);
    	  rom_ext = strtok( NULL, ",");
    	}	
    	
    
    
    }
    else //if(CDMCFLAG==BROWSE_HD)
    {
 	strcpy(tname, (char*)"hdd0:");
    	strcat(tname, rom3);
    	
    	//unmount previous mounted partition
 	if (hddStatus == HDD_DEV_MOUNTED)
 	{	
 		if (fileXioUmount(hddDevName)  < 0)
 		{
			printf("umount of %s failed!\n", hddpath);
			return;
		}
 		hddStatus = HDD_DEV_UNMOUNTED;
 	}
  	
 	//mount new selected one
    	if (fileXioMount(hddDevName, tname, FIO_MT_RDONLY) < 0)
	{
		printf("Mount of %s failed!\n", tname);
		return;
	}
	strcpy(hddpath, tname);
	printf("%s mounted.\n", hddpath);
	hddStatus = HDD_DEV_MOUNTED;
	
 	// get file list
 	ret = hddGetdir(".", rom_extensions, hddDirEntries, ARRAY_ENTRIES);
 	
 	for(i=0; i < ret; i++)
	{  	
		// make sure it has the correct extension !
		// QUICK HACK 
		// hddGetDir doesn't only return asked extensions ?	
		// -> TO BE VERIFIED / CORRECTED ..
		if (strstr(hddDirEntries[i].name,".n")!=NULL)
		{  
			sprintf(romdata[num_roms].filename,"%s%s",hddDevName, hddDirEntries[i].name);
			strcpy(romdata[num_roms].displayname, hddDirEntries[i].name);
			if (strlen(romdata[num_roms].displayname) > MAX_TEST_DISP) romdata[num_roms].displayname[MAX_TEST_DISP]='\0';
                        strcpy(romdata[num_roms].name,hddDirEntries[i].name);//romdata[num_roms].filename	
                	//printf("%s\n",romdata[num_roms].filename);			    
                	num_roms++;	     	 		     
                }	
	}	
    }


} //end ProcessROMscan
//------------------------------------------------------------------------------------


/*
 * load the host romlist into memory
 */
void loadHostlist(int fd)
{
   int  finok=0,i,j,nb=0;
   int  hostlist_size=0;
   char *buf;
   
   

   hostlist_size = fioLseek(fd,0,SEEK_END);
   fioLseek(fd,0,SEEK_SET);

   buf=memalign(64,hostlist_size);
   fioRead(fd, buf, hostlist_size);

   nb=0;j=0;
	
    for (i=0; i<(hostlist_size); i++)
    {
	if(buf[i]==0) break;
	    
	if(buf[i]!=';' && finok==0)
	{
        	romdata[nb].filename[j]=buf[i];
    		j++;   
    	}
    	if(buf[i]==';')
    	{
    	 	finok=1;
          	romdata[nb].filename[j]='\0';

        }     	
        
    	if(buf[i]=='\n'){j=0;nb++;finok=0;}
    	if(nb==ARRAY_ENTRIES) break;
    }

    free(buf);
    buf=NULL;
    
    num_roms = nb;
    
    return;
   
} // end loadHostlist


// set a entry to the specified string
void setListEntry(int index, char *entry)
{
   strcpy(romdata[index].name, entry);
   strcpy(romdata[index].filename, entry);
   strcpy(romdata[index].displayname, entry);
}


