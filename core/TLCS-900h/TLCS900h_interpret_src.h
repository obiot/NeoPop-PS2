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

	TLCS900h_interpret_src.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_SRC__
#define __TLCS900H_SRC__
//=========================================================================

//===== PUSH (mem)
inline void srcPUSH(void);

//===== RLD A,(mem)
inline void srcRLD(void);

//===== RRD A,(mem)
inline void srcRRD(void);

//===== LDI
inline void srcLDI(void);

//===== LDIR
inline void srcLDIR(void);

//===== LDD
inline void srcLDD(void);

//===== LDDR
inline void srcLDDR(void);

//===== CPI
inline void srcCPI(void);

//===== CPIR
inline void srcCPIR(void);

//===== CPD
inline void srcCPD(void);

//===== CPDR
inline void srcCPDR(void);

//===== LD (nn),(mem)
inline void srcLD16m(void);

//===== LD R,(mem)
inline void srcLD(void);

//===== EX (mem),R
inline void srcEX(void);

//===== ADD (mem),#
inline void srcADDi(void);

//===== ADC (mem),#
inline void srcADCi(void);

//===== SUB (mem),#
inline void srcSUBi(void);

//===== SBC (mem),#
inline void srcSBCi(void);

//===== AND (mem),#
inline void srcANDi(void);

//===== OR (mem),#
inline void srcORi(void);

//===== XOR (mem),#
inline void srcXORi(void);

//===== CP (mem),#
inline void srcCPi(void);

//===== MUL RR,(mem)
inline void srcMUL(void);

//===== MULS RR,(mem)
inline void srcMULS(void);

//===== DIV RR,(mem)
inline void srcDIV(void);

//===== DIVS RR,(mem)
inline void srcDIVS(void);

//===== INC #3,(mem)
inline void srcINC(void);

//===== DEC #3,(mem)
inline void srcDEC(void);

//===== RLC (mem)
inline void srcRLC(void);

//===== RRC (mem)
inline void srcRRC(void);

//===== RL (mem)
inline void srcRL(void);

//===== RR (mem)
inline void srcRR(void);

//===== SLA (mem)
inline void srcSLA(void);

//===== SRA (mem)
inline void srcSRA(void);

//===== SLL (mem)
inline void srcSLL(void);

//===== SRL (mem)
inline void srcSRL(void);

//===== ADD R,(mem)
inline void srcADDRm(void);

//===== ADD (mem),R
inline void srcADDmR(void);

//===== ADC R,(mem)
inline void srcADCRm(void);

//===== ADC (mem),R
inline void srcADCmR(void);

//===== SUB R,(mem)
inline void srcSUBRm(void);

//===== SUB (mem),R
inline void srcSUBmR(void);

//===== SBC R,(mem)
inline void srcSBCRm(void);

//===== SBC (mem),R
inline void srcSBCmR(void);

//===== AND R,(mem)
inline void srcANDRm(void);

//===== AND (mem),R
inline void srcANDmR(void);

//===== XOR R,(mem)
inline void srcXORRm(void);

//===== XOR (mem),R
inline void srcXORmR(void);

//===== OR R,(mem)
inline void srcORRm(void);

//===== OR (mem),R
inline void srcORmR(void);

//===== CP R,(mem)
inline void srcCPRm(void);

//===== CP (mem),R
inline void srcCPmR(void);

//=============================================================================
#endif
