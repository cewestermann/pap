#pragma warning(disable : 4996)

#include "sm8086.h"

typedef struct InstructionByte {
	u8 byte;
	size_t length; // Bit sequence length
} InstructionByte;

struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
};

typedef enum {
	reg2reg,
	imm2mem,
	imm2reg,
	mem2acc,
	acc2mem,
	reg2seg,
	seg2reg,
	type_count
} instruction_type;

const char* instruction_type_strings[] = {
	"reg2reg",
	"imm2mem",
	"imm2reg",
	"mem2acc",
	"acc2mem",
	"reg2seg",
	"seg2reg",
	"type_count"
};

InstructionByte instructions[type_count] = {
	// {bit_sequence, number_of_bits}
	[reg2reg] = {0b100010, 6},
	[imm2mem] = {0b1100011, 7},
	[imm2reg] = {0b1011, 4},
	[mem2acc] = {0b1010000, 7},
	[acc2mem] = {0b1010001, 7},
	[reg2seg] = {0b10001110, 8},
	[seg2reg] = {0b10001100, 8}
};

static struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
static void declare_match(u8 idx);
static u8 get_instruction_type(u8 first_byte);

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("bleb.asm", "w");

	struct File file = read_entire_file("encodings\\listing_0042_completionist_decode");
	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;
	u8 first_byte = *buffer++;

	u8 itype = get_instruction_type(first_byte);

	switch (itype) {
	case reg2reg: decode_reg2reg(first_byte, buffer, outfile); break;
	case imm2reg: decode_imm2reg(first_byte, buffer, outfile); break;
	}
	return(EXIT_SUCCESS);
}

static u8 get_instruction_type(u8 first_byte) {
	u8 match;
	for (u8 i = 0; i < type_count; i++) {
		size_t diff = 8 - instructions[i].length;

		if ((first_byte >> diff) == instructions[i].byte) {
			declare_match(i);
			match = i;
		}
	}
	return match;
}

static void declare_match(u8 idx) {
	printf("Current instruction: %s\n", instruction_type_strings[idx]);
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
