#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define global static

#define MAX_TOKENS 1024

typedef uint8_t u8;
typedef int64_t i64;
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
  NUM_TYPES,
} TokenType;

global const char* token_types[NUM_TYPES] = {
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
};

typedef struct {
  TokenType type;
  union {
    char* string;
    f64 number;
  } value;
} Token;

global Token token_list[MAX_TOKENS];
global size_t token_list_idx = 0;

static void token_list_append(Token token) {
  token_list[token_list_idx++] = token;
}

static void token_list_print() {
  for (size_t i = 0; i < token_list_idx; i++) {
    printf("%s\n", token_types[token_list[i].type]);
  }
}

static void parse_string(FILE* file, Token current_token) {
  char string_buffer[32];            
  char* strp = string_buffer;

  size_t count = 0;

  int c;
  while ((c = fgetc(file)) != EOF && c != '"') {
    *strp++ = (char)c;
    count++;
  }
  *strp = '\0';

#if 0
  for (size_t i = 0; i < count; i++) {
    printf("%c", string_buffer[i]);
  }
  printf("\n");
#endif

  current_token.type = STRING;
  current_token.value.string = string_buffer;
  token_list_append(current_token);
}

static void parse_number(FILE* file, Token* current_token, int c) {
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
  token_list_append(*current_token);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: parser.exe <filename.json>\n");
    return EXIT_FAILURE;
  }

  FILE* file;
  errno_t err = fopen_s(&file, argv[1], "r");

  if (err) {
    fprintf(stderr, "Error opening file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  int c;

  while ((c = fgetc(file)) != EOF) {
    Token current_token = {0};
    if (!isspace((u8)c)) {
      switch (c) {
        case '{': 
          {
            current_token.type = L_BRACE;
            token_list_append(current_token);
          } break;
        case '}':
          {
            current_token.type = R_BRACE;
            token_list_append(current_token);
          } break;
        case '[':
          {
            current_token.type = L_BRACKET;
            token_list_append(current_token);
          } break;
        case ']':
          {
            current_token.type = R_BRACKET;
            token_list_append(current_token);
          } break;
        case ':':
          {
            current_token.type = COLON;
            token_list_append(current_token);
          } break;
        case ',':
          {
            current_token.type = COMMA;
            token_list_append(current_token);
          } break;
        case '"':
          {
            parse_string(file, current_token);
          } break;

        case '-': 
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
            parse_number(file, &current_token, c);
          } break;
      }
    }
  }
#if 1
  printf("\n");
  token_list_print();
#endif
  fclose(file);
  
  return EXIT_SUCCESS;
}



