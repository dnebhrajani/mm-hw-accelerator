//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   mmult.c
// Author: Durga V. Nebhrajani
// Description:
//      Main driver for the matrix multiplier application. Parses
//      input, loads data into hardware or emulator, runs the
//      multiplication, and writes the result.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "mmult.h"

bool use_hw = true;

void initialize() {
  if (use_hw) {
    reg_ba  = map_mem(REG_BA,  REG_SIZE,  reg_fd);
    xram_ba = map_mem(XRAM_BA, XRAM_SIZE, xram_fd);
    mram_ba = map_mem(MRAM_BA, MRAM_SIZE, mram_fd);
    yram_ba = map_mem(YRAM_BA, YRAM_SIZE, yram_fd);
  }
  else {
    return;
  }
}

int loadn(ast *mdata) {
  if (use_hw) {
    return loadn_hw(mdata);
  }
  else {
    return loadn_emu(mdata);    
  }
}


int loadx(ast *mdata) {
  if (use_hw) {
    return loadx_hw(mdata);
  }
  else {
    return loadx_emu(mdata);    
  }
}


int loadm(ast *mdata) {
  if (use_hw) {
    return loadm_hw(mdata);
  }
  else {
    return loadm_emu(mdata);    
  }
}


int run(void) {
  if (use_hw) {
    return run_hw();
  }
  else {
    return run_emu();    
  }
}


int gety(int64_t *Y, int n) {
  if (use_hw) {
    return gety_hw(Y, n);
  }
  else {
    return 0;    
  }
}

void terminate() {
  if (use_hw) {
    munmap((void *)reg_ba,  REG_SIZE);
    munmap((void *)xram_ba, XRAM_SIZE);
    munmap((void *)mram_ba, MRAM_SIZE);
    munmap((void *)yram_ba, YRAM_SIZE);

    close(reg_fd);
    close(xram_fd);
    close(mram_fd);
    close(yram_fd);
  }
  else {
    return;
  }
}


int main(int argc, char **argv) {
  initialize();
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments provided. Fatal.\n");
    // usage message
    exit(1);
  }
  char *infilename = argv[1];
  char *outfilename = argv[2];

  if (!strcmp(argv[0], "mmult")) {
    printf("Running mmult on hardware.\n");
    use_hw = true;
  }
  else if (!strcmp(argv[0], "mmultemu")) {
    printf("Running mmult on emulator.\n");
    use_hw = false;
  }

  FILE *outfile = fopen(outfilename, "w");
  if (!outfile) {
    fprintf(stderr, "Unable to open output file '%s'.\n", outfilename);
    exit(1);
  }


  struct timespec start, end;
  long long elapsed_ns;

  //----------------------------------------------------------------------
  // Step 1: Parse input file
  //----------------------------------------------------------------------
  ast *mdata = parse(infilename);
  if (check_ast(mdata)) {
    free_ast(mdata);
    exit(1);
  }

  //----------------------------------------------------------------------
  // Step 2: Load ast data into structures/hw
  //----------------------------------------------------------------------

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (loadn(mdata)) {
    fprintf(stderr, "Failed to load size N. Fatal.\n");
    free_ast(mdata);
    exit(1);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  elapsed_ns =
    (end.tv_sec - start.tv_sec) * 1000000000LL +
    (end.tv_nsec - start.tv_nsec);

  printf("loadn(): %lld ns\n", elapsed_ns);

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (loadx(mdata)) {
    fprintf(stderr, "Failed to load X vector. Fatal.\n");
    free_ast(mdata);
    exit(1);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  elapsed_ns =
    (end.tv_sec - start.tv_sec) * 1000000000LL +
    (end.tv_nsec - start.tv_nsec);

  printf("loadx(): %lld ns\n", elapsed_ns);

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (loadm(mdata)) {
    fprintf(stderr, "Failed to load M matrix. Fatal.\n");
    free_ast(mdata);
    exit(1);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  elapsed_ns =
    (end.tv_sec - start.tv_sec) * 1000000000LL +
    (end.tv_nsec - start.tv_nsec);

  printf("loadm(): %lld ns\n", elapsed_ns);

  //----------------------------------------------------------------------
  // Step 3: Run matrix multiplier
  //----------------------------------------------------------------------

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (run()) {
    fprintf(stderr, "Matrix multiplication failed. Fatal.\n");
    free_ast(mdata);
    exit(1);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);

  elapsed_ns =
    (end.tv_sec - start.tv_sec) * 1000000000LL +
    (end.tv_nsec - start.tv_nsec);

  printf("run(): %lld ns\n", elapsed_ns);
  
  //----------------------------------------------------------------------
  // Step 4: Get result
  //----------------------------------------------------------------------
  if (gety(Y, mdata->N)) {
    fprintf(stderr, "Failed to get result vector Y. Fatal.\n");
    free_ast(mdata);
    exit(1); 
  }
  
  //----------------------------------------------------------------------
  // Step 5: Write output
  //----------------------------------------------------------------------
  write_output(outfile);
  free_ast(mdata);
  terminate();  
  return 0;
}
