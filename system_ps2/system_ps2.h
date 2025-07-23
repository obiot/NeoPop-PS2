/* 
 * ps2 specific functions
 */

#ifndef __SYSTEM_PS2_H__
#define __SYSTEM_PS2_H__

#define PAL_MODE 50
#define NTSC_MODE 60 

#define X_TYPE 0
#define S_TYPE 1
 

/* 
VRAM layout

0x000000 - FB 1
0x080000 - FB 2 (FB 1 + 512*256*4)
0x100000 - ZBuf (FB 2 * 2)
0x180000 - End of ZBuf. Star of TEX and CLUT area.
*/
#define NEOPOP_TEX	0x100000 + 0x080000
#define BACK_TEX	0x180000 + 0x0A0000
#define VRAM_MAX	0x3E8000


#define BUF_WIDTH  160
#define BUF_HEIDTH 152

#define PS2_SCREEN_WIDTH 320

#define FULLSCREEN_DPW  216 // almost double with same aspect ratio (x1.35)
#define FULLSCREEN_DPH  205 // almost double with same aspect ratio (x1.35)

// boot mode
#define BOOT_CD 0
#define BOOT_MC 1
#define BOOT_HO 2
#define BOOT_HD 3
#define BOOT_UNKNOW 4 

extern int boot_mode;


extern int 	boot_mode; 
extern char 	path_prefix[128]; 

extern int 	whichdrawbuf;
extern int 	whichbackbuf;

extern u16* framebuffer;//[256*256]  __attribute__((aligned(64))); 

// Structure will be used directly in the options save/load code.
typedef struct
{
	u8 version;
	u8 renderFilter;		// 0 = Nearest, 1 = Linear
	u8 soundOn;			// 0 = Sound off, 1 = Sound on
	u8 fullscreen;			// 0 = original,1 double, 2 fullscreen
	u8 showFPS;			// 1 ON
	u8 rfu;				// RFU
	u8 SaveOn;			// 0 = Save off, 1 = save on
	u8 dispXPAL, dispYPAL;		// X & Y offset parameters for both
	u8 dispXNTSC, dispYNTSC;	// PAL and NTSC display modes.

} struct_neopopSettings; 

extern struct_neopopSettings neopopSettings  __attribute__((aligned(64))); 

typedef struct
{
	u32 	vdph;			// video heigth
	u32 	snd_sample;		// snd sample
	u32 	fps_rate;		// snd sample
	u32  	x1_offset;		// x1 texture offset
	u32 	x2_offset;		// x2 texture offset
	u32  	y1_offset;		// y1 texture offset
	u32 	y2_offset;		// y2 texture offset
	s32 	dispx;			// X screen offset 
	s32 	dispy;			// Y screen offset 
	
} struct_machine  __attribute__((aligned(64)));
extern struct_machine machine_def; 


extern void (*ps2_render_screen)(void); 


void ps2_init();
void ps2_shutdown();
void ps2_rebootIOP();
void ps2_setDisplay();
void setBootPath(int argc, char *argv[]);
int  ps2_getBootDevice(char *path);
void ps2_blit();
void ps2_system_uploadbackground(int fullscreen);
void ps2_render_fullscreen();
void ps2_render_double();
void ps2_render_windowed();
void ps2_vsync();
void ps2_switch_buffers();
void ps2_loadModules(int module_type);

#endif

