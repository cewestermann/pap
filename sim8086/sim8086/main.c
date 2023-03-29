#pragma warning(disable : 4996)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

#define N_ENCODING_FIELDS 8

const char* registers[2][N_ENCODING_FIELDS] = {
	{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
	{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}
};

const char* eac[N_ENCODING_FIELDS] = {
	"bx + si",
	"bx + di",
	"bp + si",
	"bp + di",
	"si",
	"di",
	"direct adress",
	"bx",
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

typedef struct InstructionByte {
	u8 byte;
	size_t length; // Bit sequence length
} InstructionByte;

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

struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
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

typedef struct Opcodes {
	u8 first_byte;
	u8 second_byte;
} Opcodes;

static struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
static void declare_match(size_t idx);
static size_t get_instruction_type(u8 first_byte);
static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);
static void write_mod11_to_file(FILE* outfile, u8 d, char const* const reg_field, char const* const r_m_field);
static void write_eac_to_file(FILE* outfile, u8 d, char const* const reg_field, char const* const r_m_field);
static void write_eac_to_file_disp(FILE* outfile, u8 d, char const* const reg_field, i32 data);
static void write_source_address_calculation(FILE* outfile, char const* const reg_field, char const* const address);
static void decode_reg2reg(FILE* outfile, Opcodes* ops, u8* buffer, size_t* n);
static void decode_imm2reg(FILE* outfile, Opcodes* ops, u8* buffer, size_t* n);

int main(int argc, char* argv[]) {
	FILE* outfile = fopen("bleb.asm", "w");

	struct File file = read_entire_file("sim8086\\encodings\\listing_0039_more_movs");
	fprintf(outfile, "bits 16\n\n");
	// Implicitly cast void pointer to char pointer.
	u8* buffer = file.contents;

	for (size_t n = 0; n < file.size; n += 2) {
		Opcodes ops = {
			.first_byte = *buffer++,
			.second_byte = *buffer++,
		};

		size_t itype = get_instruction_type(ops.first_byte);

		switch (itype) {
		case reg2reg: decode_reg2reg(outfile, &ops, buffer, &n); break;
		case imm2reg: 
		{
			// TODO: Move this into a function, but note that it breaks as first_byte equals 0 after 6 iterations
			Instruction inst = {
			.w = (ops.first_byte >> 3) & 1,
			.reg = ops.first_byte & 0b111
			};

			if (inst.w) {
				u8 third_byte = *buffer++;
				n++;
				inst.data = (third_byte << 8 | ops.second_byte);
			}
			else
				inst.data = ops.second_byte;

			const char* const dst_reg = registers[inst.w][inst.reg];

			fprintf(outfile, "mov %s, %d\n", dst_reg, inst.data);
		} break;
		default: printf("No such itype: %zu\n", itype);          exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

static void decode_imm2reg(FILE* outfile, Opcodes* ops, u8* buffer, size_t* n) {
	Instruction inst = {
	.w = (ops->first_byte >> 3) & 1,
	.reg = ops->first_byte & 0b111
	};

	if (inst.w) {
		u8 third_byte = *buffer++;
		(*n)++;
		inst.data = (third_byte << 8 | ops->second_byte);
	}
	else
		inst.data = ops->second_byte;

	const char* const dst_reg = registers[inst.w][inst.reg];

	fprintf(outfile, "mov %s, %d\n", dst_reg, inst.data);
}

static void decode_reg2reg(FILE* outfile, Opcodes* ops, u8* buffer, size_t* n) {
	Instruction inst = {
		.d = ((ops->first_byte >> 1) & 1),
		.w = ops->first_byte & 1,
		.mod = get_mod_encoding(ops->second_byte),
		.reg = ((ops->second_byte >> 3) & 0b111),
		.r_m = get_r_m_encoding(ops->second_byte),
	};

	char const* const reg_field = registers[inst.w][inst.reg];
	char const* const r_m_field = registers[inst.w][inst.r_m];

	switch (inst.mod) {
	case 0b11: write_mod11_to_file(outfile, inst.d, reg_field, r_m_field); break;
	//case 0b00: write_eac_to_file(outfile, inst.d, reg_field, r_m_field); break;
	case 0b00: write_source_address_calculation(outfile, reg_field, eac[inst.r_m]); break;
	case 0b01:
	{
		inst.disp_lo = *buffer++;
		inst.data = inst.disp_lo;
		(*n)++;
		write_eac_to_file_disp(outfile, inst.d, reg_field, inst.data);
	} break;
	case 0b10:
	{
		inst.disp_lo = *buffer++;
		inst.disp_hi = *buffer++;
		inst.data = (inst.disp_hi << 8 | inst.disp_lo);
		(*n) += 2;
		write_eac_to_file_disp(outfile, inst.d, reg_field, inst.data);
	} break;
	default:
		printf("ERROR: No such MOD number: %d", inst.mod);
		exit(EXIT_FAILURE);
	}
}

static void write_mod11_to_file(FILE* outfile, u8 d, char const* const reg_field, char const* const r_m_field) {
	if (d) // reg_field is the destination
		fprintf(outfile, "mov %s, %s\n", reg_field, r_m_field);
	else // reg_field is the source
		fprintf(outfile, "mov %s, %s\n", r_m_field, reg_field);
}

static void write_eac_to_file(FILE* outfile, u8 d, char const* const reg_field, char const* const r_m_field) {
	if (d)
		fprintf(outfile, "mov %s, [%s]\n", reg_field, r_m_field);
	else
		fprintf(outfile, "mov [%s], %s\n", r_m_field, reg_field);
}

static void write_source_address_calculation(FILE* outfile, char const* const reg_field, char const* const address) {
	fprintf(outfile, "mov %s, [%s]\n", reg_field, address);
}

static void write_eac_to_file_disp(FILE* outfile, u8 d, char const* const reg_field, i32 data) {
	fprintf(outfile, "mov %s, %d\n", reg_field, data);
}


static u8 get_mod_encoding(u8 second_byte) {
	// The mod bits do not seem to change position, so we can
	// use a function for this..
	return ((second_byte >> 6) & 0b11);
}

static u8 get_r_m_encoding(u8 second_byte) {
	// r_m field does not seem to change position, so we can use
	// a function for this.
	return second_byte & 0b111;
}

static size_t get_instruction_type(u8 first_byte) {
	size_t i;

	for (i = 0; i < type_count; i++) {
		size_t diff = 8 - instructions[i].length;

		if ((first_byte >> diff) == instructions[i].byte) {
			declare_match(i);
			return i;
		}
	}

	printf("Exceeded number of types. No matching type found.\n");
	exit(EXIT_FAILURE);
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
