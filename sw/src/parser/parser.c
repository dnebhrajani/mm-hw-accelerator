//--------------------------------------------------------------------------------
// Module: hwacc
// File: parser.c
// Author: Durga V. Nebhrajani
// Description:
//      Parser for data input files. Builds and returns an ast struct
//      populated with N, matrix (M) and vector (X).
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "parser.h"

ast *parse(char *fname) {
  token_type toktype;
  char *tokval;
  parse_state pstate = START;
  token *ctok = NULL;
  int toklnum;
  char *tokfile;

  
  FILE *fp = fopen(fname, "r");
  if (!fp) {
    fprintf(stderr, "parse(): Unable to open file '%s'.\n", fname);
    exit(1);
  }

  scanner scan;
  scanner_init(&scan, fp, fname);

  // malloc for ast
  ast *hwacc_ast = malloc(sizeof(ast));
  if (!hwacc_ast) {
    fprintf(stderr, "parse(): malloc failed.\n");
    exit(1);
  }
  hwacc_ast->N = 0;
 
  // malloc for vec and mat also - max sizes.
  bool is_vec = false;
  bool seen_vars   = false;
  bool seen_vector = false;
  bool seen_matrix = false;
  
  hwacc_ast->X = calloc(MAXN, sizeof(int16_t));
  hwacc_ast->M = calloc(MAXN*MAXN, sizeof(int16_t));

  if (!hwacc_ast->X || !hwacc_ast->M) {
    fprintf(stderr, "parse(): malloc failed.\n");
    free(hwacc_ast);
    fclose(fp);
    exit(1);
  }

  while (1) {
    ctok = next_token(&scan);
    if (ctok->type == TOK_EOF) {
//      free(ctok);
      break;
    }
    toktype = ctok->type;
    tokval  = ctok->value;
    toklnum = ctok->linenum;
    tokfile = ctok->filename;
    
    //----------------------------------------------------------------------
    // STATE == START
    //----------------------------------------------------------------------
    if (pstate == START) {
      if (toktype == TOK_KEYWORD && strcmp(tokval, "vars") == 0) {
        if (seen_vars) {
          fprintf(stderr,
                  "Semantic Error: 'vars' may only be defined once.\n");
          goto error;

        }
        seen_vars = true;
        pstate = KW_VAR;
      }
      else if ((toktype == TOK_KEYWORD) && strcmp(tokval, "vector") == 0) {
        if (seen_vector) {
          fprintf(stderr,
                  "Semantic Error: 'vector' may only be defined once.\n");
          goto error;

        }
        seen_vector = true;
        pstate = KW_VEC;
        hwacc_ast->lX = 0;
        is_vec = true;
      }
      else if ((toktype == TOK_KEYWORD) && strcmp(tokval, "matrix") == 0) {
        if (seen_matrix) {
          fprintf(stderr,
                  "Semantic Error: 'matrix' may only be defined once.\n");
          goto error;

        }
        seen_matrix = true;
        pstate = KW_VEC;
        hwacc_ast->lM = 0;
        is_vec = false;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected keyword, received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;
      }
    }

    //----------------------------------------------------------------------
    // STATE == KW_VAR
    //----------------------------------------------------------------------
    else if (pstate == KW_VAR) {
      if (toktype == TOK_EQUALS) {
        pstate = VARS_EQ;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected '=', received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;
      }
    }

    //----------------------------------------------------------------------
    // STATE == VARS_EQ, got '=', expecting number
    //----------------------------------------------------------------------
    else if (pstate == VARS_EQ) {
      if (toktype == TOK_NUMBER) {
        hwacc_ast->N = (int16_t)atoi(tokval);
        pstate = NUMBER;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected number, received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;
        
      }
    }
    
    //----------------------------------------------------------------------
    // STATE == NUMBER, got number, expecting ';'
    //----------------------------------------------------------------------
    else if (pstate == NUMBER) {
      if (toktype == TOK_SEMICOLON) {
        // got semicolon
        pstate = START;
        
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected ';', received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;

      }
    }

    
    //----------------------------------------------------------------------
    // STATE == KW_VEC
    //----------------------------------------------------------------------
    else if (pstate == KW_VEC) {
      if (toktype == TOK_EQUALS) {
        pstate = SYM_EQ;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected '=', received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;
                
      }
    }

    //----------------------------------------------------------------------
    // STATE == SYM_EQ, got '=', expecting '['
    //----------------------------------------------------------------------
    else if (pstate == SYM_EQ) {
      if (toktype == TOK_LBRACKET) {
        pstate = SYM_LBR;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected number, received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;

      }
    }
    
    //----------------------------------------------------------------------
    // STATE == SYM_LBR, got '[,  expecting number
    //----------------------------------------------------------------------
    else if (pstate == SYM_LBR) {
      if (toktype == TOK_NUMBER) {
        if (is_vec) {
          hwacc_ast->X[(hwacc_ast->lX)++] = (int16_t)atoi(tokval);
        }
        else {
          hwacc_ast->M[(hwacc_ast->lM)++] = (int16_t)atoi(tokval);
        }
        pstate = ELEMENT;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected number, received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;

      }
    }

    //----------------------------------------------------------------------
    // STATE == ELEMENT, got number, expecting ',' or ']'
    //----------------------------------------------------------------------
    else if (pstate == ELEMENT) {
      if (toktype == TOK_COMMA) {
        // number must follow a comma
        pstate = SYM_LBR;
        
      }
      else if (toktype == TOK_RBRACKET) {
        pstate = SYM_RBR;
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected ';' or ']', received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;

      }
    }

    //----------------------------------------------------------------------
    // STATE == SYM_RBR, got vector, expecting ';'
    //----------------------------------------------------------------------
    else if (pstate == SYM_RBR) {
      if (toktype == TOK_SEMICOLON) {
        // got semicolon
        pstate = START;
        
      }
      else {
        // Error out
        fprintf(stderr,
                "File %s, Line %d, Syntax Error: Expected ';', received '%s'.\n",
                tokfile, toklnum, tokval);
        goto error;

      }
    }
//    if (ctok) {
//      free(ctok);
//    }
  }

  fclose(fp);
  return hwacc_ast;

error:
  if (ctok) {
//    free(ctok);
  }
//  fclose(fp);
//  free_ast(hwacc_ast);
  exit(1);
}

int check_ast(ast *hwacc_ast) {
  if (hwacc_ast->N <= 0) {
    fprintf(stderr,
            "Semantic Error: vars must be greater than 0 (got %d).\n",
            hwacc_ast->N);
    return 1;
  }

  if (hwacc_ast->N > MAXN) {
    fprintf(stderr,
            "Semantic Error: vars must be lesser than or equal to %d (got %d).\n",
            MAXN, hwacc_ast->N);
    return 1;
  }

  if (hwacc_ast->lX != hwacc_ast->N) {
    fprintf(stderr,
            "Semantic Error: vector contains %d elements, expected %d.\n",
            hwacc_ast->lX, hwacc_ast->N);
    return 1;
  }

  if (hwacc_ast->lM != hwacc_ast->N * hwacc_ast->N) {
    fprintf(stderr,
            "Semantic Error: matrix contains %d elements, expected %d.\n",
            hwacc_ast->lM, hwacc_ast->N * hwacc_ast->N);
    return 1;
  }
  
  return 0;
}

void print_ast(ast *hwacc_ast) {

  printf("Value of N is: %d\n", hwacc_ast->N);

  printf("Values of X are:\n");
  for (int i = 0; i < hwacc_ast->lX; i++) {
    printf("%d ", hwacc_ast->X[i]);
  }
  printf("\n");

  printf("Values of M are:\n");
  for (int i = 0; i < hwacc_ast->N; i++) {
    for (int j = 0; j < hwacc_ast->N; j++) {
      printf("%d ", hwacc_ast->M[i * hwacc_ast->N + j]);
    }
    printf("\n");
  }
}

void free_ast(ast *hwacc_ast) {
  if (!hwacc_ast) {
    return;
  }

  free(hwacc_ast->X);
  free(hwacc_ast->M);
  free(hwacc_ast);
}
