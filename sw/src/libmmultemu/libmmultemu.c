//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   libmmultemu.c
// Author: Durga V. Nebhrajani
// Description:
//      Software emulator library for the Matrix Multiplier Core.
//      Provides load, run, and output functions that execute matrix-
//      vector multiplication in software without FPGA hardware.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "libmmultemu.h"

int16_t N;
int16_t M[MAXN][MAXN];
int16_t X[MAXN];
int64_t Y[MAXN];

 
//--------------------------------------------------------------------------------
// Error Handling Infrastructure
//--------------------------------------------------------------------------------
 
typedef enum {
  EM_NOTE,
  EM_WARNING,
  EM_ERROR,
  EM_FATAL
} em_error_level;
 
typedef struct {
  const char    *code;
  const char    *message;
  em_error_level severity;
} em_error;
 
//--------------------------------------------------------------------------------
// Internal Function: _printerr
//
// Description:
//      Prints a formatted error message to stderr using the provided
//      error struct and the name of the calling function.
//
// Parameters:
//      err:    Pointer to em_error struct containing code, message,
//              and severity level.
//      func:   String containing the name of the function that
//              encountered the error.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------
 
static void _printerr(const em_error *err, const char *func) {
  const char *severity;
  switch (err->severity) {
  case EM_NOTE:
    severity = "NOTE";
    break;
  case EM_WARNING:
    severity = "WARNING";
    break;
  case EM_FATAL:
    severity = "FATAL";
    break;
  default:
    severity = "ERROR";
    break;
  }
  fprintf(stderr,
          "[%s] [%s] %s(): %s\n",
          severity,
          err->code,
          func,
          err->message);
}

//--------------------------------------------------------------------------------
// Function: loadn_emu
//
// Description:
//      Populates the N integer indication dimension
//
// Parameters:
//      mdata:   struct ast containing required values.
//
// Returns:
//      integer indicating success or failure.
//
// Time Complexity:
//      O(1)
//--------------------------------------------------------------------------------
int loadn_emu(ast *mdata) {
  N = mdata->N;
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
int loadx_emu(ast *mdata) {
  for (int i = 0; i < mdata->lX; ++i) {
    X[i] = mdata->X[i];
  }
  return 0;
}

//--------------------------------------------------------------------------------
// Function: loadm_emu
//
// Description:
//      Populates the matrix 2D array. Entries outside the
//      region covered by N are left as 0.
//
// Parameters:
//      mdata:   struct ast containing required values.
//
// Returns:
//      integer indicating success or failure.
//
// Time Complexity:
//      O(n^2)
//--------------------------------------------------------------------------------
int loadm_emu(ast *mdata) {
  for (int i = 0; i < mdata->N; ++i) {
    for (int j = 0; j < mdata->N; ++j) {
      M[i][j] = mdata->M[i*(mdata->N) + j];
    }
  }
  return 0;
}

//--------------------------------------------------------------------------------
// Function: write_output
//
// Description:
//      Write result vector to provided file
//
// Parameters:
//      outfile:   FILE ptr to output file 
//
// Returns:
//      None.
//
// Time Complexity:
//      O(n)
//--------------------------------------------------------------------------------

void write_output(FILE* outfile) {
  fprintf(outfile, "Value of N is: %d\n", N);

  fprintf(outfile, "Values of X are:\n");
  for (int i = 0; i < N; ++i) {
    fprintf(outfile, "%d\n", X[i]);
  }
  fprintf(outfile, "\n\n");

  fprintf(outfile, "Values of M are:\n");
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      fprintf(outfile, "%d\n", M[i][j]);
    }
    fprintf(outfile, "\n\n");
  }

  fprintf(outfile, "Values of Y are:\n");
  for (int i = 0; i < N; ++i) {
    fprintf(outfile, "%lld\n", Y[i]);
  }
  fprintf(outfile, "\n");
  return;
}
 
//--------------------------------------------------------------------------------
// Internal Function: m_multiply
//
// Description:
//      Computes the product of the global N x N matrix M
//      and the global N x 1 vector X, storing the output in the global
//      Y vector.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------
 
void m_multiply(void) {
  for (int16_t i = 0; i < N; i++) {
    Y[i] = 0;
    for (int16_t j = 0; j < N; j++)
      Y[i] += (int64_t)M[i][j] * X[j];
  }
}

int run_emu(void) {
  static const em_error EM_ERR_INVALID_N = {
    "E01",
    "Invalid N: Provided value of N is outside allowed range.",
    EM_FATAL
  };
  if (N < 1 || N > MAXN) {
    _printerr(&EM_ERR_INVALID_N, "run_emu");
    exit(1);
  }

  m_multiply();
  return 0;
}
