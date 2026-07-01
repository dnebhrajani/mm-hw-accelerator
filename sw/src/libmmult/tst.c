//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   tst.c
// Author: Durga V. Nebhrajani
// Description:
//      Quick smoke-test for the hardware library. Loads a 10x10
//      identity matrix, runs the multiplier, and prints Y.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "libmmult.h"
  
int main(void) {
  int16_t X[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int16_t M[] = {
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 2
  };
  ast test_data;
  test_data.N = 10;
  test_data.lX = 10;
  test_data.lM = 100;  
  test_data.X = X;
  test_data.M = M;

  loadn_hw(&test_data);
  
  loadx_hw(&test_data);

  loadm_hw(&test_data);

  run_hw();

  int64_t Y[256];
  gety_hw(Y, test_data.N);

  for (int i = 0; i < test_data.N; ++i) {
    printf("%lld ", Y[i]);
  }
  printf("\n");
  
}
