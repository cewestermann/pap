#pragma warning(disable : 4996)

#include "sm8086.h"

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("completionist_decode.asm", "w");

	struct File file = read_entire_file("listing_0042_completionist_decode");
	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;
	u8 first_byte = *buffer++;

	u8 itype = get_instruction_type(first_byte);

	switch (itype) {
	case reg2reg: decode_reg2reg(first_byte, buffer, outfile); break;
	}


	return(EXIT_SUCCESS);
}

u8 get_instruction_type(u8 first_byte) {
	u8 match;
	for (size_t i = 0; i < type_count; i++) {
		u8 diff = 8 - instructions[i].length;

		if ((first_byte >> diff) == instructions[i].byte) {
			declare_match(i);
			match = i;
		}
	}
	return match;
}

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile) {
	u8 d = ((first_byte >> 1) & 1);
	u8 w = first_byte & 1;

	u8 second_byte = *buffer++;
	u8 mod = ((second_byte >> 6) & 0b11);
	u8 reg = ((second_byte >> 3) & 0b111);
	u8 r_m = second_byte & 0b111;

	switch (mod) {
	case 0: break;
	}


}

static void declare_match(u8 idx) {
	printf("Current instruction: %s\n", instruction_type_strings[idx]);
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
