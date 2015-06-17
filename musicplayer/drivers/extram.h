//*****************************************************************************
//
// extram.h - Functions related to initialisation and management of external
//            RAM.
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

#ifndef __EXTRAM_H__
#define __EXTRAM_H__

#include "inc/hw_types.h"

//*****************************************************************************
//
// Public function prototypes.
//
//*****************************************************************************
extern tBoolean SDRAMInit(unsigned long ulEPIDivider, unsigned long ulConfig,
                          unsigned long ulRefresh);
extern tBoolean ExtRAMHeapInit(void);
extern void *ExtRAMAlloc(unsigned long ulSize);
extern void ExtRAMFree(void *pvBlock);
extern unsigned long ExtRAMMaxFree(unsigned long *pulTotalFree);

//*****************************************************************************
//
// With the addition of external RAM on other daughter boards in addition to the
// basic SDRAM daughter, this driver and various function it contains were
// renamed.  The previous functions are deprecated but defined here to aid
// backwards compatibility.
//
//*****************************************************************************
#ifndef DEPRECATED
#define SRAMHeapInit(a) ExtRAMHeapInit(a)
#define SDRAMAlloc(a) ExtRAMAlloc(a)
#define SDRAMFree(a) ExtRAMFree(a)
#define SDRAMMaxFree(a) ExtRAMMaxFree(a)
#endif

#endif // __EXTRAM_H__
