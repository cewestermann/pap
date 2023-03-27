#pragma once

#ifndef DECODE_H
#define DECODE_H 1

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

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile);

#endif