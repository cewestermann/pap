#include <stdio.h>

#include "common.h"
#include "decode.h"

static u8 get_mod_encoding(u8 second_byte);
static u8 get_r_m_encoding(u8 second_byte);
static void write_encoding_to_file(FILE* outfile);


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

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile) {
	u8 d = ((first_byte >> 1) & 1);
	u8 w = first_byte & 1;

	u8 second_byte = *buffer++;
	u8 mod = get_mod_encoding(second_byte);
	u8 reg = ((second_byte >> 3) & 0b111);
	u8 r_m = get_r_m_encoding(second_byte);

	i8 disp_lo = ((mod == 0b01) || (mod == 0b10)) ? *buffer++ : 0;
	i16 disp_hi = (mod == 0b10) ? *buffer++ : 0;

	char* output_text;

	switch (mod) {
	case 0b11: output_text = registers[0][r_m]; break;
	case 0b00: output_text = eac[r_m]; break;
	case 0b01: output_text = 0; //TODO

}