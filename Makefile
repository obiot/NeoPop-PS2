#     _  ___  ___   ___   ___   ___     ____    ___ ___ 
# |\  | |___ |   |  ___| |   |  ___|  /  ___|  |    ___|
# | \_| |___ |___| |     |___| |     /  |   ___|   |___  Neopop/Ps2
#------------------------------------------------------------------
# Copyright 2005 - evilo <evilo13@gmail.com>
# Released under GNU license
# Review neopop/ps2 README & LICENSE files for further details.
#------------------------------------------------------------------
# Cygwin / GCC 3.2.2 / PS2SDK 1.2
#------------------------------------------------------------------


EE_BIN      = neopop.elf

NeoPop_CORE = Core/bios.o Core/biosHLE.o Core/dma.o Core/flash.o Core/gfx.o Core/gfx_scanline_mono.o Core/gfx_scanline_colour.o \
	      Core/interrupt.o Core/mem.o Core/neopop.o Core/rom.o Core/sound.o Core/state.o Core/Z80_interface.o \
	      Core/TLCS-900h/TLCS900h_interpret_single.o Core/TLCS-900h/TLCS900h_interpret_src.o \
	      Core/TLCS-900h/TLCS900h_interpret.o Core/TLCS-900h/TLCS900h_registers.o Core/TLCS-900h/TLCS900h_interpret_reg.o \
	      Core/TLCS-900h/TLCS900h_interpret_dst.o Core/z80/dasm.o Core/z80/Z80.o
	      
NeoPop_PS2 = system_ps2/cdrom/cdvd_rpc.o \
	     system_ps2/gs/gfxpipe.o system_ps2/gs/gs.o system_ps2/gs/hw.o \
	     system_ps2/misc/timer.o system_ps2/system_ps2.o\
	     system_ps2/sound/sjpcm_rpc.o \
	     system_ps2/system_comms.o system_ps2/system_graphics.o \
	     system_ps2/system_input.o system_ps2/system_io.o system_ps2/system_language.o \
	     system_ps2/system_rom.o system_ps2/system_sound.o \
	     system_ps2/system_main.o system_ps2/irx/cdvdIrx.o system_ps2/irx/fileXioIrx.o \
	     system_ps2/irx/iomanXIrx.o system_ps2/irx/isjpcmIrx.o system_ps2/irx/ps2hddIrx.o \
	     system_ps2/irx/poweroffIrx.o system_ps2/irx/ps2fsIrx.o system_ps2/irx/ps2atadIrx.o \
	     system_ps2/irx/ps2dev9Irx.o \
	     system_ps2/hdd/hdd.o \
	     system_ps2/gui/ps2print.o system_ps2/gui/ingame.o \
	     system_ps2/gui/ps2romselect.o system_ps2/gui/splash.o
	     

EE_OBJS     = $(NeoPop_CORE) $(NeoPop_PS2)
EE_CFLAGS  += -funroll-loops -finline-functions  -fshort-double -mlong64 -mhard-float -EL \
	      -fno-exceptions -fno-builtin  -ffast-math -DINLINE="inline static" -DLSB_FIRST
EE_LDFLAGS += -L./system_ps2/lib -L$(PS2SDK)/sbv/lib
EE_INCS    += -I$(PS2SDK)/sbv/include/ -I./Core/ -I./Core/TLCS-900h/ -I./Core/z80/
EE_LIBS    += -lc -lm -lcdvd -lmc -lpad -lhdd -lfileXio -lpoweroff -lpatches  -lcdvdfs -lgcc


BIN_DIR =../bin
CP    = cp

all:    make-irx $(EE_BIN) 
	$(EE_STRIP) $(EE_BIN)
	$(CP) $(EE_BIN) $(BIN_DIR)/$(EE_BIN)

clean:
	rm -f -R $(EE_BIN) $(NeoPop_CORE) $(NeoPop_PS2) 
	
clean_core:
	rm -f -R $(EE_BIN) $(NeoPop_CORE)
	
clean_sys:
	rm -f -R $(EE_BIN) $(NeoPop_PS2)

make-irx: 
	$(MAKE) -C system_ps2/sound/iop

clean-irx: 
	$(MAKE) -C system_ps2/sound/iop clean 
 	
reset :
	ps2client -h 192.168.1.110 reset

run :
	ps2client -h 192.168.1.110 execee host:$(BIN_DIR)/$(EE_BIN) 	


include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

