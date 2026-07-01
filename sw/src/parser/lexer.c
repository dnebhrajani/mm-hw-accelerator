//--------------------------------------------------------------------------------
// Module: mm_hw_accelerator
// File:   lexer.c
// Author: Durga V. Nebhrajani
// Description:
//      Implementation file for the character-level scanner/lexer.
//      Reads from a FILE* one character at a time and produces a
//      stream of typed tokens. Intended for use by parser.c to
//      tokenize matrix, vector, and N input files.
// Copyright:
//      Copyright (c) 2026 Durga V. Nebhrajani
//      All rights reserved.
//--------------------------------------------------------------------------------

#include "lexer.h"

char filename[64] = "";

//--------------------------------------------------------------------------------
// Internal Function: peek_char
//
// Description:
//      Reads the next character from the file stream without consuming
//      it, leaving the stream position unchanged via ungetc().
//      Used to identify a leading '-' as a negative number sign
//      versus a standalone minus character.
//
// Parameters:
//      scan:   Pointer to the active scanner instance.
//
// Returns:
//      The next character in the stream as an int, or EOF if the
//      stream is exhausted.
//--------------------------------------------------------------------------------

static int peek_char(scanner *scan) {
  int c = fgetc(scan->fp);
  if (c != EOF) {
    ungetc(c, scan->fp);
  }
  return c;
}

//--------------------------------------------------------------------------------
// Internal Function: skip_comment
//
// Description:
//      Advances the scanner past a single-line commen that
//      begins with '//' and extends to the end of the line.
//      If the current character is not '/', this function does nothing.
//      If only one '/' is found (not a full '//'), the scanner is
//      left on the character after the first '/', and no skipping
//      occurs.
//
// Parameters:
//      scan:   Pointer to the active scanner instance.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------

static void skip_comment(scanner *scan) {
  if (scan->curr != '/') {
    return;
  }
  if (peek_char(scan) != '/') {
    return;
  }
  while (scan->curr != '\n' && scan->curr != EOF) {
    scan->curr = fgetc(scan->fp);
  }
}

//--------------------------------------------------------------------------------
// Internal Function: skip_whitespace
//
// Description:
//      Advances the scanner past any whitespace characters.
//      Stops at the first non-whitespace character or EOF.
//
// Parameters:
//      scan:   Pointer to the active scanner instance.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------

static void skip_whitespace(scanner *scan) {
  while (scan->curr != EOF && isspace(scan->curr)) {
    if (scan->curr == '\n') {
      scan->linenum++;
    }
    scan->curr = fgetc(scan->fp);
  }
}

//--------------------------------------------------------------------------------
// Internal Function: skip_noise
//
// Description:
//      Advances the scanner past any combination of whitespace and
//      single-line comments. Alternates between skip_whitespace and
//      skip_comment until a meaningful character or EOF is reached.
//      Handles inline comments (text followed by '//' on the same
//      line) as well as consecutive comment lines.
//
// Parameters:
//      scan:   Pointer to the active scanner instance.
//
// Returns:
//      None.
//--------------------------------------------------------------------------------

static void skip_noise(scanner *scan) {
  skip_whitespace(scan);
  while (scan->curr == '/') {
    skip_comment(scan);
    skip_whitespace(scan);
  }
}

//--------------------------------------------------------------------------------
// Internal Function: token_type_str
//
// Description:
//      Converts token_type enum (stored internally as int) to equivalent string
//
// Parameters:
//      type:   token_type enum that needs conversion to string.
//
// Returns:
//      The string equivalent to the token_type enum, UNKNOWN by default.
//--------------------------------------------------------------------------------

const char *token_type_str(token_type type) {
  switch(type) {
  case TOK_NAME:
    return "NAME";
  case TOK_KEYWORD:
    return "KEYWORD";
  case TOK_EQUALS:
    return "EQUALS";
  case TOK_NUMBER:
    return "NUMBER";
  case TOK_COMMA:
    return "COMMA";
  case TOK_SEMICOLON:
    return "SEMICOLON";
  case TOK_LBRACKET:
    return "LBRACKET";
  case TOK_RBRACKET:
    return "RBRACKET";
  case TOK_EOF:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}

//--------------------------------------------------------------------------------
// Function: scanner_init
//
// Description:
//      Initialises a scanner instance by associating it with an open
//      FILE*. Reads the first byte from the stream.
//
// Parameters:
//      scan:   Pointer to the scanner instance to initialise.
//      fp:     Pointer to an already-opened FILE for reading.
//
// Returns:
//      None.
//
// Error Handling:
//      Caller is responsible for ensuring fp is non-NULL and open
//      for reading prior to calling this function.
//
// Usage Example:
//      scanner scan;
//      scanner_init(&scan, fopen("matrix.txt", "r"));
//--------------------------------------------------------------------------------

void scanner_init(scanner *scan, FILE *fp, const char *fname) {
  scan->fp = fp;
  scan->curr = fgetc(fp);
  scan->linenum = 1;
  scan->filename = fname;
}

//--------------------------------------------------------------------------------
// Function: next_token
//
// Description:
//      Scans the input stream and returns the next token. Whitespace
//      is skipped before each token. Recognised token types are:
//
//          TOK_EOF        End of file.
//          TOK_NAME       Sequence of letters and underscores.
//          TOK_KEYWORD    Sequence of letters that form a keyword.
//          TOK_NUMBER     Optional leading '-' followed by digits.
//          TOK_EQUALS     '='
//          TOK_COMMA      ','
//          TOK_SEMICOLON  ';'
//          TOK_LBRACKET   '['
//          TOK_RBRACKET   ']'
//          TOK_UNK        Any unrecognised character.
//
//      The token value string is always null-terminated and at most
//      sizeof(tok.value) - 1 characters long; longer are
//      silently truncated.
//
// Parameters:
//      scan:   Pointer to the active scanner instance.
//
// Returns:
//      A token struct with type and value fields populated.
//
// Error Handling:
//      Unrecognised characters produce a TOK_UNK; the parser decides
//      how to handle them. Truncation of overlong lexemes is silent.
//
// Usage Example:
//      token tok = next_token(&scan);
//      if (tok.type == TOK_NUMBER) { ... }
//--------------------------------------------------------------------------------

token *next_token(scanner *scan) {
//  token *tok = malloc(sizeof(token));
  static token tok;

  int i = 0;

  skip_noise(scan);

  tok.linenum = scan->linenum;
  tok.filename = (char *)scan->filename;

  // EOF
  if (scan->curr == EOF) {
    tok.type = TOK_EOF;
    tok.value[0] = '\0';
    return &tok;
  }

  // TOK_NAME: letters and underscores
  if (isalpha(scan->curr) || scan->curr == '_') {
    bool invalid = false;

    tok.value[i++] = (char)scan->curr;
    scan->curr = fgetc(scan->fp);

    while (scan->curr != EOF &&
           (isalnum(scan->curr) || scan->curr == '_') &&
           i < (int16_t)sizeof(tok.value) - 1) {

      if (isdigit(scan->curr)) {
        invalid = true;
      }

      tok.value[i++] = (char)scan->curr;
      scan->curr = fgetc(scan->fp);
    }

    tok.value[i] = '\0';

    if (invalid) {
      tok.type = TOK_UNK;
      return &tok;
    }

    if (strcmp(tok.value, "vars") == 0 ||
        strcmp(tok.value, "matrix") == 0 ||
        strcmp(tok.value, "vector") == 0) {
      tok.type = TOK_KEYWORD;
    }
    else {
      tok.type = TOK_NAME;
    }

    return &tok;
  }

  // TOK_NUMBER: optional '-' followed by one or more digits
  if (isdigit(scan->curr) ||
      (scan->curr == '-' && isdigit(peek_char(scan)))) {

    bool invalid = false;

    if (scan->curr == '-') {
      tok.value[i++] = '-';
      scan->curr = fgetc(scan->fp);
    }

    while (scan->curr != EOF &&
           (isalnum(scan->curr) || scan->curr == '_') &&
           i < (int)sizeof(tok.value) - 1) {

      if (!isdigit(scan->curr)) {
        invalid = true;
      }

      tok.value[i++] = (char)scan->curr;
      scan->curr = fgetc(scan->fp);
    }

    tok.value[i] = '\0';

    if (invalid) {
      tok.type = TOK_UNK;
    }
    else {
      tok.type = TOK_NUMBER;
    }

    return &tok;
  }

  // Single-character punctuation tokens
  switch (scan->curr) {
  case '=': tok.type = TOK_EQUALS;
    strcpy(tok.value, "="); break;
  case ',': tok.type = TOK_COMMA;
    strcpy(tok.value, ","); break;
  case ';': tok.type = TOK_SEMICOLON;
    strcpy(tok.value, ";"); break;
  case '[': tok.type = TOK_LBRACKET;
    strcpy(tok.value, "["); break;
  case ']': tok.type = TOK_RBRACKET;
    strcpy(tok.value, "]"); break;
  default:
    tok.type     = TOK_UNK;
    tok.value[0] = (char)scan->curr;
    tok.value[1] = '\0';
    break;
  }
  scan->curr = fgetc(scan->fp);
  return &tok;
}

/* int main(void) { */
/*   FILE *fp = fopen("../test/tst1.txt", "r"); */
/*   if (!fp) { */
/*     printf("Unable to open input file.\n"); */
/*     exit(1); */
/*   } */

/*   scanner scan; */
/*   scanner_init(&scan, fp); */

/*   token tok; */

/*   for (tok = next_token(&scan); tok->type != TOK_EOF; */
/*        tok = next_token(&scan)) { */
/*     printf("%s %s\n", token_type_str(tok->type), */
/*                                tok->value); */
/*   } */

/*   fclose(fp); */
/*   return 0; */
/* } */
