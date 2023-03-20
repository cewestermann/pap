#pragma warning(disable : 4996)

#include <stdlib.h>

#include "sm8086.h"

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("completionist_decode.asm", "w");

	struct File file = read_entire_file("listing_0042_completionist_decode");

	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;

	enum encoding_type etype = get_encoding_type(*buffer++);

	return(EXIT_SUCCESS);
}

enum encoding_type get_encoding_type(u8 first_byte) {
	switch (first_byte) {
		case 
	}
}










struct File read_entire_file(char* filename) {
	struct File result;

	FILE* file = fopen(filename, "rb");

	if (file) {
		// Go to end
		fseek(file, 0, SEEK_END); 
		// Get file size (returns number of bytes to position of file position)
		result.size = ftell(file);
		// Go back to beginning
		fseek(file, 0, SEEK_SET); 

		// Allocate memory to the contents of the file
		result.contents = malloc(result.size);

		if (result.contents && result.size)
			fread(result.contents, result.size, 1, file);
		fclose(file);
	}
	else {
		printf("ERROR: Unable to load \"%s\"\n", filename);
		exit(EXIT_FAILURE);
	}
	return result;
}

static void free_entire_file(struct File* file) {
	if (file->contents) {
		free(file->contents);
		file->contents = 0;
	}

	file->size = 0;
}
