#include <stdio.h>
#include "io_and_mem.h"

#include "value.h"
#include "table.h"
#include "generator.h"
#include "vm.h"

typedef struct {
    uint8_t* ip;

    Value* stack;
    Value* stk_ptr;
    Value* constants;
    int co_count;
    Table symbols;
} VM;

VM vm;

#define PUSH(x) (*vm.stk_ptr++ = x)
#define POP() (*--vm.stk_ptr)

void init_vm(void) {
    vm.stack = ALLOC(Value, 256);
    vm.stk_ptr = vm.stack;
    vm.constants = ALLOC(Value, 256);
    vm.co_count = 0;
    vm.ip = NULL;
    init_table(&vm.symbols);
}

void free_vm(void) {
    FREE(Value, vm.stack);
    FREE(Value, vm.constants);
    vm.co_count = 0;
    vm.ip = NULL;
}

uint8_t load_const(Value value) {
    vm.constants[vm.co_count++] = value;
    return vm.co_count -1;
}
Value get_const(uint8_t idx) {
    return vm.constants[idx];
}

void eval(uint8_t* stream, int count) {
    vm.ip = stream;

    int offset = 0;
    for(;offset <= count;) {
        switch(*vm.ip++) {
            case POP_JUMP_IF_FALSE:
                if(POP() == false) {
                    int jump_offset = *vm.ip++ << 8 | *vm.ip++;
                    vm.ip += jump_offset;
                }
                else {
                    vm.ip += 2;
                }
                break;
            case BINARY_OP:
                switch(*vm.ip++) {
                    case ADD_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(a + b);
                    } break;
                    case SUB_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b - a);
                    } break;
                    case MUL_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b * a);
                    } break;
                    case DIV_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b / a);
                    } break;
                    case LESS_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b < a);
                    } break;
                    case LESS_EQUAL_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b <= a);
                    } break;
                    case GREATER_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b > a);
                    } break;
                    case GREATER_EQUAL_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b >= a);
                    } break;
                    case NOT_EQUAL_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b != a);
                    } break;
                    case EQUAL_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH(b == a);
                    } break;
                } break;
            case UNARY_OP:
                switch(*vm.ip++) {
                    case NEGATE_OP:
                    {
                        PUSH(-POP());
                    } break;
                    case NOT_OP:
                    {
                        PUSH(!POP());
                    } break;
                } break;
            case LOAD_NAME:
                Value value;
                if(!table_get(&vm.symbols, *vm.ip++ - 'a', &value)) {
                    fprintf(stderr, "Accessing undefined variable");
                }
                PUSH(value);
                break;
            case STORE_NAME:
                {
                    Value value = POP();
                    if(!table_set(&vm.symbols, *vm.ip++ - 'a', value)) {
                        fprintf(stderr, "Accessing undefined variable");
                    }
                } break;
            case LOAD_CONST:
                {
                    Value value = vm.constants[*vm.ip++];
                    PUSH(value);
                }
                break;
            case POP_TOP:
                POP();
                break;
            case RETURN_VALUE:
                printf("Eval Result: %g\n", POP());
                return;
                break;
        }
        offset = vm.ip - stream;
    }
}