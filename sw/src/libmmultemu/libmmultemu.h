//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   libmmultemu.h
// Author: Durga V. Nebhrajani
// Description:
//      Header file for the software emulator library for the Matrix
//      Multiplier Core.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------
#ifndef LIBMMULTEMU_H
#define LIBMMULTEMU_H

#include <stdio.h>
#include <stdlib.h>
#include "../parser/parser.h"

//--------------------------------------------------------------------------------
// Functions of the libmmultemu
//--------------------------------------------------------------------------------

extern int loadn_emu(ast *);

extern int loadx_emu(ast *);

extern int loadm_emu(ast *);

extern int run_emu(void);

extern void write_output(FILE *);

//extern int gety_emu(void);

#endif
