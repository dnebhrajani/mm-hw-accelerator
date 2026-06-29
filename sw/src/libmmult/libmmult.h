#ifndef LIBMMULT_H
#define LIBMMULT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../parser/parser.h"

//------------------------------------------------------------------------------
// Addressing: Base addresses, Offsets and space sizes
//------------------------------------------------------------------------------
#define REG_BA    0x40300000  // Base address of register space
#define REG_SIZE  64*4        // 16 registers; x4 bytes
#define XRAM_BA   0x40320000  // Base address of XRAM space
#define XRAM_SIZE 256*4       // 128 locations; x4 bytes
#define YRAM_BA   0x40330000  // Base address of XRAM space
#define YRAM_SIZE 512*4       // 16384 locations; x4 bytes
#define MRAM_BA   0x40340000  // Base address of JRAM space
#define MRAM_SIZE 65536*4     // 16384 locations; x4 bytes

#define ID_OFFSET   0
#define CTL_OFFSET  1
#define STS_OFFSET  2
#define N_OFFSET    3
#define CCNT_OFFSET 4

extern int16_t N;
extern int16_t M[MAXN][MAXN];
extern int16_t X[MAXN];
extern int64_t Y[MAXN];

extern void mm_reg_wr(uint32_t, int);

extern int32_t mm_reg_rd(uint32_t);

extern int loadn_hw(ast *);

extern int loadx_hw(ast *);

extern int loadm_hw(ast *);

extern int run_hw(void);

extern int gety_hw(int64_t *, int);

extern int *readx_hw(int);

extern volatile uint32_t *map_mem(int, int, int);

#endif
