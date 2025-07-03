#include "api/core/object.h"
#include "api/core/vm.h"
#include "error.h"
#include "internal.h"
#include "gc.h"
#include "object.h"
#include "opcode.h"
#include "type.h"
#include "vm.h"

#define GC_ALLOCATION_THRESHOLD 10000

#define PUSH(obj) { \
    ASSERTNULL(instance->evaluation_stack, "Evaluation stack is full"); \
    instance->allocation_counter++; \
    instance->evaluation_stack[instance->sp++] = obj; \
    obj->next = instance->root; \
    instance->root = obj; \
}

#define FORWARD(size) { \
    ip += size; \
}

#define PEEK() (instance->evaluation_stack[instance->sp - 1])
#define POPP() (instance->evaluation_stack[--instance->sp])
#define POPN(_times) { \
    for (size_t i = 0; i < _times; i++) \
        POPP(); \
}

#define LENGTH_OF_MAGIC_NUMBER  4
#define LENGTH_OF_VERSION       4
#define LENGTH_OF_BYTECODE_SIZE 8

// Bytecode verification (bytes 0 - 3)
#define VERIFY_MAGIC_NUMBER(bytecode) { \
    int magic_value = get_int(bytecode, 0); \
    if (magic_value != MAGIC_NUMBER) { \
        PD("invalid magic number: expected 0x%08X but got 0x%08X", MAGIC_NUMBER, magic_value); \
    } \
}

// Bytecode version (bytes 4 - 7)
#define VERIFY_VERSION(bytecode) { \
    int version = get_int(bytecode, 4); \
    if (version != VERSION) { \
        PD("incompatible bytecode version: bytecode was compiled with version %d but runtime expects version %d", version, VERSION); \
    } \
}

// Get bytecode size (bytes 8 - 15)
#define VERIFY_BYTECODE_SIZE(bytecode, outvariable) { \
    long size = get_long(bytecode, 8); \
    if (size == 0) { \
        PD("bytecode size is 0"); \
    } \
    outvariable = size; \
}

#define DUMP_BYTECODE(bytecode, size) { \
    for (size_t i = 0; i < size; i++) { \
        printf("[%02zu]: %02X = %d\n", i, bytecode[i], bytecode[i]); \
    } \
    printf("\n"); \
}

vm_t *instance = NULL;

INTERNAL
int get_int(uint8_t *bytecode, size_t ip) {
    int value = 0;
    for (size_t i = 0; i < 4; i++) {
        value = value | (bytecode[ip + i] << (i * 8));
    }
    return value;
}

INTERNAL
long get_long(uint8_t *bytecode, size_t ip) {
    long value = 0;
    for (size_t i = 0; i < 8; i++) {
        value = value | (bytecode[ip + i] << (i * 8));
    }
    return value;
}

INTERNAL
float get_float(uint8_t *bytecode, size_t ip) {
    union FloatBytes {
        float f;
        uint8_t bytes[4];
    };
    union FloatBytes bytes;
    bytes.f = 0;
    for (size_t i = 0; i < 4; i++) {
        bytes.bytes[i] = bytecode[ip + i];
    }
    return bytes.f;
}

INTERNAL
double get_double(uint8_t *bytecode, size_t ip) {
    union DoubleBytes {
        double d;
        uint8_t bytes[8];
    };
    union DoubleBytes bytes;
    bytes.d = 0;
    for (size_t i = 0; i < 8; i++) {
        bytes.bytes[i] = bytecode[ip + i];
    }
    return bytes.d;
}

INTERNAL
char* get_string(uint8_t *_bytecode, size_t _ip) {
    char* str = string_allocate("");
    while (_bytecode[_ip] != 0) {
        str = string_append_char(str, _bytecode[_ip]);
        _ip++;
    }
    return str;
}

INTERNAL
void do_mul(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        long result = (long)_lhs->value.i32 * _rhs->value.i32;
        if (result >= INT32_MIN && result <= INT32_MAX) {
            PUSH(object_new_int((int)result));
            return;
        }
        PUSH(object_new_double((double)result));
        return;
    }

    // Fallback path using coercion
    double lhs_value = number_coerce_to_double(_lhs);
    double rhs_value = number_coerce_to_double(_rhs);
    double result = lhs_value * rhs_value;

    // Try to preserve integer types if possible
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
}

INTERNAL
void do_add(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        // Use XOR trick to detect overflow
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int sum = a + b;
        if (((a ^ sum) & (b ^ sum)) < 0) {
            // Overflow occurred, promote to double
            PUSH(object_new_double((double)a + (double)b));
            return;
        }
        PUSH(object_new_int(sum));
        return; 
    }

    // Fast path for strings
    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* str = string_allocate("");
        str = string_append(str, (char*)_lhs->value.opaque);
        str = string_append(str, (char*)_rhs->value.opaque);
        PUSH(object_new_string(str));
        return;
    }

    // Handle string + non-string case
    if (OBJECT_TYPE_STRING(_lhs) || OBJECT_TYPE_STRING(_rhs)) {
        goto ERROR;
    }

    // Fallback path using coercion
    double lhs_value = number_coerce_to_double(_lhs);
    double rhs_value = number_coerce_to_double(_rhs);
    double result = lhs_value + rhs_value;

    // Try to preserve integer types if possible
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
}

INTERNAL
void do_sub(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int diff = a - b;
        if (((a ^ b) & (a ^ diff)) < 0) {
            PUSH(object_new_double((double)a - (double)b));
            return;
        }
        PUSH(object_new_int(diff));
        return;
    }

    // Fallback path using coercion
    double lhs_value = number_coerce_to_double(_lhs);
    double rhs_value = number_coerce_to_double(_rhs);
    double result = lhs_value - rhs_value;

    // Try to preserve integer types if possible
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
}

INTERNAL void do_shl(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a << b;
        PUSH(object_new_int(result));
        return;
    }

    // Fallback path using coercion
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    long result = lhs_value << rhs_value;

    // Check if result can be represented as an int
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double((double)result));
    return;
    ERROR:;
}

INTERNAL void do_shr(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a >> b;
        PUSH(object_new_int(result));
        return;
    }

    // Fallback path using coercion
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    long result = lhs_value >> rhs_value;

    // Try to preserve integer types if possible
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double((double)result));
    return;
    ERROR:;
}

INTERNAL void do_cmp_lt(object_t *_lhs, object_t *_rhs) {
    if (!object_is_number(_lhs) || !object_is_number(_rhs)) {
        goto ERROR;
    }
    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value < rhs_value) {
        PUSH(instance->tobj);
        return;
    }
    PUSH(instance->fobj);
    return;
    ERROR:;
}

INTERNAL void do_cmp_lte(object_t *_lhs, object_t *_rhs) {
    if (!object_is_number(_lhs) || !object_is_number(_rhs)) {
        goto ERROR;
    }
    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value <= rhs_value) {
        PUSH(instance->tobj);
        return;
    }
    PUSH(instance->fobj);
    return;
    ERROR:;
}

INTERNAL void do_cmp_gt(object_t *_lhs, object_t *_rhs) {
    if (!object_is_number(_lhs) || !object_is_number(_rhs)) {
        goto ERROR;
    }
    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value > rhs_value) {
        PUSH(instance->tobj);
        return;
    }
    PUSH(instance->fobj);
    return;
    ERROR:;
}

INTERNAL void do_cmp_gte(object_t *_lhs, object_t *_rhs) {
    if (!object_is_number(_lhs) || !object_is_number(_rhs)) {
        goto ERROR;
    }
    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value >= rhs_value) {
        PUSH(instance->tobj);
        return;
    }
    PUSH(instance->fobj);
    return;
    ERROR:;
}

INTERNAL void do_cmp_eq(object_t *_lhs, object_t *_rhs) {
    if (object_is_number(_lhs) && object_is_number(_rhs)) {
        long lhs_value = number_coerce_to_long(_lhs);
        long rhs_value = number_coerce_to_long(_rhs);
        if (lhs_value == rhs_value) {
            PUSH(instance->tobj);
            return;
        }
        PUSH(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* lhs_str = (char*)_lhs->value.opaque;
        char* rhs_str = (char*)_rhs->value.opaque;
        if (strcmp(lhs_str, rhs_str) == 0) {
            PUSH(instance->tobj);
            return;
        }
        PUSH(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_NULL(_lhs) && OBJECT_TYPE_NULL(_rhs)) {
        PUSH(instance->tobj);
        return;
    }

    PUSH(instance->fobj);
    return;
    ERROR:;
}

INTERNAL void do_cmp_ne(object_t *_lhs, object_t *_rhs) {
    if (object_is_number(_lhs) && object_is_number(_rhs)) {
        long lhs_value = number_coerce_to_long(_lhs);
        long rhs_value = number_coerce_to_long(_rhs);
        if (lhs_value != rhs_value) {
            PUSH(instance->tobj);
            return;
        }
        PUSH(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* lhs_str = (char*)_lhs->value.opaque;
        char* rhs_str = (char*)_rhs->value.opaque;
        if (strcmp(lhs_str, rhs_str) != 0) {
            PUSH(instance->tobj);
            return;
        }
        PUSH(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_NULL(_lhs) || OBJECT_TYPE_NULL(_rhs)) {
        PUSH(instance->fobj);
        return;
    }

    PUSH(instance->tobj);
    return;
    ERROR:;
}

INTERNAL void do_and(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a & b;
        PUSH(object_new_int(result));
        return;
    }
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    long result = lhs_value & rhs_value;
    // Check if result can be represented as an int
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
    } else {
        PUSH(object_new_double((double)result));
    }
    return;
    ERROR:;
}

INTERNAL void do_or(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a | b;
        PUSH(object_new_int(result));
        return;
    }
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    long result = lhs_value | rhs_value;
    // Check if result can be represented as an int
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
    } else {
        PUSH(object_new_double((double)result));
    }
    return;
    ERROR:;
}

INTERNAL void do_xor(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a ^ b;
        PUSH(object_new_int(result));
        return;
    }
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    long result = lhs_value ^ rhs_value;
    // Check if result can be represented as an int
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
    } else {
        PUSH(object_new_double((double)result));
    }
    return;
    ERROR:;
}

INTERNAL void vm_execute(env_t* _env, size_t _header_size, size_t _ip, uint8_t *_bytecode) {
    ASSERTNULL(instance, "VM is not initialized");
    
    // Get bytecode size
    size_t bytecode_size;
    VERIFY_BYTECODE_SIZE(_bytecode, bytecode_size);

    char* file_path = 
        get_string(_bytecode, _header_size);

    // Add 1 to skip the null terminator of file_path string
    char* file_name = 
        get_string(_bytecode, _header_size + strlen(file_path) + 1);

    // printf("file_path: '%s'\n", file_path);
    // printf("file_name: '%s'\n", file_name);

    // Calculate the starting IP
    size_t ip = 
        _header_size + 
        (strlen(file_path) + 1) + 
        (strlen(file_name) + 1) + 
        _ip;

    // Shallow copy the bytecode
    uint8_t *bytecode = _bytecode;

    while (ip < bytecode_size) {
        opcode_t opcode = bytecode[ip++];

        if (instance->allocation_counter >= GC_ALLOCATION_THRESHOLD) gc_collect(instance);

        // Check if opcode is valid
        switch (opcode) {
            case OPCODE_LOAD_NAME: {
                char* name = get_string(bytecode, ip);
                instance->name_resolver(
                    instance, 
                    _env, name
                );
                FORWARD(strlen(name) + 1);
                break;
            }
            case OPCODE_LOAD_INT: {
                PUSH(object_new_int(
                    get_int(bytecode, ip)
                ));
                FORWARD(4);
                break;
            }
            case OPCODE_LOAD_DOUBLE: {
                PUSH(object_new_double(
                    get_double(bytecode, ip)
                ));
                FORWARD(8);
                break;
            }
            case OPCODE_LOAD_BOOL: {
                if (bytecode[ip] == 1) {
                    PUSH(instance->tobj);
                } else {
                    PUSH(instance->fobj);
                }
                FORWARD(1);
                break;
            }
            case OPCODE_LOAD_STRING: {
                char* str = get_string(bytecode, ip);
                PUSH(object_new_string(
                    string_allocate(str)
                ));
                FORWARD(strlen(str) + 1);
                break;
            }
            case OPCODE_LOAD_NULL: {
                PUSH(instance->null);
                break;
            }
            case OPCODE_STORE_NAME: {
                char* name = get_string(bytecode, ip);
                env_put(_env, name, POPP());
                FORWARD(strlen(name) + 1);
                break;
            }
            case OPCODE_ADD: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_add(obj1, obj2);
                break;
            }
            case OPCODE_SUB: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_sub(obj1, obj2);
                break;
            }
            case OPCODE_SHL: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_shl(obj1, obj2);
                break;
            }
            case OPCODE_SHR: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_shr(obj1, obj2);
                break;
            }
            case OPCODE_CMP_LT: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_lt(obj1, obj2);
                break;
            }
            case OPCODE_CMP_LTE: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_lte(obj1, obj2);
                break;
            }
            case OPCODE_CMP_GT: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_gt(obj1, obj2);
                break;
            }
            case OPCODE_CMP_GTE: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_gte(obj1, obj2);
                break;
            }
            case OPCODE_CMP_EQ: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_eq(obj1, obj2);
                break;
            }
            case OPCODE_CMP_NE: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_cmp_ne(obj1, obj2);
                break;
            }
            case OPCODE_AND: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_and(obj1, obj2);
                break;
            }
            case OPCODE_OR: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_or(obj1, obj2);
                break;
            }
            case OPCODE_XOR: {
                object_t *obj1 = POPP();
                object_t *obj2 = POPP();
                do_xor(obj1, obj2);
                break;
            }
            case OPCODE_POP_JUMP_IF_FALSE: {
                int jump_offset = get_int(bytecode, ip);
                if (!object_is_truthy(POPP())) {
                    ip += jump_offset;
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_POP_JUMP_IF_TRUE: {
                int jump_offset = get_int(bytecode, ip);
                if (object_is_truthy(POPP())) {
                    ip += jump_offset;
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_JUMP_IF_FALSE_OR_POP: {
                int jump_offset = get_int(bytecode, ip);
                object_t *obj = PEEK();
                if (!object_is_truthy(obj)) {
                    ip += jump_offset;
                } else {
                    POPP();
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_JUMP_IF_TRUE_OR_POP: {
                int jump_offset = get_int(bytecode, ip);
                object_t *obj = PEEK();
                if (object_is_truthy(obj)) {
                    ip += jump_offset;
                } else {
                    POPP();
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_JUMP_FORWARD: {
                int jump_offset = get_int(bytecode, ip);
                ip += jump_offset;
                break;
            }
            case OPCODE_POPTOP: {
                printf("TOP: %s\n", object_to_string(POPP()));
                break;
            }
            case OPCODE_RETURN: {
                return;
            }
            default: {
                DUMP_BYTECODE(bytecode, bytecode_size);
                PD("unknown opcode %02X at %02zu", opcode, ip);
            }
        }
    }
}

// -----------------------------

DLLEXPORT void vm_init() {
    instance = (vm_t *) malloc(sizeof(vm_t));
    ASSERTNULL(instance, ERROR_ALLOCATING_VM);
    // evaluation stack
    instance->evaluation_stack = 
        (object_t **) malloc(sizeof(object_t *) * EVALUATION_STACK_SIZE);
    ASSERTNULL(instance->evaluation_stack, ERROR_ALLOCATING_EVALUATION_STACK);
    instance->sp = 0;
    // counter
    instance->allocation_counter = 0;
    // name resolver
    instance->name_resolver = vm_name_resolver;
    // root object
    instance->root = object_new(OBJECT_TYPE_OBJECT);
    // singleton null
    instance->null = object_new(OBJECT_TYPE_NULL);
    // singleton boolean
    instance->tobj = object_new_bool(true);
    instance->fobj = object_new_bool(false);
}

DLLEXPORT void vm_set_name_resolver(vm_t* _vm, vm_name_resolver_t _resolver) {
    _vm->name_resolver = _resolver;
}

DLLEXPORT void vm_name_resolver(vm_t* _vm, env_t* _env, char* _name) {
    if (!env_has(_env, _name, true)) {
        PD("variable %s not found", _name);   
    }
    PUSH(env_get(_env, _name));
}

DLLEXPORT void vm_run_main(uint8_t* _bytecode) {
    ASSERTNULL(instance, ERROR_VM_NOT_INITIALIZED);
    // Verify magic number
    VERIFY_MAGIC_NUMBER(_bytecode);
    // Verify version
    VERIFY_VERSION(_bytecode);
    // Skip magic number
    env_t* env = env_new(NULL);
    vm_execute(env, LENGTH_OF_MAGIC_NUMBER+LENGTH_OF_VERSION+LENGTH_OF_BYTECODE_SIZE, 0, _bytecode);
    printf("VM executed\n");
}
