/*
  input.c -- Input Devices
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

/*-- Include Files ---------------------------------------------------------*/
#include <kernel.h>
#include <stdio.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <sifcmd.h> 
#include <loadfile.h>
#include <libpad.h>

#include "neopop_ps2.h"
#include "gui/ingame.h"
#include "system_ps2.h"

#define ROM_PADMAN

/*
 * Global var's
 */
// contains the pad's current state
static char padBuf_1[256] __attribute__((aligned(64))) __attribute__ ((section (".bss")));

static char actAlign[6];
static int actuators;

static struct padButtonStatus buttons;
static u32 paddata;



/*
 * waitPadReady()
 */
int waitPadReady(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];

    state = padGetState(port, slot);
    lastState = -1;
    while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState) {
            padStateInt2String(state, stateString);
            printf("Please wait, pad(%d,%d) is in state %s\n", 
                       port, slot, stateString);
        }
        lastState = state;
        state=padGetState(port, slot);
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1) {
        printf("Pad OK!\n");
    }
    return 0;
}

/*
 * padConnected()
 */
int padConnected(int port, int slot)
{
    int state;
    int lastState;

    state = padGetState(port, slot);
    lastState = -1;
    while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState)
        {
            if (state == PAD_STATE_DISCONN)
            	return 0;
        }
        lastState = state;
        state=padGetState(port, slot);
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1) {
        printf("Pad OK!\n");
    }
    return 1;
}

/*
 * initializePad()
 */
int initializePad(int port, int slot)
{

    int ret;
    int modes;
    int i;

    waitPadReady(port, slot);

    // How many different modes can this device operate in?
    // i.e. get # entrys in the modetable
    modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
    printf("The device has %d modes\n", modes);

    if (modes > 0) {
        printf("( ");
        for (i = 0; i < modes; i++) {
            printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
        }
        printf(")");
    }

    printf("It is currently using mode %d\n", 
               padInfoMode(port, slot, PAD_MODECURID, 0));

    // If modes == 0, this is not a Dual shock controller 
    // (it has no actuator engines)
    if (modes == 0) {
        printf("This is a digital controller?\n");
        return 1;
    }

    // Verify that the controller has a DUAL SHOCK mode
    i = 0;
    do {
        if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
            break;
        i++;
    } while (i < modes);
    if (i >= modes) {
        printf("This is no Dual Shock controller\n");
        return 1;
    }

    // If ExId != 0x0 => This controller has actuator engines
    // This check should always pass if the Dual Shock test above passed
    ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
    if (ret == 0) {
        printf("This is no Dual Shock controller??\n");
        return 1;
    }

    //printf("Enabling dual shock functions\n");

    // When using MMODE_LOCK, user cant change mode with Select button
    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_UNLOCK);

    /*
    waitPadReady(port, slot);
    printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

    waitPadReady(port, slot);        
    printf("enterPressMode: %d\n", padEnterPressMode(port, slot));
    */

    waitPadReady(port, slot);
    actuators = padInfoAct(port, slot, -1, 0);
    printf("# of actuators: %d\n",actuators);

    if (actuators != 0) {
        actAlign[0] = 0;   // Enable small engine
        actAlign[1] = 1;   // Enable big engine
        actAlign[2] = 0xff;
        actAlign[3] = 0xff;
        actAlign[4] = 0xff;
        actAlign[5] = 0xff;

        waitPadReady(port, slot);
        printf("padSetActAlign: %d\n", 
                   padSetActAlign(port, slot, actAlign));
    }
    else {
        printf("Did not find any actuators.\n");
    }
    waitPadReady(port, slot);

    return 1;
}



            
/*--------------------------------------------------------------------------*/
void system_input_init(void)
{
    int ret;
    
    padInit(0);

    // paddle 1
    printf("Init Paddle 1\n");
    if((ret = padPortOpen(PADDLE_1, 0, padBuf_1)) != 0)
    {
    	ps2_vsync();
        if(!initializePad(PADDLE_1, 0))
    	{
        	printf("pad initalization failed!\n");
        	SleepThread();
    	}
    }
    else
    {
        printf("padOpenPort failed: %d\n", ret);
        SleepThread();
    }
    

}

void system_input_shutdown(void)
{
 	padPortClose(PADDLE_1,0);
 	padEnd();
}


void inline system_input_update(void)
{
    int ret;
    
    // reset key values
    ram[JOYPORT_ADDR] = 0x00;
    

    // ------- controller 1 ---------------------------------------
    while (((ret=padGetState(PADDLE_1, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?

  
    if (padRead(PADDLE_1, 0, &buttons) != 0)
    {
    	paddata = 0xffff ^ (buttons.btns);

    	if  ((paddata & PAD_LEFT) 
    	 || (((buttons.mode >> 4) == 0x07)
    	 && (buttons.ljoy_h < 64)))         	ram[JOYPORT_ADDR] |= mP1LEFT; // joySet(P1LEFT);
        else if((paddata & PAD_RIGHT) 
         || (((buttons.mode >> 4) == 0x07)
         && (buttons.ljoy_h > 192)))		ram[JOYPORT_ADDR] |= mP1RIGHT; // joySet(P1RIGHT);
        if ((paddata & PAD_UP) 
         || (((buttons.mode >> 4) == 0x07)
         && (buttons.ljoy_v < 64)))		ram[JOYPORT_ADDR] |= mP1UP; // joySet(P1UP);
        else if ((paddata & PAD_DOWN) 
         || (((buttons.mode >> 4) == 0x07)
         && (buttons.ljoy_v > 192)))		ram[JOYPORT_ADDR] |= mP1DOWN; // joySet(P1DOWN);
      
        if(paddata & PAD_SQUARE) 		ram[JOYPORT_ADDR] |= mP1C; //joySet(P1C);
        if(paddata & PAD_CROSS) 		ram[JOYPORT_ADDR] |= mP1A; //joySet(P1A);
        if(paddata & PAD_CIRCLE) 		ram[JOYPORT_ADDR] |= mP1B; //joySet(P1B);
        

    	if(paddata & PAD_SELECT)		IngameMenu();

    } //endif
	
}
        

void system_input_waitforX(void)
{
   int ret;
   struct padButtonStatus buttons; 
   //wait for X key press
   while(1)
   {  
    	while (((ret=padGetState(PADDLE_1, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?

    	if (padRead(PADDLE_1, 0, &buttons) != 0)
    	{
	    	paddata = 0xffff ^ (buttons.btns);
                if(paddata & PAD_CROSS)
                {
            		return;
            	}
    	}
   }

}

int system_input_isButtonPressed(u32 button)
{
   int ret;
   static u32 mypaddata;
   struct padButtonStatus mybuttons; 
    
   while (((ret=padGetState(PADDLE_1, 0)) != PAD_STATE_STABLE)&&(ret!=PAD_STATE_FINDCTP1)&&(ret != PAD_STATE_DISCONN)); // more error check ?
   if (padRead(PADDLE_1, 0, &mybuttons) != 0)
   {
    	mypaddata = 0xffff ^ (mybuttons.btns);
     	if(mypaddata & button)
     	{
            return 1;
        }
   }
   
   return 0;

}
