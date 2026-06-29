#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokenizer.h"

#define MAXN 256

typedef enum {
  START,
  KW_VAR,
  KW_VEC,
  KW_MAT,
  VARS_EQ,
  SYM_LBR,
  SYM_RBR,
  SYM_EQ,
  ELEMENT,
  SYM_COMMA,
  NUMBER,
  DONE
  
} parse_state;

// typedef struct ast_t {
//   int16_t  N;
//   int16_t *X;
//   int16_t  lX;
//   int16_t *M;
//   int16_t  lM;
// } ast;

typedef struct ast_t {
  int      N;
  int16_t *X;
  int      lX;
  int16_t *M;
  int      lM;
} ast;

extern ast *parse();

extern int check_ast(ast *);

extern void print_ast(ast *);

extern void free_ast(ast *);

#endif
