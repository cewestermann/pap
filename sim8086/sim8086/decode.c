#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "pap_types.h"
#include "decode.h"

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

const char* instruction_type_strings[] = {
	"mov_reg2reg",
	"mov_imm2mem",
	"mov_imm2reg",
	"mov_mem2acc",
	"mov_acc2mem",
	"mov_reg2seg",
	"mov_seg2reg",
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
	[mov_reg2reg] = {0b100010, 6},
	[mov_imm2mem] = {0b1100011, 7},
	[mov_imm2reg] = {0b1011, 4},
	[mov_mem2acc] = {0b1010000, 7},
	[mov_acc2mem] = {0b1010001, 7},
	[mov_reg2seg] = {0b10001110, 8},
	[mov_seg2reg] = {0b10001100, 8},

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

typedef struct Instruction {
    bool d;
    bool s;
    bool w;
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


static void declare_match(size_t idx);
static int decode_reg2reg(u8 first_byte, u8** filebuffer, FILE* outfile);
static int decode_imm2reg(u8 first_byte, u8** filebuffer, FILE* outfile);
static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);
static void write_mod11(FILE* outfile, Instruction* inst);
static void write_eac(FILE* outfile, Instruction* inst);
static i32 displacement_16bit(u8** filebuffer);

decode_func* decoders[] = {
    [mov_reg2reg] = decode_reg2reg,
    [mov_imm2reg] = decode_imm2reg
};

static int decode_reg2reg(u8 first_byte, u8** filebuffer, FILE* outfile) {
    /* Decode mov_reg2reg. Return the number of bytes
     * that have been pulled from the buffer during decoding */
    u8 bytes_grabbed = 0;

    u8 second_byte = **filebuffer;
    (*filebuffer)++;
    bytes_grabbed++;

    Instruction inst = {
        .d = (first_byte >> 1) & 1,
        .w = first_byte & 1,
        .mod = get_mod_encoding(second_byte),
        .reg = (second_byte >> 3) & 0b111,
        .r_m = get_r_m_encoding(second_byte),
    };

    switch (inst.mod) {
    case 0b11: write_mod11(outfile, &inst); break;
    case 0b00:
    {
        if (inst.r_m == 0b110) {
            inst.data = displacement_16bit(filebuffer);
            bytes_grabbed += 2;
        }
        write_eac(outfile, &inst);
        break;
    }
    case 0b01:
    {
        inst.disp_lo = **filebuffer;
        (*filebuffer)++;
        inst.data = inst.disp_lo;
        bytes_grabbed++;

        write_eac(outfile, &inst);
        break;
    }
    case 0b10:
    {
        inst.data = displacement_16bit(filebuffer);
        bytes_grabbed += 2;

        write_eac(outfile, &inst);
        break;
    }
    default:
        printf("ERROR: No such MOD number: %d", inst.mod);
        exit(EXIT_FAILURE);
    }
    return bytes_grabbed;
}

static int decode_imm2reg(u8 first_byte, u8** filebuffer, FILE* outfile) {
    u8 bytes_grabbed = 0;

    Instruction inst = {
        .w = (first_byte >> 3) & 1,
        .reg = first_byte & 0b111
    };

    u8 second_byte = **filebuffer;
    (*filebuffer)++;
    bytes_grabbed++;

    if (inst.w) {
        u8 hi = **filebuffer;
        (*filebuffer)++;
        bytes_grabbed++;
        inst.data = (hi << 8 | second_byte);
    } else {
        inst.data = second_byte; 
    }

    fprintf(outfile, "mov, %s, %d\n", registers[inst.w][inst.reg], inst.data);
    return bytes_grabbed;
}


static i32 displacement_16bit(u8** filebuffer) {
	u8 disp_lo = **filebuffer;
	(*filebuffer)++;
	u8 disp_hi = **filebuffer;
	(*filebuffer)++;
	i32 result = (disp_hi << 8 | disp_lo);
	return result;
}

static void  write_mod11(FILE* outfile, Instruction* inst) {
    char const*const reg_field = registers[inst->w][inst->reg];
    char const*const r_m_field = registers[inst->w][inst->r_m];
    if (inst->d)
        fprintf(outfile, "mov %s, %s\n", reg_field, r_m_field);
    else
        fprintf(outfile, "mov %s, %s\n", r_m_field, reg_field);
}

static void write_eac(FILE* outfile, Instruction* inst) {
    char const*const reg_field = registers[inst->w][inst->reg];
    char const*const eac_field = eac[inst->r_m];

	if (inst->d) {
		if (inst->data) { // Displacement
			fprintf(outfile, "mov %s, [%s + %d]\n", reg_field, eac_field, inst->data);
		}
		else {
			fprintf(outfile, "mov %s, [%s]\n", reg_field, eac_field);
		}
	}
	else {
		if (inst->data) {
			fprintf(outfile, "mov [%s + %d], %s\n", eac_field, inst->data, reg_field);
		}
		else {
			fprintf(outfile, "mov [%s], %s\n", eac_field, reg_field);
		}
	}
}

static u8 get_mod_encoding(u8 second_byte) {
	return ((second_byte >> 6) & 0b11);
}

static u8 get_r_m_encoding(u8 second_byte) {
	return second_byte & 0b111;
}

size_t get_instruction_type(u8 first_byte) {
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



