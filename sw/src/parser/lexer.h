//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   lexer.h
// Author: Durga V. Nebhrajani
// Description:
//      Header file for the character-level scanner/lexer.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#ifndef LEXER_H
#define LEXER_H


#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
  TOK_NAME,
  TOK_KEYWORD,
  TOK_EQUALS,
  TOK_NUMBER,
  TOK_COMMA,
  TOK_SEMICOLON,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_EOF,
  TOK_UNK
} token_type;

typedef struct {
  token_type type;
  char value[1024];
  int linenum;
  char *filename;
} token;

typedef struct {
  FILE *fp;
  int curr;
  int linenum;
  const char *filename;
} scanner;

extern void scanner_init(scanner *, FILE *, const char *);

extern token *next_token(scanner *);

#endif
