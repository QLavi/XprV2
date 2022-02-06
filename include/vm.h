#pragma once

#include "value.h"

void init_vm(void);
void free_vm(void);
uint8_t load_const(Value value);
Value get_const(uint8_t idx);
void eval(uint8_t* stream, int count);
