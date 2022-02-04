#pragma once

#include <stdio.h>
#include <stdlib.h>

#define ALLOC(type, count) (type*)x_alloc(NULL, sizeof(type) * (count), __LINE__, __FILE__)
#define REALLOC(type, old_ptr, count)  (type*)x_alloc(old_ptr, sizeof(type) * (count), __LINE__, __FILE__)
#define FREE(type, ptr) x_alloc(ptr, 0, __LINE__, __FILE__)

void* x_alloc(size_t* old_ptr, size_t size, int line_no, char* filename);
char* load_file(char* filename);

