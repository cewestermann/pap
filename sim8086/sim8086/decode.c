#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utils.h"

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
	"bp", // Direct address
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
	add_reg2either,
	arithmetic_imm2reg,
	add_imm2acc,
	sub_reg2either,
	sub_imm2reg,
	sub_imm_from_acc,
	cmp_reg_reg,
	cmp_imm_acc,
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
	"add_reg2either",
	"arithmetic_imm2reg",
	"add_imm2acc",
	"sub_reg2either",
	"sub_imm2reg",
	"sub_imm_from_acc",
	"cmp_reg_reg",
	"cmp_imm_acc",
	"type_count"
};

typedef struct InstructionByte {
	u8 byte;
	size_t length; // Bit sequence length
} InstructionByte;

InstructionByte instructions[type_count] = {
	// MOV
	// {bit_sequence, number_of_bits}
	[reg2reg] = {0b100010, 6},
	[imm2mem] = {0b1100011, 7},
	[imm2reg] = {0b1011, 4},
	[mem2acc] = {0b1010000, 7},
	[acc2mem] = {0b1010001, 7},
	[reg2seg] = {0b10001110, 8},
	[seg2reg] = {0b10001100, 8},

	// For arithmetics, we have to look at the octal value in the second byte
	[arithmetic_imm2reg] = {0b100000, 6},

	// ADD
	[add_reg2either] = {0b000000, 6},
	[add_imm2acc] = {0b0000010, 7},

	// SUB
	[sub_reg2either] = {0b001010, 6},
	[sub_imm_from_acc] = {0b0010110, 7},

	// CMP
	[cmp_reg_reg] = {0b001110, 6},
	[cmp_imm_acc] = {0b0011110, 7},
};

typedef struct DecodeContext {
    FILE* outfile;
    u8 first_byte;
    u8** buffer;
    size_t* file_pos;
} DecodeContext;

static size_t get_instruction_type(u8 first_byte);
static void declare_match(size_t idx);
static void decode_reg2reg(DecodeContext* dc);
static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);

static void decode_reg2reg(DecodeContext* dc) {
    Opcodes* oc = dc->opcodes;

    u8 d = ((oc.first_byte >> 1) & 1);
    u8 w = oc.first_byte & 1;
    u8 mod = get_mod_encoding(oc.second_byte);
    u8 reg = ((dc->opcodes->second_byte >> 3) & 0b111);
    u8 r_m = get_r_m_encoding(dc->opcodes->second_byte);
}

static u8 get_mod_encoding(u8 second_byte) {
	return ((second_byte >> 6) & 0b11);
}

static u8 get_r_m_encoding(u8 second_byte) {
	return second_byte & 0b111;
}

static size_t get_instruction_type(u8 first_byte) {
    /* Match the first byte against the instruction type table
     * by comparing the first n unique bits of the possible
     * instructions */
	size_t i;

    // If first byte is zero, the buffer is at an
    // incorrect location and has not been updated correctly
    assert(first_byte);

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



