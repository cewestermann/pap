#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define INITIAL_TOKEN_MAX 1024

#define global static

typedef double f64;
typedef uint8_t u8;

typedef enum {
  L_BRACE,
  R_BRACE,
  L_BRACKET,
  R_BRACKET,
  COLON,
  COMMA,
  STRING,
  NUMBER,
  TOKEN_NUM,
} TokenType;

const char* token_type_list[TOKEN_NUM] = {
  "L_BRACE",
  "R_BRACE",
  "L_BRACKET",
  "R_BRACKET",
  "COLON",
  "COMMA",
  "STRING",
  "NUMBER",
};

typedef struct {
  char *data;
  size_t length;
} String;

typedef struct {
  TokenType type;
  union {
    f64 number;
    String string;
  } value;
} Token;

global Token* tokens;
global size_t token_count = 0;
global size_t token_capacity = INITIAL_TOKEN_MAX;

static void tokens_init(void) {
  tokens = (Token*)malloc(INITIAL_TOKEN_MAX * sizeof(Token));
}

static void append_token(Token token) {
  if (token_count == token_capacity) {
    size_t new_capacity = token_capacity * 2;
    Token* new_tokens = (Token*)realloc(tokens, new_capacity * sizeof(Token));

    if (!new_tokens) {
      free(tokens);
      fprintf(stderr, "Failed to allocate memory for tokens\n");
      exit(EXIT_FAILURE);
    }

    tokens = new_tokens;
    token_capacity = new_capacity;
  }

  tokens[token_count++] = token;
}

static Token* parse_string(Token *token, FILE *file) {
  Token *result = token;
  String string_result = {};

  /* We know for certain that strings do not exceed 5 characters, 
   * and we make it 32 just for safety */
  char string_buffer[32];
  size_t length = 0;

  if (!string_buffer) {
    fprintf(stderr, "Failed to allocate memory for string buffer.\n");
    return result;
  }

  // TODO(chris): What about escaped "? 
  int c;
  while ((c = fgetc(file)) != EOF && c != '"')
    string_buffer[length++] = (char)c;

  string_buffer[length] = '\0';

  result->type = STRING;
  result->value.string = string_buffer;
  return result;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: parser.exe <json_filename>\n");
    return EXIT_FAILURE;
  }
  FILE* file;
  char* filename = argv[1];

  errno_t err = fopen_s(&file, filename, "r");

  if (err) {
    fprintf(stderr, "Could not open file\n");
    fclose(file);
    return EXIT_FAILURE;
  }
  tokens_init();

  int c;
  while ((c = fgetc(file)) != EOF) {
    if (!isspace((u8)c)) {
      Token current_token = {};
      switch (c) {
        case '{': 
          {
            current_token.type = L_BRACE;
            append_token(current_token);
          } break;
        case '}':
          {
            current_token.type = R_BRACE;
            append_token(current_token);
          } break;
        case '[':
          {
            current_token.type = L_BRACKET;
            append_token(current_token);
          } break;
        case ']':
          {
            current_token.type = R_BRACKET;
            append_token(current_token);
          } break;
        case ':':
          {
            current_token.type = COLON;
            append_token(current_token);
          } break;
        case ',':
          {
            current_token.type = COMMA;
            append_token(current_token);
          } break;
        case '"':
          {
            parse_string(&current_token, file);
          } break;
#if 0
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
            parse_f64(file);
          } break;
#endif
      }
    }
  }

  for (size_t i = 0; i < token_count; i++) {
    printf("Token type: %s\n", token_type_list[tokens[i].type]);
  }


  return EXIT_SUCCESS;
}
