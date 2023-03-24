#pragma once

#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;


enum encoding_type {
	// TODO: Add encoding types as we progress. 
	mov,
};

struct File {
	size_t size;
	void* contents; // Void pointer (A generic pointer that does not need an explicit type)
};


struct File read_entire_file(char* filename);
static void free_entire_file(struct File* file);
enum encoding_type get_encoding_type(u8 first_byte);
