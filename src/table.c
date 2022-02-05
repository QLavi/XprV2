#include <stdio.h>
#include "table.h"

void init_table(Table* table) {
    for(int x = 0; x < 26; x++) {
        table->data[x] = -1;
    }
}

bool table_set(Table* table, int ix, Value value) {
    if(ix > 26) {
        fprintf(stderr, "out of range index: %i\n", ix);
        return false;
    }
    table->data[ix] = value;
    return true;
}

bool table_get(Table* table, int ix, Value* value) {
    if(ix > 26) {
        fprintf(stderr, "out of range index: %i\n", ix);
        return false;
    }
    Value present = table->data[ix];
    if(present == -1) {
        fprintf(stderr, "no value present on the index provided: %i\n", ix);
        return false;
    }
    *value = present;
    return true;
}
