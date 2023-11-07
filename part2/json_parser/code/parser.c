#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef uint8_t u8;
typedef uint64_t u64;

typedef double f64;

#include "buffer.c"

typedef enum {
  TOKEN_EOF,
  TOKEN_ERROR,

  TOKEN_OPEN_BRACE,
  TOKEN_CLOSE_BRACE,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_STRING,
  TOKEN_FACTOR,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_NULL,
  TOKEN_COUNT,

} TokenType;

typedef struct {
  TokenType type;
  Buffer value;
} Token;

typedef struct {
  Buffer label;
  Buffer value;

  struct Element *first_sub_element;
  struct Element *next_sibling;
} Element;

/* The parser that can be passed around */
typedef struct {
  Buffer source;
  u64 pos;
  bool had_error;
} Parser;

static Token next_token(Parser* parser) {
  Token result = {0};

  /* Just for convenience? */
  Buffer source = parser->source;
  u64 pos = parser->pos;
  
  // TODO(cew): Continue here ! 

}


static Buffer read_entire_file(char* filename) {
  Buffer result = {0};

  FILE* file = fopen(filename, "rb");

  if (file) {
#if _WIN32
    struct __stat64 stat;
    _stat64(filename, &stat);
#else
    struct stat stat;
    stat(filename, &stat);
#endif
    
    result = allocate_buffer(stat.st_size);
    if (result.data) {
      if (fread(result.data, result.size, 1, file) != 1) {
        fprintf(stderr, "ERROR: Unable to read \"%s\".\n", filename);
        free_buffer(&result);
      }
    }
    fclose(file);
  } else {
    fprintf(stderr, "ERROR: Unable to open \"%s\".\n", filename);
  }

  return result;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: parser.exe <file.json>\n");
    return EXIT_FAILURE;
  }

  Buffer json_input = read_entire_file(argv[1]);

  return EXIT_SUCCESS;
}
