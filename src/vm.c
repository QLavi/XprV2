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


void init_vm(void) {
    vm.stack = ALLOC(Value, 256);
    vm.stk_ptr = vm.stack;
    vm.constants = ALLOC(Value, 256);
    vm.co_count = 0;
    vm.ip = NULL;
    init_table(&vm.symbols);
}

#define PUSH(x) (*vm.stk_ptr++ = x)
#define POP() (*--vm.stk_ptr)

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
                    int jump_offset = vm.ip[0] << 8 | vm.ip[1];
                    vm.ip += 2;
                    vm.ip += jump_offset;
                }
                else {
                    vm.ip += 2;
                } break;
            case JUMP_BLOCK:
            {
                int jump_offset = vm.ip[0] << 8 | vm.ip[1];
                vm.ip += 2;
                vm.ip += jump_offset;
            } break;
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
                    case MOD_OP:
                    {
                        Value a = POP();
                        Value b = POP();
                        PUSH((int)(b) % (int)(a));
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
                        Value value = -POP();
                        PUSH(value);
                    } break;
                    case NOT_OP:
                    {
                        Value value = !POP();
                        PUSH(value);
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
            } break;
            case PRINT_VALUE:
            {
                Value value = POP();
                printf("%0.1f\n", value);
                vm.ip++;
            } break;
            case POP_TOP:
                (void)POP();
                break;
            case RETURN_VALUE:
                (void)POP(); // No Functions so this is just blank
                return;
                break;
        }
        offset = vm.ip - stream;
    }
}
