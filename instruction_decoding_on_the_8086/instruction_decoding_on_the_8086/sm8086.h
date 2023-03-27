#pragma once

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

struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
static void declare_match(u8 idx);
u8 get_instruction_type(u8 first_byte);

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile);