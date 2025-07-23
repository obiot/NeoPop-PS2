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

	TLCS900h_interpret_single.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_SINGLE__
#define __TLCS900H_SINGLE__
//=========================================================================

//===== NOP
inline void sngNOP(void);

//===== NORMAL
 inline void sngNORMAL(void);

//===== PUSH SR
 inline void sngPUSHSR(void);

//===== POP SR
 inline void sngPOPSR(void);

//===== MAX
 inline void sngMAX(void);

//===== HALT
 inline void sngHALT(void);

//===== EI #3
 inline void sngEI(void);

//===== RETI
 inline void sngRETI(void);

//===== LD (n), n
 inline void sngLD8_8(void);

//===== PUSH n
 inline void sngPUSH8(void);

//===== LD (n), nn
 inline void sngLD8_16(void);

//===== PUSH nn
 inline void sngPUSH16(void);

//===== INCF
 inline void sngINCF(void);

//===== DECF
 inline void sngDECF(void);

//===== RET condition
 inline void sngRET(void);

//===== RETD dd
 inline void sngRETD(void);

//===== RCF
 inline void sngRCF(void);

//===== SCF
 inline void sngSCF(void);

//===== CCF
 inline void sngCCF(void);

//===== ZCF
 inline void sngZCF(void);

//===== PUSH A
 inline void sngPUSHA(void);

//===== POP A
 inline void sngPOPA(void);

//===== EX F,F'
 inline void sngEX(void);

//===== LDF #3
 inline void sngLDF(void);

//===== PUSH F
 inline void sngPUSHF(void);

//===== POP F
 inline void sngPOPF(void);

//===== JP nn
 inline void sngJP16(void);

//===== JP nnn
 inline void sngJP24(void);

//===== CALL #16
 inline void sngCALL16(void);

//===== CALL #24
 inline void sngCALL24(void);

//===== CALR $+3+d16
 inline void sngCALR(void);

//===== LD R, n
 inline void sngLDB(void);

//===== PUSH RR
 inline void sngPUSHW(void);

//===== LD RR, nn
 inline void sngLDW(void);

//===== PUSH XRR
 inline void sngPUSHL(void);

//===== LD XRR, nnnn
 inline void sngLDL(void);

//===== POP RR
 inline void sngPOPW(void);

//===== POP XRR
 inline void sngPOPL(void);

//===== JR cc,PC + d
 inline void sngJR(void);

//===== JR cc,PC + dd
 inline void sngJRL(void);

//===== LDX dst,src
 inline void sngLDX(void);

//===== SWI num
 inline void sngSWI(void);

//=============================================================================
#endif
