#include <stdio.h>
#include <string.h>

#include "sm8086.h"

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


static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);
static void write_mov_encoding(u8 w, u8 r_m, u8 reg, u8 d, FILE* outfile);
static void write_eac_mod_00(u8 r_m, FILE* outfile);
static void write_eac_mod_01(u8 r_m, i8 disp_lo, FILE* outfile);
static void write_eac_mod_10(u8 r_m, i16 disp_hi, FILE* outfile);

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

// TODO: Define a function that takes in the action (mov, push, etc..)
// and somehow writes that plus the source, destination, eac etc. to a file.
static void write_mov_encoding(u8 w, u8 r_m, u8 reg, u8 d, FILE* outfile) {
	char const* const reg_field = registers[w][reg];
	char const* const r_m_field = registers[w][r_m];

	if (d) // Reg is the destination
		fprintf(outfile, "mov %s, %s\n", reg_field, r_m_field);
	else
		fprintf(outfile, "mov %s, %s\n", r_m_field, reg_field);
}

static void write_eac_mod_00(u8 r_m, FILE* outfile) {
	fprintf(outfile, "%s\n", eac[r_m]);
}

static void write_eac_mod_01(u8 r_m, i8 disp_lo, FILE* outfile) {
	fprintf(outfile, "%s + %d\n", eac[r_m], disp_lo);
}

static void write_eac_mod_10(u8 r_m, i16 disp_hi, FILE* outfile) {
	fprintf(outfile, "%s + %d\n", eac[r_m], disp_hi);
}

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile) {
	u8 d = ((first_byte >> 1) & 1);
	u8 w = first_byte & 1;

	u8 second_byte = *buffer++;
	u8 mod = get_mod_encoding(second_byte);
	u8 reg = ((second_byte >> 3) & 0b111);
	u8 r_m = get_r_m_encoding(second_byte);

	i8 disp_lo = ((mod == 0b01) || (mod == 0b10)) ? *buffer++ : 0;
	i16 disp_hi = (mod == 0b10) ? (*buffer++ << 8 | disp_lo) : 0;

	const char* output_text;

	switch (mod) {
	case 0b11: write_mov_encoding(w, r_m, reg, d, outfile); break;
	case 0b00: write_eac_mod_00(r_m, outfile); break;
	case 0b01: write_eac_mod_01(r_m, disp_lo, outfile); break;
	case 0b10: write_eac_mod_10(r_m, disp_hi, outfile); break;
	default: 
		printf("ERROR: Cannot recognize mod %d", mod);
		exit(EXIT_FAILURE);
	}
}

