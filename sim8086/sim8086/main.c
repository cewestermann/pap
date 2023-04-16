#include <stdlib.h>
#include <stdio.h>

#include "decode.h"

/* Structs */
struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
};

/* Function declarations */
static struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);

/* Entry point */
int main(int argc, char* argv[argc + 1]) {
	FILE* outfile = fopen("out.asm", "w");
	struct File file = read_entire_file("./encodings/listing_0041_add_sub_cmp_jnz");

	u8* filebuffer = file.contents;

	for (size_t n = 0; n < file.size; n += 2) {
        u8 first_byte = *filebuffer++;
        n++;

		size_t itype = get_instruction_type(first_byte);

        decoder = decoders[itype];

        n += decoder(first_byte, &filebuffer, outfile);
    }

    return EXIT_SUCCESS;
}

static struct File read_entire_file(char* filename) {
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
