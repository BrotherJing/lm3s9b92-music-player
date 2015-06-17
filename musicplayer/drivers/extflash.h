//*****************************************************************************
//
// extflash.h - Header file for functions accessing the external flash on the
//              lm3s9b96 SRAM/Flash daughter board.
//
// Copyright (c) 2009 Luminary Micro, Inc.  All rights reserved.
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 5228 of the DK-LM3S9B96 Firmware Package.
//
//*****************************************************************************

#ifndef __EXTFLASH_H__
#define __EXTFLASH_H__

//*****************************************************************************
//
// The base address of the external flash and SRAM on the daughter board
//
//*****************************************************************************
#define EXT_FLASH_BASE          0x60000000
#define EXT_SRAM_BASE           0x68000000

#define SRAM_MEM_SIZE 0x00100000        //Size in bytes (1MByte)

//*****************************************************************************
//
// Exported function prototypes.
//
//*****************************************************************************
extern tBoolean ExtFlashPresent(void);
extern unsigned long ExtFlashChipSizeGet(void);
extern unsigned long ExtFlashBlockSizeGet(unsigned long ulAddress,
                                          unsigned long *pulBlockStart);
extern tBoolean ExtFlashEraseIsComplete(unsigned long ulAddress,
                                        tBoolean *pbError);
extern tBoolean ExtFlashBlockErase(unsigned long ulAddress, tBoolean bSync);
extern tBoolean ExtFlashChipErase(tBoolean bSync);
extern unsigned long ExtFlashWrite(unsigned long ulAddress,
                                   unsigned long ulLength,
                                   unsigned char *pucSrc);

//*****************************************************************************
//
// The previous ExtFlashInit function has been deprecated since the relevant
// initialization is now performed within PinoutSet().  The following macro is
// defined for backwards compatibility.
//
//*****************************************************************************
#ifndef DEPRECATED
#define ExtFlashInit(a) ExtFlashPresent()
#endif

#endif // __EXTFLASH_H__
