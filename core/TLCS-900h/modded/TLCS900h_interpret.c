//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	TLCS900h_interpret.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

26 JUL 2002 - neopop_uk
=======================================
- Fixed a nasty bug that only affects [src]"EX (mem), XWA", 
	it was executing "EX F,F'" instead - Very bad! 

28 JUL 2002 - neopop_uk
=======================================
- Added generic DIV and DIVS functions

30 AUG 2002 - neopop_uk
=======================================
- Fixed detection of R32+d16 addressing mode.

02 SEP 2002 - neopop_uk
=======================================
- Added the undocumented type 0x13 R32 address mode.

09 SEP 2002 - neopop_uk
=======================================
- Extra cycles for addressing modes.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_registers.h"
#include "interrupt.h"
#include "mem.h"
#include "bios.h"
#include "TLCS900h_interpret.h"
#include "TLCS900h_interpret_single.h"
#include "TLCS900h_interpret_src.h"
#include "TLCS900h_interpret_dst.h"
#include "TLCS900h_interpret_reg.h"

//=========================================================================

_u32	mem;		//Result of addressing mode
int	size;		//operand size, 0 = Byte, 1 = Word, 2 = Long

_u8	first;		//The first byte
_u8	R;		//big R
_u8	second;		//The second opcode

BOOL	brCode;		//Register code used?
_u8	rCode;		//The code

_u8	cycles;		//How many state changes?
_u8	cycles_extra;	//How many extra state changes?

//=========================================================================

inline _u16 fetch16(void)
{
	_u16 a = loadW(pc);
	pc += 2;
	return a;
}

inline  _u32 fetch24(void)
{
	_u32 b, a = loadW(pc);
	pc += 2;
	b = loadB(pc++);
	return (b << 16) | a;
}

inline  _u32 fetch32(void)
{
	_u32 a = loadL(pc);
	pc += 4;
	return a;
}

//=============================================================================

inline void parityB(_u8 value)
{
	_u8 count = 0, i;

	//for (i = 0; i < 8; i++)
	for (i = 8; i--;)
	{
		if (value & 1) count++;
		value >>= 1;
	}

	// if (count & 1) == FALSE, means even, thus SET
	SETFLAG_V((count & 1) == 0);
}

inline void parityW(_u16 value)
{
	_u8 count = 0, i;

	//for (i = 0; i < 16; i++)
	for (i = 16; i--;)
	{
		if (value & 1) count++;
		value >>= 1;
	}

	// if (count & 1) == FALSE, means even, thus SET
	SETFLAG_V((count & 1) == 0);
}

//=========================================================================

inline void push8(_u8 data)	{ REGXSP -= 1;	storeB(REGXSP, data);}
inline void push16(_u16 data)	{ REGXSP -= 2;	storeW(REGXSP, data);}
inline void push32(_u32 data)	{ REGXSP -= 4;	storeL(REGXSP, data);}

inline _u8 pop8(void)	 {  _u8 temp = loadB(REGXSP); REGXSP += 1; return temp;}
inline _u16 pop16(void)  { _u16 temp = loadW(REGXSP); REGXSP += 2; return temp;}
inline _u32 pop32(void)  { _u32 temp = loadL(REGXSP); REGXSP += 4; return temp;}

//=============================================================================

inline _u16 generic_DIV_B(_u16 val, _u8 div)
{
	if (div == 0)
	{ 
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else
	{
		_u16 quo = val / (_u16)div;
		_u16 rem = val % (_u16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

inline _u32 generic_DIV_W(_u32 val, _u16 div)
{
	if (div == 0)
	{ 
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		_u32 quo = val / (_u32)div;
		_u32 rem = val % (_u32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

inline _u16 generic_DIVS_B(_s16 val, _s8 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else
	{
		_s16 quo = val / (_s16)div;
		_s16 rem = val % (_s16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

inline _u32 generic_DIVS_W(_s32 val, _s16 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		_s32 quo = val / (_s32)div;
		_s32 rem = val % (_s32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

inline _u8 generic_ADD_B(_u8 dst, _u8 src)
{
	
	//_u8 half = (dst & 0xF) + (src & 0xF);
	_u32 resultC = (_u32)dst + (_u32)src;
	_u8 result = (_u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) + (src & 0xF)) > 0xF);
	
        
	if ((((_s8)dst >= 0) && ((_s8)src >= 0) && ((_s8)result < 0)) ||
		(((_s8)dst < 0)  && ((_s8)src < 0) && ((_s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

inline _u16 generic_ADD_W(_u16 dst, _u16 src)
{
	//_u16 half = (dst & 0xF) + (src & 0xF);
	_u32 resultC = (_u32)dst + (_u32)src;
	_u16 result = (_u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) + (src & 0xF)) > 0xF);

	if ((((_s16)dst >= 0) && ((_s16)src >= 0) && ((_s16)result < 0)) ||
		(((_s16)dst < 0)  && ((_s16)src < 0) && ((_s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

inline _u32 generic_ADD_L(_u32 dst, _u32 src)
{
	_u64 resultC = (_u64)dst + (_u64)src;
	_u32 result = (_u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((_s32)dst >= 0) && ((_s32)src >= 0) && ((_s32)result < 0)) || 
		(((_s32)dst < 0)  && ((_s32)src < 0) && ((_s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

inline _u8 generic_ADC_B(_u8 dst, _u8 src)
{
	//_u8 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	_u32 resultC = (_u32)dst + (_u32)src + (_u32)FLAG_C;
	_u8 result = (_u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) + (src & 0xF) + FLAG_C) > 0xF);

	if ((((_s8)dst >= 0) && ((_s8)src >= 0) && ((_s8)result < 0)) || 
		(((_s8)dst < 0)  && ((_s8)src < 0) && ((_s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

inline _u16 generic_ADC_W(_u16 dst, _u16 src)
{
	//_u16 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	_u32 resultC = (_u32)dst + (_u32)src + (_u32)FLAG_C;
	_u16 result = (_u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) + (src & 0xF) + FLAG_C) > 0xF);

	if ((((_s16)dst >= 0) && ((_s16)src >= 0) && ((_s16)result < 0)) || 
		(((_s16)dst < 0)  && ((_s16)src < 0) && ((_s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

inline _u32 generic_ADC_L(_u32 dst, _u32 src)
{
	_u64 resultC = (_u64)dst + (_u64)src + (_u64)FLAG_C;
	_u32 result = (_u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((_s32)dst >= 0) && ((_s32)src >= 0) && ((_s32)result < 0)) || 
		(((_s32)dst < 0)  && ((_s32)src < 0) && ((_s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

inline _u8 generic_SUB_B(_u8 dst, _u8 src)
{
	//_u8 half = (dst & 0xF) - (src & 0xF);
	_u32 resultC = (_u32)dst - (_u32)src;
	_u8 result = (_u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) - (src & 0xF)) > 0xF);

	if ((((_s8)dst >= 0) && ((_s8)src < 0) && ((_s8)result < 0)) ||
		(((_s8)dst < 0) && ((_s8)src >= 0) && ((_s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

inline _u16 generic_SUB_W(_u16 dst, _u16 src)
{
	//_u16 half = (dst & 0xF) - (src & 0xF);
	_u32 resultC = (_u32)dst - (_u32)src;
	_u16 result = (_u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) - (src & 0xF)) > 0xF);

	if ((((_s16)dst >= 0) && ((_s16)src < 0) && ((_s16)result < 0)) ||
		(((_s16)dst < 0) && ((_s16)src >= 0) && ((_s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

inline _u32 generic_SUB_L(_u32 dst, _u32 src)
{
	_u64 resultC = (_u64)dst - (_u64)src;
	_u32 result = (_u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((_s32)dst >= 0) && ((_s32)src < 0) && ((_s32)result < 0)) ||
		(((_s32)dst < 0) && ((_s32)src >= 0) && ((_s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

inline _u8 generic_SBC_B(_u8 dst, _u8 src)
{
	//_u8 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	_u32 resultC = (_u32)dst - (_u32)src - (_u32)FLAG_C;
	_u8 result = (_u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) - (src & 0xF) - FLAG_C) > 0xF);

	if ((((_s8)dst >= 0) && ((_s8)src < 0) && ((_s8)result < 0)) ||
		(((_s8)dst < 0) && ((_s8)src >= 0) && ((_s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

inline _u16 generic_SBC_W(_u16 dst, _u16 src)
{
	//_u16 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	_u32 resultC = (_u32)dst - (_u32)src - (_u32)FLAG_C;
	_u16 result = (_u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	//SETFLAG_H(half > 0xF);
	SETFLAG_H(((dst & 0xF) - (src & 0xF) - FLAG_C) > 0xF);

	if ((((_s16)dst >= 0) && ((_s16)src < 0) && ((_s16)result < 0)) ||
		(((_s16)dst < 0) && ((_s16)src >= 0) && ((_s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

inline _u32 generic_SBC_L(_u32 dst, _u32 src)
{
	_u64 resultC = (_u64)dst - (_u64)src - (_u64)FLAG_C;
	_u32 result = (_u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((_s32)dst >= 0) && ((_s32)src < 0) && ((_s32)result < 0)) ||
		(((_s32)dst < 0) && ((_s32)src >= 0) && ((_s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

BOOL conditionCode(int cc)
{
	switch(cc)
	{
	case 0:	return 0;	//(F)
	case 1:	if (FLAG_S ^ FLAG_V) return 1; else return 0;	//(LT)
	case 2:	if (FLAG_Z | (FLAG_S ^ FLAG_V)) return 1; else return 0;	//(LE)
	case 3:	if (FLAG_C | FLAG_Z) return 1; else return 0;	//(ULE)
	case 4: if (FLAG_V) return 1; else return 0;	//(OV)
	case 5:	if (FLAG_S) return 1; else return 0;	//(MI)
	case 6:	if (FLAG_Z) return 1; else return 0;	//(Z)
	case 7:	if (FLAG_C) return 1; else return 0;	//(C)
	case 8:	return 1;	//always True														
	case 9:	if (FLAG_S ^ FLAG_V) return 0; else return 1;	//(GE)
	case 10:if (FLAG_Z | (FLAG_S ^ FLAG_V)) return 0; else return 1;	//(GT)
	case 11:if (FLAG_C | FLAG_Z) return 0; else return 1;	//(UGT)
	case 12:if (FLAG_V) return 0; else return 1;	//(NOV)
	case 13:if (FLAG_S) return 0; else return 1;	//(PL)
	case 14:if (FLAG_Z) return 0; else return 1;	//(NZ)
	case 15:if (FLAG_C) return 0; else return 1;	//(NC)
	}

#ifdef NEOPOP_DEBUG
	system_debug_message("Unknown Condition Code %d", cc);
#endif
	return FALSE;
}

//=============================================================================

_u8 get_rr_Target(void)
{
	_u8 target = 0x80;

	if (size == 0 && first == 0xC7)
		return rCode;

	//Create a regCode
	switch(first & 7)
	{
	case 0: if (size == 1)	target = 0xE0;	break;
	case 1:	
		if (size == 0)	target = 0xE0;
		if (size == 1)	target = 0xE4;
		break;
	case 2: if (size == 1)	target = 0xE8;	break;
	case 3:
		if (size == 0)	target = 0xE4;
		if (size == 1)	target = 0xEC;
		break;
	case 4: if (size == 1)	target = 0xF0;	break;
	case 5:	
		if (size == 0)	target = 0xE8;
		if (size == 1)	target = 0xF4;
		break;
	case 6: if (size == 1)	target = 0xF8;	break;
	case 7:
		if (size == 0)	target = 0xEC;
		if (size == 1)	target = 0xFC;
		break;
	}

	return target;
}

_u8 get_RR_Target(void)
{
	_u8 target = 0x80;

	//Create a regCode
	switch(second & 7)
	{
	case 0: if (size == 1)	target = 0xE0;	break;
	case 1:	
		if (size == 0)	target = 0xE0;
		if (size == 1)	target = 0xE4;
		break;
	case 2: if (size == 1)	target = 0xE8;	break;
	case 3:
		if (size == 0)	target = 0xE4;
		if (size == 1)	target = 0xEC;
		break;
	case 4: if (size == 1)	target = 0xF0;	break;
	case 5:	
		if (size == 0)	target = 0xE8;
		if (size == 1)	target = 0xF4;
		break;
	case 6: if (size == 1)	target = 0xF8;	break;
	case 7:
		if (size == 0)	target = 0xEC;
		if (size == 1)	target = 0xFC;
		break;
	}

	return target;
}

//=========================================================================

static inline void ExXWA()	{mem = regL(0);}
static inline void ExXBC()	{mem = regL(1);}
static inline void ExXDE()	{mem = regL(2);}
static inline void ExXHL()	{mem = regL(3);}
static inline void ExXIX()	{mem = regL(4);}
static inline void ExXIY()	{mem = regL(5);}
static inline void ExXIZ()	{mem = regL(6);}
static inline void ExXSP()	{mem = regL(7);}

static inline void ExXWAd()	{mem = regL(0) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXBCd()	{mem = regL(1) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXDEd()	{mem = regL(2) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXHLd()	{mem = regL(3) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXIXd()	{mem = regL(4) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXIYd()	{mem = regL(5) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXIZd()	{mem = regL(6) + (_s8)FETCH8; cycles_extra = 2;}
static inline void ExXSPd()	{mem = regL(7) + (_s8)FETCH8; cycles_extra = 2;}

static inline void Ex8()	{mem = FETCH8;		cycles_extra = 2;}
static inline void Ex16()	{mem = fetch16();	cycles_extra = 2;}
static inline void Ex24()	{mem = fetch24();	cycles_extra = 3;}

static inline void ExR32()
{
	_u8 data = FETCH8;

	if (data == 0x03)
	{
		_u8 rIndex, r32;
		r32 = FETCH8;		//r32
		rIndex = FETCH8;	//r8
		mem = rCodeL(r32) + (_s8)rCodeB(rIndex);
		cycles_extra = 8;
		return;
	}

	if (data == 0x07)
	{
		_u8 rIndex, r32;
		r32 = FETCH8;		//r32
		rIndex = FETCH8;	//r16
		mem = rCodeL(r32) + (_s16)rCodeW(rIndex);
		cycles_extra = 8;
		return;
	}

	//Undocumented mode!
	if (data == 0x13)
	{
		mem = pc + (_s16)fetch16();
		cycles_extra = 8;	//Unconfirmed... doesn't make much difference
		return;
	}

	cycles_extra = 5;

	if ((data & 3) == 1)
		mem = rCodeL(data) + (_s16)fetch16();
	else
		mem = rCodeL(data);
}

static inline void ExDec()
{
	_u8 data = FETCH8;
	_u8 r32 = data & 0xFC;

	cycles_extra = 3;

	switch(data & 3)
	{
	case 0:	rCodeL(r32) -= 1;	mem = rCodeL(r32);	break;
	case 1:	rCodeL(r32) -= 2;	mem = rCodeL(r32);	break;
	case 2:	rCodeL(r32) -= 4;	mem = rCodeL(r32);	break;
	}
}

static inline void ExInc()
{
	_u8 data = FETCH8;
	_u8 r32 = data & 0xFC;

	cycles_extra = 3;

	switch(data & 3)
	{
	case 0:	mem = rCodeL(r32);	rCodeL(r32) += 1;		break;
	case 1:	mem = rCodeL(r32);	rCodeL(r32) += 2;		break;
	case 2:	mem = rCodeL(r32);	rCodeL(r32) += 4;		break;
	}
}

static inline void ExRC()
{
	brCode = TRUE;
	rCode = FETCH8;
	cycles_extra = 1;
}

//=========================================================================

//Address Mode & Register Code
static inline void decodeExtraOptimized(u8 first)
{
	
	switch (first)
	{	
		   
		case 128 :			 
		case 144 :
		case 160 :
		case 176 :ExXWA();
			   break;
		case 129 :
		case 145 :
		case 161 :
		case 177 :ExXBC();
			   break;
		case 130 :
		case 146 :
		case 162 :
		case 178 :ExXDE();
			   break;
		case 131 :
		case 147 :
		case 163 :
		case 179 :ExXHL();
			   break;	
		case 132 :
		case 148 :
		case 164 :
		case 180 :ExXIX();
			   break;
		case 133 :	
		case 149 :
		case 165 :
		case 181 :ExXIY();
			   break;
		case 134 :
		case 150 :
		case 166 :
		case 182 :ExXIZ();
			   break;
		case 135 :
		case 151 :
		case 167 :
		case 183 :ExXSP();
			   break;
		case 136 :
		case 152 :
		case 168 :
		case 184 :ExXWAd();
			   break;
		case 137 :
		case 153 :
		case 169 :
		case 185 :ExXBCd();
			   break;
		case 138 :
		case 154 :
		case 170 :
		case 186 :ExXDEd();
			   break;	
		case 139 :
		case 155 :
		case 171 :
		case 187 :ExXHLd();
			   break;	
		case 140 :
		case 156 :
		case 172 :
		case 188 :ExXIXd();
			   break;
		case 141 :
		case 157 :
		case 173 :
		case 189 :ExXIYd();
			   break;	
		case 142 :
		case 158 :
		case 174 :
		case 190 :ExXIZd();
			   break;	
		case 143 :
		case 159 :
		case 175 :
		case 191 :ExXSPd();
			   break;
			   
		
		case 192 :
		case 208 :
		case 240 :
		case 224 :Ex8();
			   break;
		
		case 193 :
		case 209 :
		case 241 :
		case 225 :Ex16();
			   break;
		
		case 194 :
		case 210 :
		case 242 :
		case 226 :Ex24();
			   break;
		
		case 195 :
		case 211 :
		case 243 :
		case 227 :ExR32();
			   break;	
		
		case 196 :
		case 212 :
		case 244 :
		case 228 :ExDec();
			   break;	
		
		case 197 :
		case 213 :
		case 245 :
		case 229 :ExInc();
			   break;	

		case 199 :	
		case 215 :
		case 231 :ExRC();
			   break;
	
		default: break;
	}
};

//=========================================================================

static void e(void)
{
	instruction_error("Unknown instruction %02X", first);
}

static void es(void)
{
	instruction_error("Unknown [src] instruction %02X", second);
}

static void ed(void)
{
	instruction_error("Unknown [dst] instruction %02X", second);
}

static void er(void)
{
	instruction_error("Unknown [reg] instruction %02X", second);
}

//=========================================================================
static inline void srcDecodeOptimized(u8 second)
{
	
	switch (second)
	{	
		/*case 0  :	//0
		case 1  :
		case 2  :
		case 3  :es();break;*/
		case 4  :srcPUSH();break;
		//case 5  :es();break;
		case 6  :srcRLD();break;
		case 7  :srcRRD();break;
		/*case 8  :
		case 9  :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :es();break;*/
		case 16 :srcLDI();break;	//1
		case 17 :srcLDIR();break;
		case 18 :srcLDD();break;
		case 19 :srcLDDR();break;
		case 20 :srcCPI();break;
		case 21 :srcCPIR();break;
		case 22 :srcCPD();break;
		case 23 :srcCPDR();break;
		case 24 :es();break;
		case 25 :srcLD16m();break;
		/*case 26 :
		case 27 :
		case 28 :
		case 29 :
		case 30 :
		case 31 :es();break;*/
		case 32 :/*2*/
		case 33 :
		case 34 :
		case 35 :
		case 36 :
		case 37 :
		case 38 :
		case 39 :srcLD();break;
		/*case 40 :
		case 41 :
		case 42 :
		case 43 :
		case 44 :
		case 45 :
		case 46 :
		case 47 :es();break;*/
		case 48 :	/*3*/
		case 49 :
		case 50 :
		case 51 :
		case 52 :
		case 53 :
		case 54 :
		case 55 :srcEX();break;
		case 56 :srcADDi();break;
		case 57 :srcADCi();break;
		case 58 :srcSUBi();break;
		case 59 :srcSBCi();break;
		case 60 :srcANDi();break;
		case 61 :srcXORi();break;
		case 62 :srcORi();break;
		case 63 :srcCPi();break;
		case 64 :	/*4*/
		case 65 :
		case 66 :
		case 67 :
		case 68 :
		case 69 :
		case 70 :
		case 71 :srcMUL();break;
		case 72 :
		case 73 :
		case 74 :
		case 75 :
		case 76 :
		case 77 :
		case 78 :
		case 79 :srcMULS();break;
		case 80 :		/*5*/
		case 81 :
		case 82 :
		case 83 :
		case 84 :
		case 85 :
		case 86 :
		case 87 :srcDIV();break;
		case 88 :
		case 89 :
		case 90 :
		case 91 :
		case 92 :
		case 93 :
		case 94 :
		case 95 :srcDIVS();break;
		case 96 :		/*6*/	
		case 97 :
		case 98 :
		case 99 :
		case 100:
		case 101:
		case 102:
		case 103:srcINC();break;
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:srcDEC();break;
		/*case 112:		//7
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:es();break;*/
		case 120:srcRLC();break;		
		case 121:srcRRC();break;	
		case 122:srcRL();break;	
		case 123:srcRR();break;	
		case 124:srcSLA();break;	
		case 125:srcSRA();break;	
		case 126:srcSLL();break;	
		case 127:srcSRL();break;
		case 128:	/*8*/
		case 129:
		case 130:
		case 131:
		case 132:
		case 133:
		case 134:
		case 135:srcADDRm();break;
		case 136:
		case 137:
		case 138:
		case 139:
		case 140:
		case 141:
		case 142:
		case 143:srcADDmR();break;
		case 144:	/*9*/	
		case 145:
		case 146:
		case 147:
		case 148:
		case 149:
		case 150:
		case 151:srcADCRm();break;
		case 152:	
		case 153:
		case 154:
		case 155:
		case 156:
		case 157:
		case 158:
		case 159:srcADCmR();break;
		case 160:	/*A*/
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 166:
		case 167:srcSUBRm();break;
		case 168:	
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:
		case 174:
		case 175:srcSUBmR();break;
		case 176:	/*B*/
		case 177:
		case 178:
		case 179:	
		case 180:
		case 181:
		case 182:
		case 183:srcSBCRm();break;
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:
		case 190:
		case 191:srcSBCmR();break;
		case 192:	/*C*/
		case 193:
		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:srcANDRm();break;
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:srcANDmR();break;
		case 208:	/*D*/	
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
		case 215:srcXORRm();break;
		case 216:
		case 217:
		case 218:
		case 219:
		case 220:
		case 221:
		case 222:
		case 223:srcXORmR();break;
		case 224:	/*E*/	
		case 225:
		case 226:
		case 227:
		case 228:
		case 229:
		case 230:
		case 231:srcORRm();break;
		case 232:
		case 233:
		case 234:
		case 235:
		case 236:
		case 237:
		case 238:
		case 239:srcORmR();break;
		case 240:	/*F*/	
		case 241:
		case 242:
		case 243:
		case 244:
		case 245:
		case 246:
		case 247:srcCPRm();break;
		case 248:
		case 249:
		case 250:
		case 251:
		case 252:
		case 253:
		case 254:
		case 255:srcCPmR();break;
	  
	        default : break;
	}

};               	
      
//Secondary (DST) Instruction decode
static void (*dstDecode[256])() = 
{
/*0*/	dstLDBi,	ed,			dstLDWi,	ed,			dstPOPB,	ed,			dstPOPW,	ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*1*/	ed,			ed,			ed,			ed,			dstLDBm16,	ed,			dstLDWm16,	ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*2*/	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,
		dstANDCFA,	dstORCFA,	dstXORCFA,	dstLDCFA,	dstSTCFA,	ed,			ed,			ed,
/*3*/	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*4*/	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*5*/	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*6*/	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*7*/	ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*8*/	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,
		dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,
/*9*/	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,
		dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,
/*A*/	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	
		dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,
/*B*/	dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,
		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,
/*C*/	dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,
		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,
/*D*/	dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,
		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,
/*E*/	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,
		dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,
/*F*/	dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,
		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET
};

//Secondary (REG) Instruction decode
static void (*regDecode[256])() = 
{
/*0*/	er,			er,			er,			regLDi,		regPUSH,	regPOP,		regCPL,		regNEG,
		regMULi,	regMULSi,	regDIVi,	regDIVSi,	regLINK,	regUNLK,	regBS1F,	regBS1B,
/*1*/	regDAA,		er,			regEXTZ,	regEXTS,	regPAA,		er,			regMIRR,	er,
		er,			regMULA,	er,			er,			regDJNZ,	er,			er,			er,
/*2*/	regANDCFi,	regORCFi,	regXORCFi,	regLDCFi,	regSTCFi,	er,			er,			er,
		regANDCFA,	regORCFA,	regXORCFA,	regLDCFA,	regSTCFA,	er,			regLDCcrr,	regLDCrcr,
/*3*/	regRES,		regSET,		regCHG,		regBIT,		regTSET,	er,			er,			er,
		regMINC1,	regMINC2,	regMINC4,	er,			regMDEC1,	regMDEC2,	regMDEC4,	er,
/*4*/	regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,
		regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,
/*5*/	regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,
		regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,
/*6*/	regINC,		regINC,		regINC,		regINC,		regINC,		regINC,		regINC,		regINC,
		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,
/*7*/	regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,
		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,
/*8*/	regADD,		regADD,		regADD,		regADD,		regADD,		regADD,		regADD,		regADD,
		regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,
/*9*/	regADC,		regADC,		regADC,		regADC,		regADC,		regADC,		regADC,		regADC,
		regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,
/*A*/	regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,
		regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,
/*B*/	regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,
		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,
/*C*/	regAND,		regAND,		regAND,		regAND,		regAND,		regAND,		regAND,		regAND,
		regADDi,	regADCi,	regSUBi,	regSBCi,	regANDi,	regXORi,	regORi,		regCPi,
/*D*/	regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,
		regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,
/*E*/	regOR,		regOR,		regOR,		regOR,		regOR,		regOR,		regOR,		regOR,
		regRLCi,	regRRCi,	regRLi,		regRRi,		regSLAi,	regSRAi,	regSLLi,	regSRLi,
/*F*/	regCP,		regCP,		regCP,		regCP,		regCP,		regCP,		regCP,		regCP,
		regRLCA,	regRRCA,	regRLA,		regRRA,		regSLAA,	regSRAA,	regSLLA,	regSRLA
};

//=========================================================================

static inline void src_B()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 0;					//Byte Size

	//(*srcDecode[second])();		//Call
	srcDecodeOptimized(second);
}

static inline void src_W()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 1;					//Word Size

	//(*srcDecode[second])();		//Call
	srcDecodeOptimized(second);
}

static inline void src_L()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 2;					//Long Size

	//(*srcDecode[second])();		//Call
	srcDecodeOptimized(second);
}

static inline void dst()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;

	//(*dstDecode[second])();		//Call
	srcDecodeOptimized(second);
}

static _u8 rCodeConversionB[8] = { 0xE1, 0xE0, 0xE5, 0xE4, 0xE9, 0xE8, 0xED, 0xEC };
static _u8 rCodeConversionW[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };
static _u8 rCodeConversionL[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };

static inline void reg_B()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 0;					//Byte Size

	if (brCode == FALSE)
	{
		brCode = TRUE;
		rCode = rCodeConversionB[first & 7];
	}

	(*regDecode[second])();		//Call
}

static inline void reg_W()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 1;					//Word Size

	if (brCode == FALSE)
	{
		brCode = TRUE;
		rCode = rCodeConversionW[first & 7];
	}

	(*regDecode[second])();		//Call
}

static inline void reg_L()
{
	second = FETCH8;			//Get the second opcode
	R = second & 7;
	size = 2;					//Long Size

	if (brCode == FALSE)
	{
		brCode = TRUE;
		rCode = rCodeConversionL[first & 7];
	}

	(*regDecode[second])();		//Call
}

//=============================================================================
//Primary Instruction decode
static inline void decodeOptimized(u8 first)
{
	switch (first)
	{	
	
		case 0  :sngNOP();break; /*0*/
		case 1  :sngNORMAL();break;
		case 2  :sngPUSHSR();break;
		case 3  :sngPOPSR();break;
		case 4  :sngMAX();break;
		case 5  :sngHALT();break;
		case 6  :sngEI();break;
		case 7  :sngRETI();break;
		case 8  :sngLD8_8();break;
		case 9  :sngPUSH8();break;
		case 10 :sngLD8_16();break;
		case 11 :sngPUSH16();break;
		case 12 :sngINCF();break;
		case 13 :sngDECF();break;
		case 14 :sngRET();break;
		case 15 :sngRETD();break;
		case 16 :sngRCF();break;	/*1*/
		case 17 :sngSCF();break;	
		case 18 :sngCCF();break;	
		case 19 :sngZCF();break;	
		case 20 :sngPUSHA();break;
		case 21 :sngPOPA();break;
		case 22 :sngEX();break;
		case 23 :sngLDF();break;
		case 24 :sngPUSHF();break;
		case 25 :sngPOPF();break;
		case 26 :sngJP16();break;
		case 27 :sngJP24();break;
		case 28 :sngCALL16();break;
		case 29 :sngCALL24();break;
		case 30 :sngCALR();break;
		case 31 :iBIOSHLE();break;
		case 32 :/*2*/
		case 33 :
		case 34 :
		case 35 :
		case 36 :
		case 37 :
		case 38 :
		case 39 :sngLDB();break;
		case 40 :
		case 41 :
		case 42 :
		case 43 :
		case 44 :
		case 45 :
		case 46 :
		case 47 :sngPUSHW();break;
		case 48 :/*3*/	
		case 49 :
		case 50 :
		case 51 :
		case 52 :
		case 53 :
		case 54 :
		case 55 :sngLDW();break;
		case 56 :
		case 57 :
		case 58 :
		case 59 :
		case 60 :
		case 61 :
		case 62 :
		case 63 :sngPUSHL();break;
		case 64 :	/*4*/
		case 65 :
		case 66 :
		case 67 :
		case 68 :
		case 69 :
		case 70 :
		case 71 :sngLDL();break;
		case 72 :
		case 73 :
		case 74 :
		case 75 :
		case 76 :
		case 77 :
		case 78 :
		case 79 :sngPOPW();break;
		case 80 :/*5*/	
		case 81 :
		case 82 :
		case 83 :
		case 84 :
		case 85 :
		case 86 :
		case 87 :/*e()*/ break;
		case 88 :
		case 89 :
		case 90 :
		case 91 :
		case 92 :
		case 93 :
		case 94 :
		case 95 :sngPOPL();break;
		case 96 :break;	/*6*/	
		case 97 :
		case 98 :
		case 99 :
		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:sngJR();break;
		case 112:/*7*/
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:
		case 120:
		case 121:
		case 122:
		case 123:
		case 124:
		case 125:
		case 126:
		case 127:sngJRL();break;
		case 128:/*8*/
		case 129:
		case 130:
		case 131:
		case 132:
		case 133:
		case 134:
		case 135:
		case 136:
		case 137:
		case 138:
		case 139:
		case 140:
		case 141:
		case 142:
		case 143:src_B();break;
		case 144:/*9*/
		case 145:
		case 146:
		case 147:
		case 148:
		case 149:
		case 150:
		case 151:
		case 152:
		case 153:
		case 154:
		case 155:
		case 156:
		case 157:
		case 158:
		case 159:src_W();break;
		case 160:/*A*/
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 166:
		case 167:
		case 168:
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:
		case 174:
		case 175:src_L();break;
		case 176:/*B*/	
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:
		case 182:
		case 183:
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:
		case 190:
		case 191:dst();break;
		case 192:/*C*/
		case 193:
		case 194:
		case 195:
		case 196:
		case 197:src_B();break;
		case 198:/*e()*/break;
		case 199:
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:reg_B();break;
		case 208:/*D*/
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:src_W();break;
		case 214:/*e()*/;break;
		case 215:
		case 216:
		case 217:
		case 218:
		case 219:
		case 220:
		case 221:
		case 222:
		case 223:reg_W();break;
		case 224:/*E*/
		case 225:
		case 226:
		case 227:
		case 228:
		case 229:src_L();break;
		case 230:/*e()*/;break;
		case 231:
		case 232:
		case 233:
		case 234:
		case 235:
		case 236:
		case 237:
		case 238:
		case 239:reg_L();break;
		case 240:/*F*/
		case 241:
		case 242:
		case 243:
		case 244:
		case 245:dst();break;		
		case 246:/*e()*/;break;		
		case 247:sngLDX();break;
		case 248:	
		case 249:
		case 250:
		case 251:
		case 252:
		case 253:
		case 254:
		case 255:sngSWI();break;
	
		default : break;	
	};		
}
//=============================================================================

_u8 TLCS900h_interpret(void)
{
	brCode = FALSE;

	first = FETCH8;	//Get the first byte
	
	//Is any extra data used by this instruction?
	cycles_extra = 0;
	
	/*
	if (decodeExtra[first])
		(*decodeExtra[first])();
	*/
	
	decodeExtraOptimized(first);
	

	//(*decode[first])();	//Decode
	
	decodeOptimized(first);	//Decode

	return cycles + cycles_extra;
}

//=============================================================================
