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

	"arithmetic_imm2reg",

	"add_reg2either",
	"add_imm2acc",

	"sub_reg2either",
	"sub_imm2acc",

	"cmp_reg2either",
	"cmp_imm2acc",
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
    [arithmetic_imm_reg] = {0b100000, 6},

	// ADD
	[add_reg_either] = {0b000000, 6},
	[add_imm_acc] = {0b0000010, 7},

	// SUB
	[sub_reg_either] = {0b001010, 6},
	[sub_imm_acc] = {0b0010110, 7},

	// CMP
	[cmp_reg_either] = {0b001110, 6}, // Yes the name is not accurate, but just for consistency
	[cmp_imm_acc] = {0b0011110, 7},
};

typedef struct Instruction {
    const char* inst_type;
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
static int mov_decode_reg2reg(u8 first_byte, u8** filebuffer, FILE* outfile);
static int mov_decode_imm2reg(u8 first_byte, u8** filebuffer, FILE* outfile);

const char* get_arithmetic_type(u8 first_byte); 
static int arithmetic_decode_reg_either(char* arithmetic_type, u8 first_byte, u8** filebuffer, FILE* outfile);
static int arithmetic_decode_imm_reg(u8 first_byte, u8** filebuffer, FILE* outfile);
static int arithmetic_decode_imm_acc(const char* arithmetic_type, u8 first_byte, u8** filebuffer, FILE* outfile);

static int add_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile);
static int sub_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile);
static int cmp_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile);

static int add_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile); 
static int sub_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile); 
static int cmp_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile); 

static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);

static void write_data_to_file(FILE* outfile, Instruction* inst, i32 displacement);
static void write_mod11(FILE* outfile, Instruction* inst);
static void write_eac(FILE* outfile, Instruction* inst);

static i32 displacement_16bit(u8** filebuffer);

decode_func* decoders[] = {
    [mov_reg2reg] = mov_decode_reg2reg,
    [mov_imm2reg] = mov_decode_imm2reg,

    [add_reg_either] = add_decode_reg_either,
    [sub_reg_either] = sub_decode_reg_either,
    [cmp_reg_either] = cmp_decode_reg_either,

    [arithmetic_imm_reg] = arithmetic_decode_imm_reg,

    [add_imm_acc] = add_decode_imm_acc,
    [sub_imm_acc] = sub_decode_imm_acc,
    [cmp_imm_acc] = cmp_decode_imm_acc,
};

static int add_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_imm_acc("add", first_byte, filebuffer, outfile);
}

static int sub_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_imm_acc("sub", first_byte, filebuffer, outfile);
}

static int cmp_decode_imm_acc(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_imm_acc("cmp", first_byte, filebuffer, outfile);
}

static int arithmetic_decode_imm_acc(const char* arithmetic_type, u8 first_byte, u8** filebuffer, FILE* outfile) {

    int bytes_grabbed = 0;

    i32 data = **filebuffer;
    (*filebuffer)++;
    bytes_grabbed++;

    bool w = first_byte & 1;
    if (w) {
        data = (data << 8 | **filebuffer);
        (*filebuffer)++;
        bytes_grabbed++;
        fprintf(outfile, "%s ax, %d\n", arithmetic_type, data);
        return bytes_grabbed;
    }
    
    fprintf(outfile, "%s al, %d\n", arithmetic_type, data);
    return bytes_grabbed;
}

const char* get_arithmetic_type(u8 second_byte) {
    static const char* arith_types[] = {
            [0b000] = "add",
            [0b101] = "sub",
            [0b111] = "cmp"
    };

    return arith_types[(second_byte >> 3) & 0b111];
}

static int arithmetic_decode_imm_reg(u8 first_byte, u8** filebuffer, FILE* outfile) {
    int bytes_grabbed = 0;

    u8 second_byte = **filebuffer;
    (*filebuffer)++;
    bytes_grabbed++;

    const char* arithmetic_type = get_arithmetic_type(second_byte);

    Instruction inst = {
        .inst_type = arithmetic_type,
        .s = (first_byte >> 1) & 1,
        .w = first_byte & 1,
        .mod = get_mod_encoding(second_byte),
        .r_m = get_r_m_encoding(second_byte),
    };

    printf("%d\n", inst.mod);
    switch (inst.mod) {
    case 0b11:
    {
        u8 data = **filebuffer;
        (*filebuffer)++;
        bytes_grabbed++;

        if (inst.w && !inst.s) {
            u8 ext = **filebuffer;
            (*filebuffer)++;
            bytes_grabbed++;
            data = (ext << 8 | data);
        }
        else {
            data = (data << 8 | data); // Sign extend to 16 bits
        }
        fprintf(outfile, "%s %s, %d\n", inst.inst_type, registers[inst.w][inst.r_m], data);
        break;
    }
    case 0b00:
    {
        inst.data = **filebuffer;
        (*filebuffer)++;
        bytes_grabbed++;

        write_data_to_file(outfile, &inst, 0);
        break;
    }
    case 0b01:
    {
        inst.disp_lo = **filebuffer;
        (*filebuffer)++;
        inst.data = **filebuffer;
        (*filebuffer)++;
        bytes_grabbed += 2;
        write_data_to_file(outfile, &inst, (i32)inst.disp_lo);
        break;
    }
    case 0b10:
    {
        i32 displacement = displacement_16bit(filebuffer);
        bytes_grabbed += 2;

        inst.data = **filebuffer;
        (*filebuffer)++;
        bytes_grabbed++;

        write_data_to_file(outfile, &inst, displacement);
        break;
    }
    default:
        printf("ERROR: No such MOD for arithmetic_imm_reg: %d\n", inst.mod);
        exit(EXIT_FAILURE);
    }
    return bytes_grabbed;
}


static int add_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_reg_either("add", first_byte, filebuffer, outfile);
}

static int sub_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_reg_either("sub", first_byte, filebuffer, outfile);
}

static int cmp_decode_reg_either(u8 first_byte, u8** filebuffer, FILE* outfile) {
    return arithmetic_decode_reg_either("cmp", first_byte, filebuffer, outfile);
}

static int arithmetic_decode_reg_either(char* arithmetic_type, u8 first_byte, u8** filebuffer, FILE* outfile) {
    u8 bytes_grabbed = 0;

    u8 second_byte = **filebuffer;
    (*filebuffer)++;
    bytes_grabbed++;

    Instruction inst = {
        .inst_type = arithmetic_type,
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
        printf("ERROR: No such MOD number: %d\n", inst.mod);
        exit(EXIT_FAILURE);
    }
    return bytes_grabbed;
}

static int mov_decode_reg2reg(u8 first_byte, u8** filebuffer, FILE* outfile) {
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

static int mov_decode_imm2reg(u8 first_byte, u8** filebuffer, FILE* outfile) {
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

    fprintf(outfile, "mov %s, %d\n", registers[inst.w][inst.reg], inst.data);
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

static void write_data_to_file(FILE* outfile, Instruction* inst, i32 displacement) {

    char* word_or_byte = (inst->w) ? "word" : "byte";

	if (displacement)
		fprintf(outfile, "%s %s [%s + %d], %d\n", inst->inst_type, word_or_byte, eac[inst->r_m], displacement, inst->data);
	else
		fprintf(outfile, "%s %s [%s], %d\n", inst->inst_type, word_or_byte, eac[inst->r_m], inst->data);
}

static void  write_mod11(FILE* outfile, Instruction* inst) {
    const char* inst_type = (inst->inst_type) ? inst->inst_type : "mov";

    char const*const reg_field = registers[inst->w][inst->reg];
    char const*const r_m_field = registers[inst->w][inst->r_m];
    if (inst->d)
        fprintf(outfile, "%s %s, %s\n", inst_type, reg_field, r_m_field);
    else
        fprintf(outfile, "%s %s, %s\n", inst_type, r_m_field, reg_field);
}

static void write_eac(FILE* outfile, Instruction* inst) {
    char const*const reg_field = registers[inst->w][inst->reg];
    char const*const eac_field = eac[inst->r_m];

    const char* inst_type = (inst->inst_type) ? inst->inst_type : "mov";

	if (inst->d) {
		if (inst->data) { // Displacement
			fprintf(outfile, "%s %s, [%s + %d]\n", inst_type, reg_field, eac_field, inst->data);
		}
		else {
			fprintf(outfile, "%s %s, [%s]\n", inst_type, reg_field, eac_field);
		}
	}
	else {
		if (inst->data) {
			fprintf(outfile, "%s [%s + %d], %s\n", inst_type, eac_field, inst->data, reg_field);
		}
		else {
			fprintf(outfile, "%s [%s], %s\n", inst_type, eac_field, reg_field);
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
    //assert(first_byte); // Assert stops filewrite

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



