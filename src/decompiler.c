#include "decompiler.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>


INTERNAL char* decompiler_get_string(uint8_t* _bytecode, size_t _ip) {
    char* str = string_allocate("");
    while (_bytecode[_ip] != 0) {
        str = string_append_char(str, _bytecode[_ip]);
        _ip++;
    }
    return str;
}

INTERNAL int decompiler_get_int(uint8_t* _bytecode, size_t _ip) {
    int value = 0;
    for (size_t i = 0; i < 4; i++) {
        value = value | (_bytecode[_ip + i] << (i * 8));
    }
    return value;
}

INTERNAL long decompiler_get_long(uint8_t* _bytecode, size_t _ip) {
    long value = 0;
    for (size_t i = 0; i < 8; i++) {
        value = value | ((long)_bytecode[_ip + i] << (i * 8));
    }
    return value;
}

INTERNAL
double decompiler_get_double(uint8_t *bytecode, size_t ip) {
    union DoubleBytes {
        double d;
        uint8_t bytes[8];
    };
    union DoubleBytes bytes;
    for (size_t i = 0; i < 8; i++) {
        bytes.bytes[i] = bytecode[ip + i];
    }
    return bytes.d;
}

INTERNAL
void* decompile_get_memory(uint8_t* _bytecode, size_t _ip) {
    uintptr_t value = 0;
    for (size_t i = 0; i < 8; i++) {
        value |= ((uintptr_t)_bytecode[_ip + i] << (i * 8));
    }
    return (void*)value;
}

#define FORWARD(n) ip += n
#define PRINT_OPCODE(format, ...) printf("[%.*zu] " format, (int)log10(bytecode_size) + 1, ip-1, ##__VA_ARGS__)

void decompile(code_t* _code, bool _with_header) {  
    size_t ip = 0;
    // String for the module name
    char* module_name = _code->file_name;
    printf("[MOD NAME]: %s\n", module_name);

    // String for the file name
    char* file_name = _code->block_name;
    printf("[EXE NAME]: %s\n", file_name);
 
    
    size_t bytecode_size = _code->size;
    uint8_t* _bytecode = _code->bytecode;

    size_t index = ip;
    while (ip < bytecode_size) {
        index = ip;
        uint8_t opcode = _bytecode[ip++];
        switch (opcode) {
            case OPCODE_LOAD_NAME: {
                char* name = decompiler_get_string(_bytecode, ip);
                PRINT_OPCODE("load_name: %s\n", name);
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_LOAD_INT: {
                int value = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("load_int: %d\n", value);
                FORWARD(4);
                break;
            }
            case OPCODE_LOAD_DOUBLE: {
                double value = decompiler_get_double(_bytecode, ip);
                PRINT_OPCODE("load_double: %f\n", value);
                FORWARD(8);
                break;
            }
            case OPCODE_LOAD_STRING: {
                char* value = decompiler_get_string(_bytecode, ip);
                PRINT_OPCODE("load_string: %s\n", value);
                FORWARD(strlen(value) + 1);
                free(value);
                break;
            }
            case OPCODE_LOAD_NULL: {
                PRINT_OPCODE("load_null: null\n");
                break;
            }
            case OPCODE_LOAD_BOOL: {
                bool value = _bytecode[ip] == 1;
                PRINT_OPCODE("load_bool: %s\n", value ? "true" : "false");
                FORWARD(1);
                break;
            }
            case OPCODE_LOAD_ARRAY: {
                int length = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("load_array: (length = %d)\n", length);
                FORWARD(4);
                break;
            }
            case OPCODE_EXTEND_ARRAY: {
                PRINT_OPCODE("extend_array\n");
                break;
            }
            case OPCODE_APPEND_ARRAY: {
                PRINT_OPCODE("append_array\n");
                break;
            }
            case OPCODE_LOAD_OBJECT: {
                int length = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("load_object: (length = %d)\n", length);
                FORWARD(4);
                break;
            }
            case OPCODE_EXTEND_OBJECT: {
                PRINT_OPCODE("extend_object\n");
                break;
            }
            case OPCODE_PUT_OBJECT: {
                PRINT_OPCODE("put_object\n");
                break;
            }
            case OPCODE_STORE_NAME: {
                char* name = decompiler_get_string(_bytecode, ip);
                PRINT_OPCODE("store_name: %s\n", name);
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_SET_NAME: {
                char* name = decompiler_get_string(_bytecode, ip);
                PRINT_OPCODE("set_name: %s\n", name);
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_RANGE: {
                PRINT_OPCODE("range\n");
                break;
            }
            case OPCODE_INDEX: {
                PRINT_OPCODE("index\n");
                break;
            }
            case OPCODE_CALL: {
                int argc = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("call: (argc = %d)\n", argc);
                FORWARD(4);
                break;
            }
            case OPCODE_INCREMENT: {
                PRINT_OPCODE("increment\n");
                break;
            }
            case OPCODE_MUL: {
                PRINT_OPCODE("mul (*)\n");
                break;
            }
            case OPCODE_DIV: {
                PRINT_OPCODE("div (/)\n");
                break;
            }
            case OPCODE_MOD: {
                PRINT_OPCODE("mod (%%)\n");
                break;
            }
            case OPCODE_ADD: {
                PRINT_OPCODE("add (+)\n");
                break;
            }
            case OPCODE_SUB: {
                PRINT_OPCODE("sub (-)\n");
                break;
            }
            case OPCODE_SHL: {
                PRINT_OPCODE("shl (<<)\n");
                break;
            }
            case OPCODE_SHR: {
                PRINT_OPCODE("shr (>>)\n");
                break;
            }
            case OPCODE_CMP_LT: {
                PRINT_OPCODE("cmp_lt (<)\n");
                break;
            }
            case OPCODE_CMP_LTE: {
                PRINT_OPCODE("cmp_lte (<=)\n");
                break;
            }
            case OPCODE_CMP_GT: {
                PRINT_OPCODE("cmp_gt (>)\n");
                break;
            }
            case OPCODE_CMP_GTE: {
                PRINT_OPCODE("cmp_gte (>=)\n");
                break;
            }
            case OPCODE_CMP_EQ: {
                PRINT_OPCODE("cmp_eq (==)\n");
                break;
            }
            case OPCODE_CMP_NE: {
                PRINT_OPCODE("cmp_ne (!=)\n");
                break;
            }
            case OPCODE_AND: {
                PRINT_OPCODE("and (&)\n");
                break;
            }
            case OPCODE_OR: {
                PRINT_OPCODE("or (|)\n");
                break;
            }
            case OPCODE_XOR: {
                PRINT_OPCODE("xor (^)\n");
                break;
            }
            case OPCODE_POP_JUMP_IF_FALSE: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("pop_jump_if_false: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_POP_JUMP_IF_TRUE: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("pop_jump_if_true: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_JUMP_IF_FALSE_OR_POP: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("jump_if_false_or_pop: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_JUMP_IF_TRUE_OR_POP: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("jump_if_true_or_pop: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_JUMP_IF_NOT_ERROR: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("jump_if_not_error: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_JUMP_FORWARD: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("jump_forward: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_ABSOLUTE_JUMP: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("absolute_jump: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_POPTOP: {
                PRINT_OPCODE("pop_top\n");
                break;
            }
            case OPCODE_NOP: {
                PRINT_OPCODE("nop\n");
                break;
            }
            case OPCODE_MAKE_FUNCTION:
            case OPCODE_MAKE_ASYNC_FUNCTION: {
                code_t* function_bytecode = (code_t*) decompile_get_memory(_bytecode, ip);
                PRINT_OPCODE("make_%s: %s\n", opcode == OPCODE_MAKE_ASYNC_FUNCTION ? "async_function" : "function", function_bytecode->block_name);
                printf("+-----------------+\n");
                printf("| MAKE FUNCTION %d |\n", opcode == OPCODE_MAKE_ASYNC_FUNCTION);
                printf("+-----------------+\n");
                printf("| PARAM COUNT: %zu |\n", function_bytecode->param_count);
                printf("+-----------------+\n");
                decompile(function_bytecode, false);
                printf("+-----------------+\n");
                printf("| END FUNCTION    |\n");
                printf("+-----------------+\n");
                FORWARD(8);
                break;
            }
            case OPCODE_SETUP_BLOCK: {
                code_t* address = (code_t*) decompile_get_memory(_bytecode, ip);
                PRINT_OPCODE("setup_block:\n");
                printf("+-----------------+\n");
                printf("| SET BLOCK       |\n");
                printf("+-----------------+\n");
                decompile(address, false);
                printf("+-----------------+\n");
                printf("| END BLOCK       |\n");
                printf("+-----------------+\n");
                FORWARD(8);
                break;
            }
            case OPCODE_SETUP_CATCH_BLOCK: {
                size_t function_size = decompiler_get_long(_bytecode, ip);
                FORWARD(8);
                char* file_path = decompiler_get_string(_bytecode, ip);
                FORWARD(strlen(file_path) + 1);
                char* block_name = decompiler_get_string(_bytecode, ip);
                FORWARD(strlen(block_name) + 1);
                
                uint8_t* block_bytecode = malloc(function_size);
                memcpy(block_bytecode, _bytecode + ip - 8 - strlen(file_path) - 1 - strlen(block_name) - 1, function_size);
                
                PRINT_OPCODE("setup_catch_block: %s\n", block_name);
                printf("+-----------------+\n");
                printf("| SET CATCH BLOCK |\n");
                printf("+-----------------+\n");
                // decompile(block_bytecode, false);
                printf("+-----------------+\n");
                printf("| END CATCH BLOCK |\n");
                printf("+-----------------+\n");
                
                free(file_path);
                free(block_name);
                free(block_bytecode);
                break;
            }
            case OPCODE_RETURN: {
                PRINT_OPCODE("return\n");
                break;
            }
            case OPCODE_COMPLETE_BLOCK: {
                PRINT_OPCODE("complete_block\n");
                break;
            }
            case OPCODE_DUPTOP: {
                PRINT_OPCODE("dup_top\n");
                break;
            }
            case OPCODE_ROT2: {
                PRINT_OPCODE("rot2\n");
                break;
            }
            case OPCODE_SAVE_CAPTURES: {
                int capture_count = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("save_captures: (capture_count = %d) ", capture_count);
                int length = 4;
                printf("[");
                for (int i = 0; i < capture_count; i++) {
                    char* name = decompiler_get_string(_bytecode, ip+length);
                    printf("%s", name);
                    length += strlen(name) + 1;
                    if (i < capture_count - 1) {
                        printf(", ");
                    }
                    free(name);
                }
                printf("]\n");
                FORWARD(length);
                break;
            }
            case OPCODE_GET_ITERATOR_OR_JUMP: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("get_iterator_or_jump: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_HAS_NEXT: {
                int jump_offset = decompiler_get_int(_bytecode, ip);
                PRINT_OPCODE("has_next: (jump_to_offset = %d)\n", jump_offset);
                FORWARD(4);
                break;
            }
            case OPCODE_GET_NEXT_VALUE: {
                PRINT_OPCODE("get_next_value\n");
                break;
            }
            case OPCODE_GET_NEXT_KEY_VALUE: {
                PRINT_OPCODE("get_next_key_value\n");
                break;
            }
            default:
                printf("[%zu] Unknown opcode: %d\n", ip-1, opcode);
                return;
        }
    }
}

#undef FORWARD
#undef PRINT_OPCODE
