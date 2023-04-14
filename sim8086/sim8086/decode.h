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

size_t get_instruction_type(u8 first_byte);

typedef int decode_func(u8 first_byte, u8** filebuffer, FILE* outfile);

extern decode_func* decoder;
