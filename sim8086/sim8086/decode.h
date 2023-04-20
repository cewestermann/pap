#pragma once

#include <stdio.h>
#include "pap_types.h"

typedef enum {
	mov_reg2reg,
	mov_imm2mem,
	mov_imm2reg,
	mov_mem2acc,
	mov_acc2mem,
	mov_reg2seg,
	mov_seg2reg,

	arithmetic_imm_reg,

	add_reg_either,
	add_imm_acc,

	sub_reg_either,
	sub_imm_acc,

	cmp_reg_either,
	cmp_imm_acc,

	type_count
} instruction_type;

size_t get_instruction_type(u8 first_byte);

typedef int decode_func(u8 first_byte, u8** filebuffer, FILE* outfile);
extern decode_func* decoders[];
