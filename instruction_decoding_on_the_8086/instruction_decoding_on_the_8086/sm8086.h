#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8; //_t is platform independent

// size_t is a base unsigned integer, thus good for sizes since they never
// can be negative.

enum OpCode {
	mov_reg_to_reg,
	immediate_to_reg
};

char* reg_fields[2][8] = {
	{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"},
	{"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}
};

char* eac[8] = {
	"bx + si",
	"bx + di",
	"bp + si",
	"bp + di",
	"si",
	"di",
	"direct address",
	"bx"
};

typedef struct Instruction {
	u8 opcode;
	u8 d;
	u8 w;
	u8 mod;
	u8 reg;
	u8 r_m;
	u8 disp_lo;
	u8 disp_hi;
} Instruction;

struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
};

struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
void write_instruction_line(FILE* outfile, Instruction inst);
void set_disp_fields(Instruction* inst, u8* buffer, u8 second_byte);
bool is_reg2reg(u8 first_byte);