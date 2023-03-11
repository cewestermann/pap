#pragma once

#include <stdio.h>
#include <stdint.h>

#define dbg(x) __builtin_dump_struct(&(x), fprintf, stderr)

typedef uint8_t u8; //_t is platform independent

// size_t is a base unsigned integer, thus good for sizes since they never
// can be negative.

enum OpCode {
	mov_reg_to_reg
};

char* fields[8] = {
	"ax", "cx", "dx", "bx",
	"sp", "bp", "si", "di"
};

struct Instruction {
	u8 opcode;
	u8 d;
	u8 w;
	u8 mod;
	u8 reg;
	u8 r_m;
};

struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
};

struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
struct Instruction decode_single_instruction(u8 first_byte, u8 second_byte);
void write_instruction_line(FILE* outfile, struct Instruction inst);