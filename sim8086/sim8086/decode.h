#pragma once

#include <stdio.h>
#include "pap_types.h"

typedef int decode_func(u8 first_byte, u8** filebuffer, FILE* outfile);

extern decode_func* decoder;

