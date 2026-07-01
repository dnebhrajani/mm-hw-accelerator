//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   generate_input.c
// Author: Durga V. Nebhrajani
// Description:
//      Generates a random matrix and vector test input file for the
//      matrix multiplier. Output is printed to stdout in the format
//      expected by the mmult parser.
//
//      Usage:
//          gcc -o gen generate_input.c
//          ./gen > testdata.txt
//
//      The output can be redirected to a file to be used as input
//      to mmult:
//          ./mmult testdata.txt output.txt
//
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(void) {
  srand(time(NULL));
  int N = 16;
  
  printf("vars = %d;\n\n", N);

  printf("vector = [");
  for (int i = 0; i < N; ++i) {
    int16_t x = (rand() % 65536) - 32768;
    printf("%d", x);
    if (i != N-1) printf(",\n");
  }
  printf("];\n\n");

  printf("matrix = [");
  for (int i = 0; i < N * N; ++i) {
    int16_t x = (rand() % 65536) - 32768;
    printf("%d", x);
    if (i != N * N - 1) {
      printf(",\n");
    }
    
  }
  printf("];\n");

  return 0;
}
