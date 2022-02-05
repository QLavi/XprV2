#pragma once

#include <stdbool.h>
#include "value.h"

typedef struct {
    Value data[26];
} Table;

void init_table(Table* table);
bool table_set(Table* table, int ix, Value value);
bool table_get(Table* table, int ix, Value* value);
