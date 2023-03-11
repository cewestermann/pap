#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "sm8086.h"

int main(int argc, char* argv[]) {
	u8* buffer;
	u8 n = 0;
	u8 first_byte;
	u8 second_byte;
	

	FILE* outfile = fopen("many_register_mov.asm", "w");

	struct File file = read_entire_file("listing_0038_many_register_mov");

	// Implicitly cast void pointer to char pointer.
	buffer = file.contents;
	fprintf(outfile, "bits 16\n\n");

	first_byte = *buffer++;
	second_byte = *buffer++;

	if ((first_byte >> 4) & 0b1111 == 0b1000) {
		// It's a register2register mov
		Instruction inst;
		inst.opcode = mov_reg_to_reg;
		inst.d = (first_byte >> 1) & 1;
		inst.w = first_byte & 1;
		inst.reg = (second_byte >> 3) & 0b111;
		inst.r_m = second_byte & 0b111;

		set_disp_fields(&inst, buffer, second_byte);
	}



	free_entire_file(&file);
	fclose(outfile);

	return 0;
}

void set_disp_fields(Instruction* inst, u8* buffer, u8 second_byte) {
	u8 mod = (second_byte >> 6) & 0b11;

	switch (mod) {
	case 0b00:
	{
		if (second_byte & 0b111 == 0b110) {
			inst->disp_lo = *buffer++;
			inst->disp_hi = *buffer++;
		}
		else {
			inst->disp_lo = NULL;
			inst->disp_hi = NULL;
		}
	}
	case 0b01:
	{
		inst->disp_lo = *buffer++;
		inst->disp_hi = NULL;
	}
	case 0b10:
	{
		inst->disp_lo = *buffer++;
		inst->disp_hi = *buffer++;
	}
	case 0b11:
	{
		inst->disp_lo = NULL;
		inst->disp_hi = NULL;

	}
	default:
		printf("MOD instruction not recognized: %x", mod);
		exit(1);
	}
}

void write_instruction_line(FILE* outfile, Instruction inst) {
	char* reg_field = reg_fields[inst.reg];
	char* r_m_field = reg_fields[inst.r_m];

	if (inst.d == 1) { // REG is the destination
		fprintf(outfile, "mov %s, %s\n", reg_field, r_m_field);
	}
	else // REG is the source
		fprintf(outfile, "mov %s, %s\n", r_m_field, reg_field);
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
