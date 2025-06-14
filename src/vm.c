#include "api/core/object.h"
#include "api/core/vm.h"
#include "error.h"
#include "internal.h"
#include "object.h"
#include "opcode.h"
#include "type.h"

typedef struct vm_struct {
    // evaluation stack
    object_t **evaluation_stack;
    size_t sp;
    // name resolver
    vm_name_resolver_t name_resolver;
    // root object
    object_t *root;
    // singleton null
    object_t *null;
    // singleton boolean
    object_t *tobj;
    object_t *fobj;
} vm_t;

#define PUSH_NEW(obj) { \
    ASSERTNULL(instance->evaluation_stack, "Evaluation stack is full"); \
    instance->evaluation_stack[instance->sp++] = obj; \
    obj->next = instance->root; \
    instance->root = obj; \
}

#define PUSH(obj) { \
    ASSERTNULL(instance->evaluation_stack, "Evaluation stack is full"); \
    instance->evaluation_stack[instance->sp++] = obj; \
}

#define FORWARD(size) { \
    ip += size; \
}

#define PEEK() (instance->evaluation_stack[instance->sp - 1])
#define POP() (instance->evaluation_stack[--instance->sp])

#define LENGTH_OF_MAGIC_NUMBER  4
#define LENGTH_OF_BYTECODE_SIZE 8

// Bytecode verification (bytes 0 - 3)
#define VERIFY_MAGIC_NUMBER(bytecode) { \
    int magic_value = get_int(bytecode, 0); \
    if (magic_value != MAGIC_NUMBER) { \
        PD("magic number is invalid"); \
    } \
}

// Get bytecode size (bytes 4 - 11)
#define VERIFY_BYTECODE_SIZE(bytecode, outvariable) { \
    long size = get_long(bytecode, 4); \
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
        int64_t result = (int64_t)_lhs->value.i32 * _rhs->value.i32;
        if (result >= INT32_MIN && result <= INT32_MAX) {
            PUSH_NEW(object_new_int((int32_t)result));
            return;
        }
        if (result >= INT64_MIN && result <= INT64_MAX) {
            PUSH_NEW(object_new_long(result)); 
            return;
        }
        PUSH_NEW(object_new_double((double)result));
        return;
    }

    // Fast path for longs
    if (OBJECT_TYPE_LONG(_lhs) && OBJECT_TYPE_LONG(_rhs)) {
        // Check if multiplication would overflow
        int64_t a = _lhs->value.i64;
        int64_t b = _rhs->value.i64;
        if (a == 0 || b == 0) {
            PUSH_NEW(object_new_long(0));
            return;
        }
        if (a > 0) {
            if (b > 0) {
                if (a > INT64_MAX / b) goto double_path;
            } else {
                if (b < INT64_MIN / a) goto double_path;
            }
        } else {
            if (b > 0) {
                if (a < INT64_MIN / b) goto double_path;
            } else {
                if (a != 0 && b < INT64_MAX / a) goto double_path;
            }
        }
        PUSH_NEW(object_new_long(a * b));
        return;
    }

    double_path:;
    double lhs_value;
    double rhs_value;

    if (OBJECT_TYPE_INT(_lhs) || OBJECT_TYPE_LONG(_lhs) || 
        OBJECT_TYPE_FLOAT(_lhs) || OBJECT_TYPE_DOUBLE(_lhs)) {
        lhs_value = number_coerce_to_double(_lhs);
    } else if (OBJECT_TYPE_STRING(_lhs)) {
        if (!string_is_number((char*)_lhs->value.opaque)) goto ERROR;
        lhs_value = number_coerce_to_double(_lhs);
    } else {
        goto ERROR;
    }

    if (OBJECT_TYPE_INT(_rhs) || OBJECT_TYPE_LONG(_rhs) ||
        OBJECT_TYPE_FLOAT(_rhs) || OBJECT_TYPE_DOUBLE(_rhs)) {
        rhs_value = number_coerce_to_double(_rhs);
    } else if (OBJECT_TYPE_STRING(_rhs)) {
        if (!string_is_number((char*)_rhs->value.opaque)) goto ERROR;
        rhs_value = number_coerce_to_double(_rhs);
    } else {
        goto ERROR;
    }

    double result = lhs_value * rhs_value;

    if (result == (double)(int32_t)result) {
        PUSH_NEW(object_new_int((int32_t)result));
    } else if (result == (double)(int64_t)result) {
        PUSH_NEW(object_new_long((int64_t)result));
    } else {
        PUSH_NEW(object_new_double(result));
    }
    return;

    ERROR:;
}

INTERNAL
void do_add(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        // Use XOR trick to detect overflow
        int32_t a = _lhs->value.i32;
        int32_t b = _rhs->value.i32;
        int32_t sum = a + b;
        if (((a ^ sum) & (b ^ sum)) < 0) {
            // Overflow occurred, promote to double
            PUSH_NEW(object_new_double((double)a + (double)b));
        } else {
            PUSH_NEW(object_new_int(sum));
        }
        return;
    }

    // Fast path for longs 
    if (OBJECT_TYPE_LONG(_lhs) && OBJECT_TYPE_LONG(_rhs)) {
        int64_t a = _lhs->value.i64;
        int64_t b = _rhs->value.i64;
        int64_t sum = a + b;
        if (((a ^ sum) & (b ^ sum)) < 0) {
            PUSH_NEW(object_new_double((double)a + (double)b));
        } else {
            PUSH_NEW(object_new_long(sum));
        }
        return;
    }

    // Fast path for strings
    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* str = string_allocate("");
        str = string_append(str, (char*)_lhs->value.opaque);
        str = string_append(str, (char*)_rhs->value.opaque);
        PUSH_NEW(object_new_string(str));
        return;
    }

    // Fallback path - convert to doubles
    double lhs_value;
    double rhs_value;

    // Coerce left operand
    if (OBJECT_TYPE_INT(_lhs) || OBJECT_TYPE_LONG(_lhs) || 
        OBJECT_TYPE_FLOAT(_lhs) || OBJECT_TYPE_DOUBLE(_lhs)) {
        lhs_value = number_coerce_to_double(_lhs);
    } else if (OBJECT_TYPE_STRING(_lhs)) {
        goto ERROR; // String + non-string not allowed
    } else {
        goto ERROR;
    }

    // Coerce right operand
    if (OBJECT_TYPE_INT(_rhs) || OBJECT_TYPE_LONG(_rhs) ||
        OBJECT_TYPE_FLOAT(_rhs) || OBJECT_TYPE_DOUBLE(_rhs)) {
        rhs_value = number_coerce_to_double(_rhs);
    } else if (OBJECT_TYPE_STRING(_rhs)) {
        if (!string_is_number((char*)_rhs->value.opaque)) goto ERROR;
        rhs_value = number_coerce_to_double(_rhs);
    } else {
        goto ERROR;
    }

    double result = lhs_value + rhs_value;
    double intpart;

    // Try to preserve integer types if possible
    if (modf(result, &intpart) == 0.0) {
        if (result <= INT32_MAX && result >= INT32_MIN) {
            PUSH_NEW(object_new_int((int32_t)result));
            return;
        }
        if (result <= INT64_MAX && result >= INT64_MIN) {
            PUSH_NEW(object_new_long((int64_t)result));
            return;
        }
    }

    PUSH_NEW(object_new_double(result));
    return;

    ERROR:;
}

INTERNAL
void do_sub(object_t *_lhs, object_t *_rhs) {
    // Fallback path - convert to doubles
    double lhs_value;
    double rhs_value;

    // Coerce left operand
    if (OBJECT_TYPE_INT(_lhs) || OBJECT_TYPE_LONG(_lhs) ||
        OBJECT_TYPE_FLOAT(_lhs) || OBJECT_TYPE_DOUBLE(_lhs)) {
        lhs_value = number_coerce_to_double(_lhs);
    } else if (OBJECT_TYPE_STRING(_lhs)) {
        if (!string_is_number((char*)_lhs->value.opaque)) goto ERROR;
        lhs_value = number_coerce_to_double(_lhs);
    } else {
        goto ERROR;
    }

    // Coerce right operand
    if (OBJECT_TYPE_INT(_rhs) || OBJECT_TYPE_LONG(_rhs) ||
        OBJECT_TYPE_FLOAT(_rhs) || OBJECT_TYPE_DOUBLE(_rhs)) {
        rhs_value = number_coerce_to_double(_rhs);
    } else if (OBJECT_TYPE_STRING(_rhs)) {
        if (!string_is_number((char*)_rhs->value.opaque)) goto ERROR;
        rhs_value = number_coerce_to_double(_rhs);
    } else {
        goto ERROR;
    }

    double result = lhs_value - rhs_value;
    double intpart;

    // Try to preserve integer types if possible
    if (modf(result, &intpart) == 0.0) {
        if (result <= INT32_MAX && result >= INT32_MIN) {
            PUSH_NEW(object_new_int((int32_t)result));
            return;
        }
        if (result <= INT64_MAX && result >= INT64_MIN) {
            PUSH_NEW(object_new_long((int64_t)result));
            return;
        }
    }

    PUSH_NEW(object_new_double(result));
    return;

    ERROR:;
}
INTERNAL
void vm_execute(env_t* _env, size_t _header_size, size_t _ip, uint8_t *_bytecode) {
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
                PUSH_NEW(object_new_int(
                    get_int(bytecode, ip)
                ));
                FORWARD(4);
                break;
            }
            case OPCODE_LOAD_LONG: {
                PUSH_NEW(object_new_long(
                    get_long(bytecode, ip)
                ));
                FORWARD(8);
                break;
            }
            case OPCODE_LOAD_FLOAT: {
                PUSH_NEW(object_new_float(
                    get_float(bytecode, ip)
                ));
                FORWARD(4);
                break;
            }
            case OPCODE_LOAD_DOUBLE: {
                PUSH_NEW(object_new_double(
                    get_double(bytecode, ip)
                ));
                FORWARD(8);
                break;
            }
            case OPCODE_LOAD_BOOL: {
                if (bytecode[ip] == 1) {
                    PUSH_NEW(instance->tobj);
                } else {
                    PUSH_NEW(instance->fobj);
                }
                FORWARD(1);
                break;
            }
            case OPCODE_LOAD_STRING: {
                char* str = get_string(bytecode, ip);
                PUSH_NEW(object_new_string(
                    string_allocate(str)
                ));
                FORWARD(strlen(str) + 1);
                break;
            }
            case OPCODE_LOAD_NULL: {
                PUSH_NEW(instance->null);
                break;
            }
            case OPCODE_STORE_NAME: {
                char* name = get_string(bytecode, ip);
                env_put(_env, name, POP());
                FORWARD(strlen(name) + 1);
                break;
            }
            case OPCODE_ADD: {
                object_t *obj1 = POP();
                object_t *obj2 = POP();
                do_add(obj1, obj2);
                break;
            }
            case OPCODE_SUB: {
                object_t *obj1 = POP();
                object_t *obj2 = POP();
                do_sub(obj1, obj2);
                break;
            }
            case OPCODE_POP_JUMP_IF_FALSE: {
                int jump_offset = get_int(bytecode, ip);
                if (!object_is_truthy(POP())) {
                    ip += jump_offset;
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_POP_JUMP_IF_TRUE: {
                int jump_offset = get_int(bytecode, ip);
                if (object_is_truthy(POP())) {
                    ip += jump_offset;
                } else {
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
                printf("TOP: %s\n", object_to_string(POP()));
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

    // Skip magic number
    env_t* env = env_new(NULL);
    vm_execute(env, LENGTH_OF_MAGIC_NUMBER+LENGTH_OF_BYTECODE_SIZE, 0, _bytecode);
    printf("VM executed\n");
}
