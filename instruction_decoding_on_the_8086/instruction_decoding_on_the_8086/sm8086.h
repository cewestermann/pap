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

void decode_reg2reg(u8 first_byte, u8* buffer, FILE* outfile);
void decode_imm2reg(u8 first_byte, u8* buffer, FILE* outfile);