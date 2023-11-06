#include "tokenizer.h"

#include <stdlib.h>

#define MAX_PAIRS 1024

typedef struct {
  char* key;
  f64 value;
} JSONEntry;

typedef struct {
  JSONEntry pair1;
  JSONEntry pair2;
} JSONPair;

JSONPair pairs[MAX_PAIRS];

void parse_list() {
}

void parse_pair() {
}

void parse_entry() {
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

  Token next_token;
  Token lookahead;

  while ((next_token = tokenize(file)).type != END_OF_FILE) {
    lookahead = tokenize(file);
    printf("Peek token: %s\n", token_types[lookahead.type]);
    break;
  }

  fclose(file);
  return EXIT_SUCCESS;
}



