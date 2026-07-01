//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   libmmult.c
// Author: Durga V. Nebhrajani
// Description:
//      Hardware interface library for the Matrix Multiplier Core.
//      Provides memory-mapped register, XRAM, MRAM, and YRAM access
//      functions that communicate with the FPGA accelerator via
//      /dev/mem.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "libmmult.h"

int reg_fd;
int xram_fd;
int mram_fd;
int yram_fd;

volatile uint32_t *reg_ba;
volatile uint32_t *xram_ba;
volatile uint32_t *mram_ba;
volatile uint32_t *yram_ba;

//--------------------------------------------------------------------------------
// Function: map_mem
//
// Description:
//      Opens /dev/mem and maps a physical address region into the
//      process virtual address space. The resulting file descriptor
//      is written back through the fd pointer so the caller can
//      close it later.
//
// Parameters:
//      base_addr:  Physical base address to map.
//      size:       Size of the region in bytes.
//      fd:         Pointer to int that receives the /dev/mem fd.
//
// Returns:
//      Pointer to the mapped memory region.
//--------------------------------------------------------------------------------

volatile uint32_t *map_mem(int base_addr, int size, int *fd) {
  volatile uint32_t *mem_ptr;

  // Open /dev/mem to access physical memory
  *fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (*fd < 0) {
    perror("open");
    exit(1);
  }

  // Map Register space region to virtual memory
  mem_ptr = (volatile uint32_t *)mmap(NULL, size,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED, *fd, base_addr);
  if (mem_ptr == MAP_FAILED) {
    perror("mmap");
    close(*fd);
    exit(1);
  }

  return mem_ptr;
}

//--------------------------------------------------------------------------------
// Function: mm_reg_wr
//
// Description:
//      Performs a single 32-bit write to the Matrix Multiplier Core
//      register space.
//
// Parameters:
//      addr_offset:  Word offset of the target register.
//      value:        32-bit value to write.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------

void mm_reg_wr(uint32_t addr_offset, int32_t value) {
  *(reg_ba + addr_offset) = value;
}

//--------------------------------------------------------------------------------
// Function: mm_reg_rd
//
// Description:
//      Performs a single 32-bit read from the Matrix Multiplier Core
//      register space.
//
// Parameters:
//      addr_offset:  Word offset of the target register.
//
// Returns:
//      32-bit value read from the register.
//--------------------------------------------------------------------------------

int32_t mm_reg_rd(uint32_t addr_offset) {
  int32_t rdval;
  rdval = *(reg_ba + addr_offset);
  // printf("Got read value %0x from addr_offset %d\n", rdval, addr_offset);
  return rdval;
}

//--------------------------------------------------------------------------------
// Function: loadn_hw
//
// Description:
//      Writes the matrix dimension N to the hardware register and
//      stores it in the global variable.
//
// Parameters:
//      mdata:  Pointer to the parsed AST.
//
// Returns:
//      0 on success.
//--------------------------------------------------------------------------------

int loadn_hw(ast *mdata) {
  N = mdata->N;
  mm_reg_wr(N_OFFSET, mdata->N);
  return 0;
}

//--------------------------------------------------------------------------------
// Function: loadx_hw
//
// Description:
//      Writes the input vector X to XRAM and the global X array.
//      Entries beyond the vector length are zero-filled.
//
// Parameters:
//      mdata:  Pointer to the parsed AST.
//
// Returns:
//      0 on success.
//
// Time Complexity:
//      O(n)
//--------------------------------------------------------------------------------

int loadx_hw(ast *mdata) {
  for (int i = 0; i < mdata->lX; ++i) {
    xram_ba[i] = mdata->X[i];
    X[i] = mdata->X[i]; // Write to the global variable as well for later printing
  }
  for (int i = mdata->lX; i < MAXN; ++i) {
    xram_ba[i] = 0;
    X[i] = 0;
  }

  return 0;
}

//--------------------------------------------------------------------------------
// Function: loadm_hw
//
// Description:
//      Writes the input matrix M to MRAM and the global M array.
//      Matrix elements are stored in row-major order with MAXN
//      stride to match the hardware addressing.
//
// Parameters:
//      mdata:  Pointer to the parsed AST.
//
// Returns:
//      0 on success.
//
// Time Complexity:
//      O(n^2)
//--------------------------------------------------------------------------------

int loadm_hw(ast *mdata) {
  volatile uint32_t *mram_addr;
  int mram_wdata;

  for (int i = 0; i < mdata->N; ++i) {
    for (int j = 0; j < mdata->N; ++j) {
      mram_addr = mram_ba + (i*MAXN + j);
      mram_wdata = mdata->M[i*(mdata->N) + j];
//      printf("MRAM[%8x] <- %d\n", MRAM_BA+(i*MAXN + j)*4, mram_wdata);
      *mram_addr = mram_wdata;
      M[i][j] = mram_wdata; // Write to global variable for printing etc.
    }
  }

  return 0;
}

//--------------------------------------------------------------------------------
// Function: readx_hw
//
// Description:
//      Reads n elements from XRAM and returns them in a newly
//      allocated array. Caller must free the returned pointer.
//
// Parameters:
//      n:  Number of elements to read.
//
// Returns:
//      Pointer to a heap-allocated int array of size n.
//--------------------------------------------------------------------------------

int *readx_hw(int n) {
  int *xram_vals = malloc(sizeof(int) * n);
  for (int i = 0; i < n; ++i) {
    xram_vals[i] = xram_ba[i];
  }

  return xram_vals;
}

//--------------------------------------------------------------------------------
// Function: gety_hw
//
// Description:
//      Reads the result vector from YRAM. Each 40-bit result is
//      stored across two consecutive 32-bit YRAM words (lower then
//      upper), which are reassembled into a 64-bit value.
//
// Parameters:
//      Y:  Pointer to the output array to populate.
//      n:  Number of result elements to read.
//
// Returns:
//      0 on success.
//--------------------------------------------------------------------------------

int gety_hw(int64_t *Y, int n) {
  int iter = 0;
  for (int i = 0; i < 2*n; i += 2) {
    int64_t lower = yram_ba[i];
    int64_t upper = yram_ba[i+1];
    upper <<= 32;
    upper += lower;
    Y[iter++] = upper;
  }

  return 0;
}

//--------------------------------------------------------------------------------
// Function: run_hw
//
// Description:
//      Starts the hardware multiplier by writing 1 to the CTL
//      register, then polls the STS register until the done bit
//      is set.
//
// Parameters:
//      None.
//
// Returns:
//      0 on success.
//--------------------------------------------------------------------------------

int run_hw(void) {
  mm_reg_wr(CTL_OFFSET, 1);

  int done = 0;
  while (done == 0) {
    done = mm_reg_rd(STS_OFFSET);
  }
  return 0;
}
