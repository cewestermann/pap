#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sm8086.h"

int main(int argc, char* argv[]) {
	u8* buffer;
	char c;
	char first_line[100];

	char* to_write;

	FILE* outfile = fopen("single_register_mov.asm", "w");

	struct File file = read_entire_file("listing_0037_single_register_mov");

	// Implicitly cast void pointer to char pointer.
	buffer = file.contents;

	struct Instruction first_inst = decode_single_instruction(buffer);

	fprintf(outfile, "bits 16\n\n");

	write_instruction_line(outfile, first_inst);

	free_entire_file(&file);
	fclose(outfile);

	return 0;
}

void write_instruction_line(FILE* outfile, struct Instruction inst) {
	char* reg_field = fields[inst.reg];
	char* r_m_field = fields[inst.r_m];

	if (inst.d == 1) { // REG is the destination
		fprintf(outfile, "mov %s, %s", reg_field, r_m_field);
	}
	else { // REG is the source
		fprintf(outfile, "mov %s, %s", r_m_field, reg_field);
	}
}

struct Instruction decode_single_instruction(char* buffer) {
	u8 first_byte = *buffer++;
	u8 second_byte = *buffer++;

	struct Instruction inst = {
		.opcode = mov_reg_to_reg,
		.d = (first_byte >> 1) & 1,
		.w = first_byte & 1,
		.mod = (second_byte >> 6) & 0b11,
		.reg = (second_byte >> 3) & 0b111,
		.r_m = second_byte & 0b111
	};

	return inst;
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
		exit(1);
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