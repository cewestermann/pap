#pragma warning(disable : 4996)

#include <stdlib.h>

#include "sm8086.h"

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("many_register_mov.asm", "w");

	struct File file = read_entire_file("listing_0039_more_movs");

	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;
	fprintf(outfile, "bits 16\n\n");

	for (size_t n = 0; n < file.size; n += 2) {
		u8 first_byte = *buffer++;
		u8 second_byte = *buffer++;

		u8 mtype = mov_type(first_byte);

		switch (mtype) {
		case 0b1000:
		{
			// It's a register2register mov
			Instruction inst;
			inst.opcode = mov_reg_to_reg;
			inst.d = (first_byte >> 1) & 1;
			inst.w = first_byte & 1;
			inst.reg = (second_byte >> 3) & 0b111;
			inst.r_m = second_byte & 0b111;

			set_reg_disp_fields(&inst, buffer, second_byte);
            
            // TODO: Clean this up
            if (!inst.disp_lo && !inst.disp_hi) {
                // No displacement bits, i.e., mod = 00
                write_eac_mod_00(&inst);
            } else if (inst.disp_lo && !inst.disp_hi) {
                // 8 bit displacement, i.e., mod = 01
                write_eac_mod_01(&inst);
            } else if (inst.disp_lo && inst.disp_hi) {
                // 16 bit displacement, i.e., mod = 10
                write_eac_mod_10(&inst);
            } else {
                printf("No such mod type");
                exit(EXIT_FAILURE);
            }
			write_instruction_line(outfile, inst);
			break;
		}
		case 0b1011:
		{
			// It's an immediate to register mov
			ImmediateInstruction inst;
			inst.w = (first_byte >> 3) & 1;
			inst.reg = first_byte & 0b111;
			// NOTE: We need to sign extend, i.e., pad with the most significant bit.

			inst.data = second_byte; // TODO: Why is this 244 in one case ?
			if (inst.w) {
				inst.data |= (*buffer++ << 8);
				n++;
			}

			write_imm_instruction_line(outfile, inst);
			break;
		}
		default:
			printf("%d is not a recognized mov type", mtype);
			exit(EXIT_FAILURE);
		}

	}

	free_entire_file(&file);
	fclose(outfile);

	return EXIT_SUCCESS;
}

void write_eac_mod_00(FILE* outfile, Instruction* inst) {
    fprintf(outfile, "%s\n", eac[inst->r_m]);
}

void write_eac_mod_01(FILE* outfile, Instruction* inst) {
    fprintf(outfile, "%s + %d\n", eac[inst->r_m], inst->disp_lo);
}

void write_eac_mod_10(FILE* outfile, Instruction* inst) {
    i16 data = (inst->disp_hi << 8);
    data |= inst->disp_lo;
    fprintf(outfile, "%s + %d\n", eac[inst->r_m], data);
}

u8 mov_type(u8 first_byte) {
	return ((first_byte >> 4) & 0b1111);
}

void set_reg_disp_fields(Instruction* inst, u8* buffer, u8 second_byte) {
	u8 mod = (second_byte >> 6) & 0b11;

	switch (mod) {
	case 0b00:
	{
		if ((second_byte & 0b111) == 0b110) {
			inst->disp_lo = *buffer++;
			inst->disp_hi = *buffer++;
		}
		break;
	}
	case 0b01:
	{
		inst->disp_lo = *buffer++;
		break;
	}
	case 0b10:
	{
		inst->disp_lo = *buffer++;
		inst->disp_hi = *buffer++;
		break;
	}
	case 0b11:
		break;

	default:
		printf("MOD instruction not recognized: %x", mod);
		exit(EXIT_FAILURE);
	}
}

void write_instruction_line(FILE* outfile, Instruction inst) {
	char const*const reg_field = reg_fields[inst.w][inst.reg];
	char const*const r_m_field = reg_fields[inst.w][inst.r_m];


	if (inst.d == 1) { // REG is the destination
		fprintf(outfile, "mov %s, %s\n", reg_field, r_m_field);
	} else // REG is the source
		fprintf(outfile, "mov %s, %s\n", r_m_field, reg_field);
}

void write_imm_instruction_line(FILE* outfile, ImmediateInstruction inst) {
	// If w == 0, only use the first byte.
	// If w == 1, use both bytes. 

	char const*const reg_field = reg_fields[inst.w][inst.reg];



	fprintf(outfile, "mov %s, %i\n", reg_field, inst.data);
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
