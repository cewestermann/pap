#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

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

typedef struct Instruction {
	u8 d;
	u8 w;
	u8 mod;
	u8 reg;
	u8 r_m;
	u8 disp_lo;
	u8 disp_hi;
	u8 addr_lo;
	u8 addr_hi;
	u8 sr;
	i32 data;
} Instruction;

static struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
static void declare_match(size_t idx);
static u8 get_instruction_type(u8 first_byte);
static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("bleb.asm", "w");

	struct File file = read_entire_file("encodings\\listing_0038_many_register_mov");
	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;

	for (size_t n = 0; n < file.size; n += 2) {
		Instruction inst = { 0 };

		u8 first_byte = *buffer++;
		u8 second_byte = *buffer++;

		u8 itype = get_instruction_type(first_byte);

		switch (itype) {
		case reg2reg:
		{
			inst.d = ((first_byte >> 1) & 1);
			inst.w = first_byte & 1;
			inst.mod = get_mod_encoding(second_byte);
			inst.reg = ((second_byte >> 3) & 0b111);
			inst.r_m = get_r_m_encoding(second_byte);

			if (inst.mod == 0b01) {
				inst.disp_lo = *buffer++;
				inst.data = inst.disp_lo;
				n++;
			}
			if (inst.mod == 0b10) {
				inst.disp_lo = *buffer++;
				inst.disp_hi = *buffer++;
				inst.data = (inst.disp_hi << 8 | inst.disp_lo);
				n += 2;
			}





		}
		}


	}

}


static u8 get_mod_encoding(u8 second_byte) {
	// The mod bits do not seem to change position, so we can
	// use a function for this.
	return ((second_byte >> 6) & 0b11);
}

static u8 get_r_m_encoding(u8 second_byte) {
	// r_m field does not seem to change position, so we can use
	// a function for this.
	return second_byte & 0b111;
}

static u8 get_instruction_type(u8 first_byte) {
	size_t i;

	for (i = 0; i < type_count; i++) {
		size_t diff = 8 - instructions[i].length;
		if ((first_byte >> diff) == instructions[i].byte) {
			declare_match(i);
			break;
		}
	}

	if (i == type_count) {
		printf("Exceeded number of types. No matching type found.\n");
		exit(EXIT_FAILURE);
	}

	return i;
}

static void declare_match(size_t idx) {
	printf("Instruction: %s\n", instruction_type_strings[idx]);
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
