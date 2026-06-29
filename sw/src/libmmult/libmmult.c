#include "libmmult.h"

int reg_fd;
int xram_fd;
int mram_fd;
int yram_fd;

volatile uint32_t *reg_ba;
volatile uint32_t *xram_ba;
volatile uint32_t *mram_ba;
volatile uint32_t *yram_ba;

volatile uint32_t *map_mem(int base_addr, int size, int fd) {
  volatile uint32_t *mem_ptr;

  // Open /dev/mem to access physical memory
  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  // Map Register space region to virtual memory
  mem_ptr = (volatile uint32_t *)mmap(NULL, size,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED, fd, base_addr);
  if (mem_ptr == MAP_FAILED) {
    perror("mmap");
    close(fd);
    exit(1);
  }

  return mem_ptr;
}


//------------------------------------------------------------------------------
// Function Name: im_reg_wr()
// Description: Performs a single write access to the register space.
// Arguments: 1. Address offset - must be one of the register offsets -
// (unsigned 32 bit integer). Valid offsets are defined
// in libising.h and are:
// IM_CTRL_OFFSET 0
// LOOPS_OFFSET 1
// CCOUNT_OFFSET 2
// ALPHA_OFFSET 4
// BETA_OFFSET 5
// SPINS_OFFSET 6
// 2. Data to be written (32 bit integer)
// Returns: Nothing
// Usage: im_reg_wr(IM_CTRL_OFFSET, 1);
//------------------------------------------------------------------------------
void mm_reg_wr(uint32_t addr_offset, int32_t value) {
  *(reg_ba + addr_offset) = value;
}

//------------------------------------------------------------------------------
// Function Name: im_reg_rd()
// Description: Performs a single read access to the register space.
// Arguments: 1. Address offset - must be one of the register offsets -
// (unsigned 32 bit integer). Valid offsets are defined
// in libising.h and are:
// IM_CTRL_OFFSET 0
// LOOPS_OFFSET 1
// CCOUNT_OFFSET 2
// ALPHA_OFFSET 4
// BETA_OFFSET 5
// SPINS_OFFSET 6
// Returns: Data read from the register
// Usage: int read_data = im_reg_rd(IM_CTRL_OFFSET);
//------------------------------------------------------------------------------
int32_t mm_reg_rd(uint32_t addr_offset) {
  int32_t rdval;
  rdval = *(reg_ba + addr_offset);
  // printf("Got read value %0x from addr_offset %d\n", rdval, addr_offset);
  return rdval;
}

int loadn_hw(ast *mdata) {
  N = mdata->N;
  mm_reg_wr(N_OFFSET, mdata->N);
  return 0;
}

//--------------------------------------------------------------------------------
// Function: loadx_emu
//
// Description:
//      Populates the vector array. Entries outside the
//      region covered by N are left as 0.
//
// Parameters:
//      mdata:   struct ast containing required values.
//
// Returns:
//      integer indicating success or failure.
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
//      Populates the vector array. Entries outside the
//      region covered by N are left as 0.
//
// Parameters:
//      mdata:   struct ast containing required values.
//
// Returns:
//      integer indicating success or failure.
//
// Time Complexity:
//      O(n)
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

int *readx_hw(int n) {
  int *xram_vals = malloc(sizeof(int) * n);
  for (int i = 0; i < n; ++i) {
    xram_vals[i] = xram_ba[i];
  }

  return xram_vals;
}

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

int run_hw(void) {
  mm_reg_wr(CTL_OFFSET, 1);

  int done = 0;
  while (done == 0) {
    done = mm_reg_rd(STS_OFFSET);
  }
  return 0;
}
