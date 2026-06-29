#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../parser/parser.h"
#include "../libmmultemu/libmmultemu.h"
#include "../libmmult/libmmult.h"

extern int16_t N;
extern int16_t M[MAXN][MAXN];
extern int16_t X[MAXN];
extern int64_t Y[MAXN];

extern int reg_fd;
extern int xram_fd;
extern int mram_fd;
extern int yram_fd;

extern volatile uint32_t *reg_ba;
extern volatile uint32_t *xram_ba;
extern volatile uint32_t *mram_ba;
extern volatile uint32_t *yram_ba;
