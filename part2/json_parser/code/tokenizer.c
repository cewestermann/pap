#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_TOKENS 1024

const char* token_types[NUM_TYPES] = {
  "L_BRACE",
  "R_BRACE",
  "L_BRACKET",
  "R_BRACKET",
  "COLON",
  "COMMA",
  "STRING",
  "INT",
  "FLOAT",
  "LITERAL",
  "END_OF_FILE",
};

static void tokenize_string(FILE* file, Token* current_token) {
  char string_buffer[32];            
  char* strp = string_buffer;

  size_t count = 0;

  int c;
  while ((c = fgetc(file)) != EOF && c != '"') {
    *strp++ = (char)c;
    count++;
  }
  *strp = '\0';

  current_token->type = STRING;
  current_token->value.string = string_buffer;
}

static void tokenize_factor(FILE* file, Token* current_token, int c) {
  char num_buffer[32];
  char* nump = num_buffer;

  bool is_negative = false;
  bool is_f64 = false;

  size_t count = 0;

  if (c == '-')
    is_negative = true;

  *nump++ = (char)c;

  while ((c = fgetc(file)) != EOF && c != ',') {
    if (count > sizeof(num_buffer) - 1) {
      fprintf(stderr, "buffer overflow\n");
      break;
    }
    *nump++ = (char)c;
    count++;
  }

  *nump = '\0';

  f64 number = strtod(num_buffer, NULL);

  printf("%.16f\n", number);

  current_token->type = FLOAT;
  current_token->value.number = number;
}

Token tokenize(FILE* file) {
  Token token = { 0 };

  int c;
  while ((c = fgetc(file)) != EOF && isspace((u8)c));

  if (c == EOF) {
    token.type = END_OF_FILE;
    return token;
  }

  switch (c) {
    case '{': token.type = L_BRACE; break;
    case '}': token.type = R_BRACE; break;
    case '[': token.type = L_BRACKET; break;
    case ']': token.type = R_BRACKET; break;
    case ':': token.type = COLON; break;
    case ',': token.type = COMMA; break;
    case '"': tokenize_string(file, &token); break;
    case '-': 
    case '0': 
    case '1': 
    case '2': 
    case '3': 
    case '4':
    case '5': 
    case '6': 
    case '7': 
    case '8': 
    case '9': tokenize_factor(file, &token, c); break;
    default:
      {
        fprintf(stderr, "No such character is accounted for: %c\n", (char)c);
      }
  }
  return token;
}

