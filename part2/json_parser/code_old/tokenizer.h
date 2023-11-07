#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef double f64;

typedef enum {
  L_BRACE,
  R_BRACE,
  L_BRACKET,
  R_BRACKET,
  COLON,
  COMMA,
  STRING,
  INT,
  FLOAT,
  LITERAL,
  END_OF_FILE,
  NUM_TYPES,
} TokenType;

typedef struct {
  TokenType type;
  union {
    char* string;
    f64 number;
  } value;
} Token;

Token tokenize(FILE* file);
extern const char* token_types[NUM_TYPES];

#endif // TOKENIZER_H
