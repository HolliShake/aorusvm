#include "api/core/object.h"
#include "api/core/vm.h"
#include "code.h"
#include "error.h"
#include "gc.h"
#include "internal.h"
#include "object.h"
#include "opcode.h"
#include "type.h"
#include "vm.h"

#define GC_ALLOCATION_THRESHOLD 1000

#define PUSH(obj) vm_push(obj)

#define PUSH_REF(obj) { \
    ASSERTNULL(instance->evaluation_stack, "Evaluation stack is not initialized"); \
    if (instance->sp >= EVALUATION_STACK_SIZE) { \
        PD("Stackoverflow"); \
    } \
    instance->evaluation_stack[instance->sp++] = obj; \
}

#define JUMP(offset) { \
    ip = offset; \
}

#define FORWARD(size) { \
    ip += size; \
}

#define TOP (instance->sp - 1)
#define BOT (instance->sp)

#define OPCODE (bytecode[ip])

#define PEEK() (instance->evaluation_stack[instance->sp - 1])
#define POPP() (instance->evaluation_stack[--instance->sp])
#define POPN(_times) { \
    for (size_t i = 0; i < _times; i++) \
        POPP(); \
}

#define SAVE_FUNCTION(function) { \
    /* Use direct lookup instead of linear search */ \
    if (instance->function_table_size > 0 && \
        instance->function_table_item[instance->function_table_size-1] == function) { \
        /* Fast path: function was just added */ \
    } else { \
        /* Check if function already exists using pointer comparison */ \
        size_t i = 0; \
        while (i < instance->function_table_size && instance->function_table_item[i] != function) i++; \
        if (i == instance->function_table_size) { \
            /* Only reallocate when needed, and grow by more than 1 to reduce reallocation frequency */ \
            if (instance->function_table_size % 16 == 0) { \
                instance->function_table_item = (code_t**)realloc( \
                    instance->function_table_item, \
                    sizeof(code_t*) * (instance->function_table_size + 17) \
                ); \
            } \
            instance->function_table_item[instance->function_table_size++] = function; \
            instance->function_table_item[instance->function_table_size] = NULL; \
        } \
    } \
}

#define DUMP_STACK() { \
    for (size_t i = instance->sp-1; i > 0; i--) { \
        if (i == instance->sp-1) { \
            printf("TOP: "); \
        } else if (i == 1) { \
            printf("BOT: "); \
        } else { \
            printf("     "); \
        } \
        printf("[%02zu]: %s", i, object_to_string(instance->evaluation_stack[i])); \
        if (i - 1 > 0) printf("\n"); \
    } \
    printf("\n"); \
}

vm_t *instance = NULL;
size_t save_top = 0;
size_t save_bot = 0;

INTERNAL vm_block_signal_t vm_execute(env_t* _env, size_t _ip, code_t* _code);

INTERNAL bool vm_object_is_in_root(object_t* _obj) {
    object_t* current = instance->root;
    while (current != NULL) {
        if (current == _obj) {
            return true;
        }
        current = current->next;
    }
    return false;
}

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
void* get_memory(uint8_t* _bytecode, size_t _ip) {
    uintptr_t value = 0;
    for (size_t i = 0; i < 8; i++) {
        value |= ((uintptr_t)_bytecode[_ip + i] << (i * 8));
    }
    return (void*)value;
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

INTERNAL void vm_enqueue(async_t* _async) {
    instance->queque[instance->aq++] = _async;
}

INTERNAL async_t* vm_dequeue() {
    if (instance->aq == 0) {
        return NULL;
    }
    async_t* async = instance->queque[0];
    // Shift left
    for (size_t i = 0; i < instance->aq; i++) {
        instance->queque[i] = instance->queque[i+1];
    }
    instance->queque[instance->aq-1] = NULL;
    instance->aq--;
    return async;
}

INTERNAL void rotate2() {
    // A B -> B A
    object_t* A = instance->evaluation_stack[instance->sp-1];
    object_t* B = instance->evaluation_stack[instance->sp-2];
    instance->evaluation_stack[instance->sp-1] = B;
    instance->evaluation_stack[instance->sp-2] = A;
}

INTERNAL void rotate3() {
    // A B C -> C A B
    object_t* A = instance->evaluation_stack[instance->sp-1];
    object_t* B = instance->evaluation_stack[instance->sp-2];
    object_t* C = instance->evaluation_stack[instance->sp-3];
    instance->evaluation_stack[instance->sp-1] = C;
    instance->evaluation_stack[instance->sp-2] = A;
    instance->evaluation_stack[instance->sp-3] = B;
}

INTERNAL void rotate4() {
    // A B C D -> D A B C
    object_t *A = instance->evaluation_stack[instance->sp-1];
    object_t *B = instance->evaluation_stack[instance->sp-2];
    object_t *C = instance->evaluation_stack[instance->sp-3];
    object_t *D = instance->evaluation_stack[instance->sp-4];
    instance->evaluation_stack[instance->sp-1] = D;
    instance->evaluation_stack[instance->sp-2] = A;
    instance->evaluation_stack[instance->sp-3] = B;
    instance->evaluation_stack[instance->sp-4] = C;
}

INTERNAL void rotate_left_3() {
    // A B C -> B C A
    object_t* A = instance->evaluation_stack[instance->sp-1];
    object_t* B = instance->evaluation_stack[instance->sp-2];
    object_t* C = instance->evaluation_stack[instance->sp-3];
    instance->evaluation_stack[instance->sp-1] = B;
    instance->evaluation_stack[instance->sp-2] = C;
    instance->evaluation_stack[instance->sp-3] = A;
}

INTERNAL
void do_increment(bool _is_postfix, object_t* _obj) {
    if (OBJECT_TYPE_INT(_obj)) {
        long result = (long)_obj->value.i32 + 1;
        if (result >= INT32_MIN && result <= INT32_MAX) {
            PUSH(object_new_int((int)result));
            return;
        }
        PUSH(object_new_double((double)result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_obj)) {
        goto ERROR;
    }

    double result = number_coerce_to_double(_obj);
    result += 1;
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    if (_is_postfix) rotate2();
    char* message = string_format(
        "cannot increment type %s", 
        object_type_to_string(_obj)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_decrement(bool _is_postfix, object_t* _obj) {
    if (OBJECT_TYPE_INT(_obj)) {
        long result = (long)_obj->value.i32 - 1;
        if (result >= INT32_MIN && result <= INT32_MAX) {
            PUSH(object_new_int((int)result));
            return;
        }
        PUSH(object_new_double((double)result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_obj)) {
        goto ERROR;
    }

    double result = number_coerce_to_double(_obj);
    result -= 1;
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    if (_is_postfix) rotate2();
    char* message = string_format(
        "cannot decrement type %s", 
        object_type_to_string(_obj)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_unary_plus(object_t* _obj) {
    if (OBJECT_TYPE_INT(_obj)) {
        int result = _obj->value.i32;
        PUSH(object_new_int(_obj->value.i32));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_obj)) {
        goto ERROR;
    }

    double result = number_coerce_to_double(_obj);
    result = +result;
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    char* message = string_format("cannot unary plus type %s", object_type_to_string(_obj));
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_unary_minus(object_t* _obj) {
    if (OBJECT_TYPE_INT(_obj)) {
        int result = -_obj->value.i32;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_obj)) {
        goto ERROR;
    }

    double result = number_coerce_to_double(_obj);
    result = -result;
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    char* message = string_format("cannot unary minus type %s", object_type_to_string(_obj));
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_not(object_t* _obj) {
    if (OBJECT_TYPE_BOOL(_obj)) {
        PUSH(object_new_bool(!_obj->value.i32));
        return;
    }

    bool result = object_is_truthy(_obj);
    PUSH(object_new_bool(!result));
    return;
}

INTERNAL
void do_bitwise_not(object_t* _obj) {
    if (OBJECT_TYPE_INT(_obj)) {
        int result = ~_obj->value.i32;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_obj)) {
        goto ERROR;
    }

    long result = number_coerce_to_long(_obj);
    result = ~result;
    if (result >= INT32_MIN && result <= INT32_MAX) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double((double)result));
    return;
    ERROR:;
    char* message = string_format("cannot bitwise not type %s", object_type_to_string(_obj));
    PUSH(object_new_error(message, true));
    free(message);
    return;
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

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot multiply type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_div(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        if (b == 0) {
            PUSH(object_new_error("division by zero", true));
            return;
        }
        int result = a / b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    // Fallback path using coercion
    double lhs_value = number_coerce_to_double(_lhs);
    double rhs_value = number_coerce_to_double(_rhs);
    if (rhs_value == 0) {
        PUSH(object_new_error("division by zero", true));
        return;
    }
    double result = lhs_value / rhs_value;

    // Try to preserve integer types if possible
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    char* message = string_format(
        "cannot divide type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL
void do_mod(object_t *_lhs, object_t *_rhs) {
    // Fast path for integers
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        if (b == 0) {
            PUSH(object_new_error("division by zero", true));
            return;
        }
        int result = a % b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    // Fallback path using coercion
    double lhs_value = number_coerce_to_double(_lhs);
    double rhs_value = number_coerce_to_double(_rhs);
    if (rhs_value == 0) {
        PUSH(object_new_error("division by zero", true));
        return;
    }
    double result = fmod(lhs_value, rhs_value);

    // Try to preserve integer types if possible
    if (result == (double)(int)result && result <= INT32_MAX && result >= INT32_MIN) {
        PUSH(object_new_int((int)result));
        return;
    }
    PUSH(object_new_double(result));
    return;
    ERROR:;
    char* message = string_format(
        "cannot modulo type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
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

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
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
    char* message = string_format(
        "cannot add type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
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

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot subtract type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_shl(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a << b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot shift left type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_shr(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a >> b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot shift right type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_lt(object_t *_lhs, object_t *_rhs) {
    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }
    
    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value < rhs_value) {
        PUSH_REF(instance->tobj);
        return;
    }
    PUSH_REF(instance->fobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare less than type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_lte(object_t *_lhs, object_t *_rhs) {
    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value <= rhs_value) {
        PUSH_REF(instance->tobj);
        return;
    }
    PUSH_REF(instance->fobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare less than or equal to type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_gt(object_t *_lhs, object_t *_rhs) {
    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value > rhs_value) {
        PUSH_REF(instance->tobj);
        return;
    }
    PUSH_REF(instance->fobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare greater than type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_gte(object_t *_lhs, object_t *_rhs) {
    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
    }

    // Coerce to long to avoid floating point comparisons
    long lhs_value = number_coerce_to_long(_lhs);
    long rhs_value = number_coerce_to_long(_rhs);
    // Compare the long values
    if (lhs_value >= rhs_value) {
        PUSH_REF(instance->tobj);
        return;
    }
    PUSH_REF(instance->fobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare greater than or equal to type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_eq(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_NUMBER(_lhs) && OBJECT_TYPE_NUMBER(_rhs)) {
        long lhs_value = number_coerce_to_long(_lhs);
        long rhs_value = number_coerce_to_long(_rhs);
        if (lhs_value == rhs_value) {
            PUSH_REF(instance->tobj);
            return;
        }
        PUSH_REF(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* lhs_str = (char*)_lhs->value.opaque;
        char* rhs_str = (char*)_rhs->value.opaque;
        if (strcmp(lhs_str, rhs_str) == 0) {
            PUSH_REF(instance->tobj);
            return;
        }
        PUSH_REF(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_NULL(_lhs) && OBJECT_TYPE_NULL(_rhs)) {
        PUSH_REF(instance->tobj);
        return;
    }

    if (object_equals(_lhs, _rhs)) {
        PUSH_REF(instance->tobj);
        return;
    }

    PUSH_REF(instance->fobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare equal type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_cmp_ne(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_NUMBER(_lhs) && OBJECT_TYPE_NUMBER(_rhs)) {
        long lhs_value = number_coerce_to_long(_lhs);
        long rhs_value = number_coerce_to_long(_rhs);
        if (lhs_value != rhs_value) {
            PUSH_REF(instance->tobj);
            return;
        }
        PUSH_REF(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_STRING(_lhs) && OBJECT_TYPE_STRING(_rhs)) {
        char* lhs_str = (char*)_lhs->value.opaque;
        char* rhs_str = (char*)_rhs->value.opaque;
        if (strcmp(lhs_str, rhs_str) != 0) {
            PUSH_REF(instance->tobj);
            return;
        }
        PUSH_REF(instance->fobj);
        return;
    }

    if (OBJECT_TYPE_NULL(_lhs) || OBJECT_TYPE_NULL(_rhs)) {
        PUSH_REF(instance->fobj);
        return;
    }
    
    PUSH_REF(instance->tobj);
    return;
    ERROR:;
    char* message = string_format(
        "cannot compare not equal type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_and(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a & b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot bitwise and type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_or(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a | b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot bitwise or type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

INTERNAL void do_xor(object_t *_lhs, object_t *_rhs) {
    if (OBJECT_TYPE_INT(_lhs) && OBJECT_TYPE_INT(_rhs)) {
        int a = _lhs->value.i32;
        int b = _rhs->value.i32;
        int result = a ^ b;
        PUSH(object_new_int(result));
        return;
    }

    if (!OBJECT_TYPE_NUMBER(_lhs) || !OBJECT_TYPE_NUMBER(_rhs)) {
        goto ERROR;
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
    char* message = string_format(
        "cannot bitwise xor type(s) %s and %s", 
        object_type_to_string(_lhs), 
        object_type_to_string(_rhs)
    );
    PUSH(object_new_error(message, true));
    free(message);
    return;
}

/**
 * We will convert block into function and execute it.
 * 
 * @param _env The environment.
 * @param _closure The closure.
 */
INTERNAL void do_block(env_t* _parent_env, object_t* _closure, vm_block_signal_t* _signal) {
    // _closure is a short lived object here, we will convert it into function and execute it.
    code_t* code = (code_t*)_closure->value.opaque;
    env_t* block_env = env_new(_parent_env);
    
    block_env->closure = code->environment;
    *_signal = vm_execute(block_env, 0, code);
    
    if (*_signal == VmBlockSignalComplete) {
        POPP();
    }
    
    block_env->parent = NULL;
    block_env->closure = NULL;
    env_free(block_env);
}

INTERNAL object_t* get_property(object_t* _obj, char* _property_name) {
    // Fast path for regular objects
    if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_t* map = (hashmap_t*)_obj->value.opaque;
        return hashmap_has_string(map, _property_name) 
            ? hashmap_get_string(map, _property_name) 
            : NULL;
    }
    
    // Handle user types (classes)
    if (OBJECT_TYPE_USER_TYPE(_obj)) {
        user_type_t* user_type;
        
        for (object_t* current = _obj; current != NULL; current = user_type->super) {
            user_type = (user_type_t*)current->value.opaque;
            hashmap_t* map = (hashmap_t*)user_type->prototype->value.opaque;
            
            if (hashmap_has_string(map, _property_name)) {
                return hashmap_get_string(map, _property_name);
            }
        }
        return NULL;
    } 
    
    // Handle user type instances (objects)
    if (OBJECT_TYPE_USER_TYPE_INSTANCE(_obj)) {
        for (object_t* current = _obj; current != NULL;) {
            if (OBJECT_TYPE_USER_TYPE_INSTANCE(current)) {
                // Check instance properties first
                user_type_instance_t* instance = (user_type_instance_t*)current->value.opaque;
                hashmap_t* map = (hashmap_t*)instance->object->value.opaque;
                
                if (hashmap_has_string(map, _property_name)) {
                    return hashmap_get_string(map, _property_name);
                }
                
                // Move up to constructor/class
                current = instance->constructor;
            } 
            else if (OBJECT_TYPE_USER_TYPE(current)) {
                // Check class prototype
                user_type_t* user_type = (user_type_t*)current->value.opaque;
                hashmap_t* map = (hashmap_t*)user_type->prototype->value.opaque;
                
                if (hashmap_has_string(map, _property_name)) {
                    return hashmap_get_string(map, _property_name);
                }
                
                // Move up to superclass
                current = user_type->super;
            }
            else {
                break;
            }
        }
    }
    
    return NULL;
}

INTERNAL void set_property(object_t* _obj, char* _property_name, object_t* _value) {
    // Fast path: determine target hashmap directly based on object type
    hashmap_t* target_map = NULL;
    
    switch (_obj->type) {
        case OBJECT_TYPE_USER_TYPE:
            target_map = (hashmap_t*)(((user_type_t*)_obj->value.opaque)->prototype->value.opaque);
            break;
        case OBJECT_TYPE_USER_TYPE_INSTANCE:
            target_map = (hashmap_t*)(((user_type_instance_t*)_obj->value.opaque)->object->value.opaque);
            break;
        case OBJECT_TYPE_OBJECT:
            target_map = (hashmap_t*)_obj->value.opaque;
            break;
        default:
            // No valid target map for other types
            return;
    }
    
    // If no valid target map found, exit early
    if (!target_map) return;
    
    // Check if property already exists before creating key object
    bool exists = hashmap_has_string(target_map, _property_name);
    
    // Create string key object only once
    object_t* key = object_new_string(_property_name);
    
    // Set the property
    hashmap_put(target_map, key, _value);
    
    // Handle key object memory management
    if (exists) {
        // Property already exists, free our temporary key
        free((char*)key->value.opaque);
        free(key);
    } else {
        // New property, add key to heap management
        vm_to_heap(key);
    }
}

INTERNAL void do_index(object_t* _obj, object_t* _index) {
    // Check if object is a collection type
    if (!OBJECT_TYPE_COLLECTION(_obj)) {
        goto ERROR;
    }

    // Handle arrays
    if (OBJECT_TYPE_ARRAY(_obj)) {
        // Validate index is a number
        if (!OBJECT_TYPE_NUMBER(_index)) {
            char* message = string_format(
                "expected number, got \"%s\"", 
                object_to_string(_index)
            );
            PUSH(object_new_error(message, true));
            free(message);
            return;
        }

        long index = number_coerce_to_long(_index);
        array_t* array = (array_t*)_obj->value.opaque;
        
        // Check bounds
        if (index < 0 || index >= array_length(array)) {
            PUSH(object_new_error("index out of bounds", true));
            return;
        }
        
        // Get and push array element
        PUSH_REF(array_get(array, index));
        return;
    } 
    
    // Handle ranges
    else if (OBJECT_TYPE_RANGE(_obj)) {
        // Validate index is a number
        if (!OBJECT_TYPE_NUMBER(_index)) {
            char* message = string_format(
                "expected number, got \"%s\"", 
                object_to_string(_index)
            );
            PUSH(object_new_error(message, true));
            free(message);
            return;
        }

        long index = number_coerce_to_long(_index);
        range_t* range = (range_t*)_obj->value.opaque;
        
        // Check bounds
        if (index < 0 || index >= range_length(range)) {
            PUSH(object_new_error("index out of bounds", true));
            return;
        }
        
        // Get and push range element
        PUSH_REF(range_get(range, index));
        return;
    } 
    
    // Handle objects (maps)
    else if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_t* map = (hashmap_t*)_obj->value.opaque;
        
        // Check if key exists
        if (!hashmap_has(map, _index)) {
            PUSH(object_new_error("key not found", true));
            return;
        }
        
        // Get and push object property
        PUSH_REF(hashmap_get(map, _index));
        return;
    }

    // This should never happen due to the OBJECT_TYPE_COLLECTION check above
    // but keeping as a safeguard
    ERROR:;
    char* message = string_format(
        "expected array or object, got \"%s\"", 
        object_to_string(_obj)
    );
    PUSH(object_new_error(message, true));
    free(message);
}

INTERNAL void do_set_index(object_t* _obj, object_t* _index, object_t* _value) {
    // Check if object is a collection type, but not range (range can't be indexed and set)
    if (!OBJECT_TYPE_COLLECTION(_obj) || OBJECT_TYPE_RANGE(_obj)) {
        goto ERROR;
    }

    // Handle arrays
    if (OBJECT_TYPE_ARRAY(_obj)) {
        array_t* array = (array_t*)_obj->value.opaque;
        array_set(array, number_coerce_to_long(_index), _value);
        return;
    }

    // Handle objects (maps)
    if (OBJECT_TYPE_OBJECT(_obj)) {
        hashmap_t* map = (hashmap_t*)_obj->value.opaque;
        hashmap_put(map, _index, _value);
        return;
    }

    // This should never happen due to the OBJECT_TYPE_COLLECTION check above
    // but keeping as a safeguard
    ERROR:;
    POPP(); // POP the value
    char* message = string_format(
        "expected array or object, got \"%s\"",
        object_to_string(_obj)
    );
    PUSH(object_new_error(message, true));
    free(message);
}

INTERNAL void do_call(env_t* _parent_env, bool _is_method, object_t *_function, int _argc) {
    code_t* code = (code_t*)_function->value.opaque;
    object_t* this = _is_method ? POPP() : NULL;
    
    if (code->param_count != _argc) {
        // Pop all arguments before returning error
        POPN(_argc);
        char* message = string_format(
            "expected %ld arguments, got %d", 
            code->param_count, 
            _argc
        );
        PUSH(object_new_error(message, true));
        free(message);
        return;
    }
    
    env_t* func_env = env_new(_parent_env);
    func_env->closure = code->environment;
    
    if (this != NULL) {
        env_put(func_env, string_allocate("this"), this);
    }
    
    vm_block_signal_t signal = vm_execute(func_env, 0, code);
    func_env->closure = NULL;
    
    if (signal != VmBlockSignalPending) {
        env_free(func_env);
    }
}

INTERNAL void do_native_call(object_t* _function, int _argc) {
    vm_native_function function = (vm_native_function)_function->value.opaque;
    function(_argc);
}

INTERNAL void vm_invoke_property(env_t* _parent_env, object_t* _obj, char* _method_name, int _argc) {
    object_t* method = NULL;
    bool is_method_call = !OBJECT_TYPE_USER_TYPE(_obj);
    
    // Find the method in the object or its prototype chain
    if (OBJECT_TYPE_USER_TYPE(_obj) || OBJECT_TYPE_USER_TYPE_INSTANCE(_obj)) {
        // Start with the constructor for instances, or the type itself
        object_t* current = OBJECT_TYPE_USER_TYPE(_obj) 
            ? _obj 
            : ((user_type_instance_t*)_obj->value.opaque)->constructor;
        
        // Walk up the prototype chain
        while (current != NULL && OBJECT_TYPE_USER_TYPE(current)) {
            user_type_t* user_type = (user_type_t*)current->value.opaque;
            hashmap_t* prototype_map = (hashmap_t*)user_type->prototype->value.opaque;
            
            if (hashmap_has_string(prototype_map, _method_name)) {
                method = hashmap_get_string(prototype_map, _method_name);
                break;
            }
            current = user_type->super;
        }
    } else if (OBJECT_TYPE_OBJECT(_obj)) {
        // Direct lookup for regular objects
        hashmap_t* obj_map = (hashmap_t*)_obj->value.opaque;
        if (hashmap_has_string(obj_map, _method_name)) {
            method = hashmap_get_string(obj_map, _method_name);
        }
    }

    // For method calls, push the object as 'this'
    if (is_method_call) PUSH_REF(_obj);

    // Handle error cases
    if (method == NULL) {
        // Method not found
        POPN(_argc);
        if (is_method_call) POPP();
        
        char* message = string_format(
            "method \"%s\" not found in \"%s\"", 
            _method_name,
            object_to_string(_obj)
        );
        PUSH(object_new_error(message, true));
        free(message);
        return;
    }

    if (!OBJECT_TYPE_CALLABLE(method)) {
        // Method is not callable
        POPN(_argc);
        if (is_method_call) POPP();
        
        char* message = string_format(
            "method \"%s\" is not callable", 
            _method_name
        );
        PUSH(object_new_error(message, true));
        free(message);
        return;
    }

    // Invoke the method
    if (OBJECT_TYPE_FUNCTION(method)) {
        do_call(_parent_env, is_method_call, method, _argc);
    } else {
        do_native_call(method, _argc);
    }
}

INTERNAL void do_new_constructor_call(env_t* _parent_env, object_t* _constructor, int _argc) {
    char* constructor_name = "init";
    user_type_t* user_type = (user_type_t*)_constructor->value.opaque;
    hashmap_t* prototype_map;
    
    // Create a new instance with empty object
    object_t* new_instance = vm_to_heap(object_new_user_type_instance(
        _constructor, 
        vm_to_heap(object_new_object())
    ));
    
    // Find constructor in prototype chain
    while (user_type != NULL) {
        prototype_map = (hashmap_t*)user_type->prototype->value.opaque;
        
        if (hashmap_has_string(prototype_map, constructor_name)) {
            // Found constructor - check if it's callable
            object_t* constructor_fn = hashmap_get_string(prototype_map, constructor_name);
            
            if (OBJECT_TYPE_CALLABLE(constructor_fn)) {
                // Call the constructor with the new instance
                vm_invoke_property(_parent_env, new_instance, constructor_name, _argc);
                // Discard constructor's return value
                POPP();
                PUSH_REF(new_instance);
                return;
            } else {
                // Constructor exists but isn't callable
                POPN(_argc);
                char* message = string_format(
                    "constructor \"%s\" is not callable", 
                    constructor_name
                );
                PUSH(object_new_error(message, true));
                free(message);
                return;
            }
        }
        
        // Move up the inheritance chain
        user_type = user_type->super ? (user_type_t*)user_type->super->value.opaque : NULL;
    }
    
    // No constructor found - use default instance
    POPN(_argc);
    PUSH_REF(new_instance);
}

INTERNAL void do_panic(int _argc) {
    char* message = string_allocate("");
    for (int i = 0; i < _argc; i++) {
        object_t* arg = POPP();
        message = string_append(message, object_to_string(arg));
        if (i < _argc - 1) {
            message = string_append(message, "\n");
        }
    }
    fprintf(stderr, "panic: %s\n", message);
    free(message);
    vm_load_null();
    gc_collect_all(instance);
    exit(EXIT_FAILURE);
}

INTERNAL void do_resolve(vm_block_signal_t _signal) {
    object_t* return_value = POPP();
    if (!OBJECT_TYPE_PROMISE(return_value)) {
        PUSH_REF(return_value);
        return;
    }
    async_promise_t* promise = (async_promise_t*) return_value->value.opaque;
    if (_signal == VmBlockSignalReturned && promise->state == ASYNC_STATE_RESOLVED) {
        PUSH_REF(promise->value);
    } else {
        // Push back
        PUSH_REF(return_value);
    }
}

INTERNAL vm_block_signal_t vm_execute(env_t* _env, size_t _ip, code_t* _code) {
    ASSERTNULL(instance, "VM is not initialized");

    char* file_path = _code->file_name;
    char* exec_name = _code->block_name;

    size_t ip = _ip;

    // Shallow copy the bytecode
    uint8_t* bytecode = _code->bytecode;

    while (ip < _code->size) {
        opcode_t opcode = bytecode[ip++];

        if (instance->allocation_counter % GC_ALLOCATION_THRESHOLD == 0) {
            gc_collect(instance, _env);
        }

        // Check if opcode is valid
        switch (opcode) {
            case OPCODE_LOAD_NAME: {
                char* name = get_string(bytecode, ip);
                instance->name_resolver(
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
                    PUSH_REF(instance->tobj);
                } else {
                    PUSH_REF(instance->fobj);
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
                PUSH_REF(instance->null);
                break;
            }
            case OPCODE_LOAD_THIS: {
                if (!env_has(_env, "this", true)) {
                    PUSH(object_new_error("this is not defined", true));
                    break;
                }
                PUSH_REF(env_get(_env, "this"));
                break;
            }
            case OPCODE_LOAD_SUPER: {
                if (!env_has(_env, "this", true)) {
                    PUSH(object_new_error("super is not defined", true));
                    break;
                }
                object_t* this = env_get(_env, "this");
                if (!OBJECT_TYPE_USER_TYPE_INSTANCE(this)) {
                    char* message = string_format(
                        "expected \"user type instance\", got \"%s\"", 
                        object_type_to_string(this)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                object_t* constructor = 
                    ((user_type_instance_t*)this->value.opaque)->constructor;
                /************/
                object_t* super = 
                    ((user_type_t*)constructor->value.opaque)->super;
                /************/
                if (super == NULL) {
                    char* message = string_format(
                        "super is not defined for \"%s\"", 
                        object_to_string(this)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                PUSH_REF(super);
                break;
            }
            case OPCODE_LOAD_ARRAY: {
                int length = get_int(bytecode, ip);
                object_t* array = object_new_array(length);
                for (int i = 0; i < length; i++) {
                    array_set((array_t*) array->value.opaque, i, POPP());
                }
                PUSH(array);
                FORWARD(4);
                break;
            }
            case OPCODE_EXTEND_ARRAY: {
                object_t* array_src = POPP();
                object_t* array_dst = PEEK();
                if (!OBJECT_TYPE_ARRAY(array_src) && !OBJECT_TYPE_RANGE(array_src)) {
                    POPP();
                    char* message = string_format(
                        "expected \"array\", got \"%s\"", 
                        object_to_string(array_src)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                if (!OBJECT_TYPE_ARRAY(array_dst)) {
                    POPP();
                    char* message = string_format(
                        "expected \"array\", got \"%s\"", 
                        object_to_string(array_dst)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                array_t* src_array = (OBJECT_TYPE_ARRAY(array_src)) 
                    ? (array_t*) array_src->value.opaque 
                    : (array_t*) (range_to_array((range_t*) array_src->value.opaque))->value.opaque;
                /************/
                array_extend((array_t*) array_dst->value.opaque, src_array);
                break;
            }
            case OPCODE_APPEND_ARRAY: {
                object_t* obj = POPP();
                object_t* arr = PEEK();
                if (!OBJECT_TYPE_ARRAY(arr)) {
                    POPP();
                    char* message = string_format(
                        "expected \"array\", got \"%s\"", 
                        object_to_string(arr)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                array_push((array_t*)arr->value.opaque, obj);
                break;
            }
            case OPCODE_LOAD_OBJECT: {
                int length = get_int(bytecode, ip);
                object_t* obj = object_new_object();
                for (int i = 0; i < length; i++) {
                    object_t* key = POPP();
                    if (OBJECT_TYPE_COLLECTION(key)) {
                        char* message = string_format(
                            "invalid key type %s", 
                            object_to_string(key)
                        );
                        PUSH(object_new_error(message, true));
                        free(message);
                        break;
                    }
                    object_t* val = POPP();
                    hashmap_put((hashmap_t*)obj->value.opaque, key, val);
                }
                PUSH(obj);
                FORWARD(4);
                break;
            }
            case OPCODE_EXTEND_OBJECT: {
                object_t* obj_src = POPP();
                object_t* obj_dst = PEEK();
                if (!OBJECT_TYPE_OBJECT(obj_src)) {
                    POPP();
                    char* message = string_format(
                        "expected \"object\", got \"%s\"", 
                        object_to_string(obj_src)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                if (!OBJECT_TYPE_OBJECT(obj_dst)) {
                    POPP();
                    char* message = string_format(
                        "expected \"object\", got \"%s\"", 
                        object_to_string(obj_dst)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                hashmap_extend((hashmap_t*)obj_dst->value.opaque, (hashmap_t*)obj_src->value.opaque);
                break;
            }
            case OPCODE_PUT_OBJECT: {
                object_t* key = POPP();
                object_t* val = POPP();
                object_t* obj_dst = PEEK();
                if (OBJECT_TYPE_COLLECTION(key)) {
                    char* message = string_format(
                        "invalid key type %s", 
                        object_to_string(key)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                if (!OBJECT_TYPE_OBJECT(obj_dst)) {
                    POPP();
                    char* message = string_format(
                        "expected \"object\", got \"%s\"", 
                        object_to_string(obj_dst)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                hashmap_put((hashmap_t*)obj_dst->value.opaque, key, val);
                break;
            }
            case OPCODE_RANGE: {
                object_t* lhs = POPP();
                if (!OBJECT_TYPE_NUMBER(lhs)) {
                    char* message = string_format(
                        "expected \"number\", got \"%s\"", 
                        object_type_to_string(lhs)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                object_t* rhs = POPP();
                if (!OBJECT_TYPE_NUMBER(rhs)) {
                    char* message = string_format(
                        "expected \"number\", got \"%s\"", 
                        object_type_to_string(rhs)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    break;
                }
                long start = number_coerce_to_long(lhs);
                long ended = number_coerce_to_long(rhs);
                long step  = (start < ended) ? 1 : -1;
                PUSH(object_new_range(
                    start, 
                    ended, 
                    step
                ));
                break;
            }
            case OPCODE_GET_PROPERTY: {
                char* name = get_string(bytecode, ip);
                object_t* obj = POPP();
                object_t* property = get_property(obj, name);
                if (property == NULL) {
                    char* message = string_format(
                        "property \"%s\" not found in \"%s\"", 
                        name,
                        object_to_string(obj)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    FORWARD(strlen(name) + 1);
                    free(name);
                    break;
                }
                PUSH_REF(property);
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_INDEX: {
                object_t* idx = POPP();
                object_t* obj = POPP();
                do_index(obj, idx);
                break;
            }
            case OPCODE_SET_INDEX: {
                object_t* idx = POPP();
                object_t* obj = POPP();
                do_set_index(obj, idx, PEEK());
                break;
            }
            case OPCODE_CALL_CONSTRUCTOR: {
                int argc = get_int(bytecode, ip);
                object_t* constructor = POPP();
                if (!OBJECT_TYPE_USER_TYPE(constructor)) {
                    for (int i = 0; i < argc; i++) POPP();
                    char* message = string_format(
                        "expected \"constructor\", got \"%s\"", 
                        object_type_to_string(constructor)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    FORWARD(4);
                    break;
                }
                do_new_constructor_call(_env, constructor, argc);
                FORWARD(4);
                break;
            }
            case OPCODE_CALL_METHOD: {
                char* method_name = get_string(bytecode, ip);
                FORWARD(strlen(method_name) + 1);
                int argc = get_int(bytecode, ip);
                object_t* obj = POPP();
                vm_invoke_property(_env, obj, method_name, argc);
                FORWARD(4);
                free(method_name);
                break;
            }
            case OPCODE_CALL: {
                int argc = get_int(bytecode, ip);
                object_t* function = POPP();
                if (!OBJECT_TYPE_CALLABLE(function)) {
                    for (int i = 0; i < argc; i++) POPP();
                    char* message = string_format(
                        "expected \"function\", got \"%s\"", 
                        object_type_to_string(function)
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    FORWARD(4);
                    break;
                }
                if (OBJECT_TYPE_FUNCTION(function)) {
                    do_call(_env, false, function, argc);
                } else {
                    do_native_call(function, argc);
                }
                FORWARD(4);
                break;
            }
            case OPCODE_STORE_NAME: {
                char* name = get_string(bytecode, ip);
                env_put(_env, name, POPP());
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_STORE_CLASS: {
                char* name = get_string(bytecode, ip);
                object_t* obj = POPP();
                object_t* user = object_new_user_type(name, NULL, obj);
                env_put(_env, name, user);
                PUSH_REF(user);
                FORWARD(strlen(name) + 1);
                break;
            }
            case OPCODE_SET_NAME: {
                char* name = get_string(bytecode, ip);
                if (!env_has(_env, name, true)) {
                    char* message = string_format(
                        "variable \"%s\" not found", 
                        name
                    );
                    PUSH(object_new_error(message, true));
                    free(message);
                    FORWARD(strlen(name) + 1);
                    break;
                }
                env_t* env = _env;
                while (env != NULL) {
                    if (env_has(env, name, false)) {
                        env_put(env, name, PEEK());
                        break;
                    }
                    env = env_parent(env);
                }
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_SET_PROPERTY: {
                char* name = get_string(bytecode, ip);
                object_t* obj = POPP();
                set_property(obj, name, PEEK());
                FORWARD(strlen(name) + 1);
                free(name);
                break;
            }
            case OPCODE_INCREMENT: {
                bool is_postfix = bytecode[ip];
                object_t *obj = POPP();
                do_increment(is_postfix, obj);
                FORWARD(1);
                break;
            }
            case OPCODE_DECREMENT: {
                bool is_postfix = bytecode[ip];
                object_t *obj = POPP();
                do_decrement(is_postfix, obj);
                FORWARD(1);
                break;
            }
            case OPCODE_UNARY_PLUS: {
                object_t *obj = POPP();
                do_unary_plus(obj);
                break;
            }
            case OPCODE_UNARY_MINUS: {
                object_t *obj = POPP();
                do_unary_minus(obj);
                break;
            }
            case OPCODE_NOT: {
                object_t *obj = POPP();
                do_not(obj);
                break;
            }
            case OPCODE_BITWISE_NOT: {
                object_t *obj = POPP();
                do_bitwise_not(obj);
                break;
            }
            case OPCODE_MUL: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_mul(obj1, obj2);
                break;
            }
            case OPCODE_DIV: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_div(obj1, obj2);
                break;
            }
            case OPCODE_MOD: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_mod(obj1, obj2);
                break;
            }
            case OPCODE_ADD: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_add(obj1, obj2);
                break;
            }
            case OPCODE_SUB: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_sub(obj1, obj2);
                break;
            }
            case OPCODE_SHL: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_shl(obj1, obj2);
                break;
            }
            case OPCODE_SHR: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_shr(obj1, obj2);
                break;
            }
            case OPCODE_CMP_LT: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_lt(obj1, obj2);
                break;
            }
            case OPCODE_CMP_LTE: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_lte(obj1, obj2);
                break;
            }
            case OPCODE_CMP_GT: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_gt(obj1, obj2);
                break;
            }
            case OPCODE_CMP_GTE: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_gte(obj1, obj2);
                break;
            }
            case OPCODE_CMP_EQ: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_eq(obj1, obj2);
                break;
            }
            case OPCODE_CMP_NE: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_cmp_ne(obj1, obj2);
                break;
            }
            case OPCODE_AND: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_and(obj1, obj2);
                break;
            }
            case OPCODE_OR: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_or(obj1, obj2);
                break;
            }
            case OPCODE_XOR: {
                object_t *obj2 = POPP();
                object_t *obj1 = POPP();
                do_xor(obj1, obj2);
                break;
            }
            case OPCODE_POP_JUMP_IF_FALSE: {
                int jump_offset = get_int(bytecode, ip);
                object_t *obj = POPP();
                if (!object_is_truthy(obj)) {
                    JUMP(jump_offset);
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_POP_JUMP_IF_TRUE: {
                int jump_offset = get_int(bytecode, ip);
                object_t *obj = POPP();
                if (object_is_truthy(obj)) {
                    JUMP(jump_offset);
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_JUMP_IF_FALSE_OR_POP: {
                int jump_offset = get_int(bytecode, ip);
                object_t *obj = PEEK();
                if (!object_is_truthy(obj)) {
                    JUMP(jump_offset);
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
                    JUMP(jump_offset);
                } else {
                    POPP();
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_JUMP_IF_NOT_ERROR: {
                int jump_offset = get_int(bytecode, ip);
                object_t* obj = PEEK();
                if (!object_is_error(obj)) {
                    JUMP(jump_offset);
                } else {
                    FORWARD(4);
                }
                break;
            }
            case OPCODE_ABSOLUTE_JUMP: {
                JUMP(get_int(bytecode, ip));
                break;
            }
            case OPCODE_JUMP_FORWARD: {
                JUMP(get_int(bytecode, ip));
                break;
            }
            case OPCODE_GET_ITERATOR_OR_JUMP: {
                int jump_offset = get_int(bytecode, ip);
                object_t* obj = POPP();
                if (!OBJECT_TYPE_COLLECTION(obj)) {
                    JUMP(jump_offset);
                    break;
                }
                PUSH(object_new_iterator(obj));
                FORWARD(4);
                break;
            }
            case OPCODE_HAS_NEXT: {
                int jump_offset = get_int(bytecode, ip);
                object_t* obj = PEEK();
                if (!iterator_has_next(obj)) {
                    JUMP(jump_offset);
                    break;
                }
                FORWARD(4);
                break;
            }
            case OPCODE_GET_NEXT_VALUE: 
            case OPCODE_GET_NEXT_KEY_VALUE: {
                object_t* obj = PEEK();
                object_t** values = iterator_next(obj);
                if (opcode == OPCODE_GET_NEXT_KEY_VALUE) {
                    if (values[1] != NULL) PUSH_REF(values[1]) // value
                    else PUSH_REF(instance->null);
                }
                PUSH_REF(values[0]); // key
                break;
            }
            case OPCODE_POPTOP: {
                POPP();
                break;
            }
            case OPCODE_AWAIT: {
                object_t* awaited = PEEK();
                
                if (!OBJECT_TYPE_PROMISE(awaited)) {
                    continue;
                }

                async_promise_t* promise = (async_promise_t*) awaited->value.opaque;

                size_t awaited_index;
                
                if (OBJECT_TYPE_PROMISE(awaited) && promise->state == ASYNC_STATE_RESOLVED) {
                    POPP(); // Pop promise and push value
                    PUSH_REF(promise->value);
                    awaited_index = instance->sp;
                } else {
                    awaited_index = instance->sp - 1; // minus 1 to point to the actual return;
                }
                
                object_t* obj = object_new_promise(ASYNC_STATE_PENDING, NULL);
                PUSH(obj);
                
                async_t* async = async_new(ip, awaited_index, _env, _code, obj);
                vm_enqueue(async);
                
                return VmBlockSignalPending;
            }
            case OPCODE_RETURN: {
                return VmBlockSignalReturned;
            }
            case OPCODE_RETURN_ASYNC: {
                object_t* value = object_new_promise(ASYNC_STATE_RESOLVED, POPP());
                PUSH(value);
                return VmBlockSignalReturned;
            }
            case OPCODE_COMPLETE_BLOCK: {
                return VmBlockSignalComplete;
            }
            case OPCODE_SETUP_CLASS:
            case OPCODE_BEGIN_CLASS: {
                if (opcode == OPCODE_SETUP_CLASS)
                if (OPCODE != OPCODE_BEGIN_CLASS) PD("incorrect bytecode format");
                FORWARD(1);
                code_t* class_bytecode = 
                    (code_t*)get_memory(bytecode, ip);
                /************/
                SAVE_FUNCTION(class_bytecode); // Slow!, optimize later
                object_t* closure = vm_to_heap(object_new_function(class_bytecode));
                vm_block_signal_t signal = VmBlockSignalPending;
                do_block(_env, closure, &signal);   
                FORWARD(8);
                break;
            }
            case OPCODE_EXTEND_CLASS: {
                object_t* super = POPP();
                if (!OBJECT_TYPE_USER_TYPE(super)) {
                    break;
                }
                object_t* class = PEEK();
                user_type_t* user = 
                    (user_type_t*)class->value.opaque;
                /************/
                user->super = super;
                break;
            }
            case OPCODE_SETUP_FUNCTION:
            case OPCODE_BEGIN_FUNCTION: {
                if (opcode == OPCODE_SETUP_FUNCTION)
                if (OPCODE != OPCODE_BEGIN_FUNCTION) PD("incorrect bytecode format");
                FORWARD(1);
                code_t* function_bytecode = 
                    (code_t*)get_memory(bytecode, ip);
                /************/
                SAVE_FUNCTION(function_bytecode); // Slow!, optimize later
                PUSH(object_new_function(function_bytecode));
                FORWARD(8);
                break;
            }
            case OPCODE_SETUP_BLOCK:
            case OPCODE_BEGIN_BLOCK: {
                if (opcode == OPCODE_SETUP_BLOCK)
                if (OPCODE != OPCODE_BEGIN_BLOCK) PD("incorrect bytecode format");
                FORWARD(1);
                code_t* block_bytecode = (code_t*)get_memory(bytecode, ip);
                SAVE_FUNCTION(block_bytecode); // Slow!, optimize later
                object_t* closure = vm_to_heap(object_new_function(block_bytecode));
                vm_block_signal_t signal = VmBlockSignalPending;
                do_block(_env, closure, &signal);   
                FORWARD(8);
                if (signal == VmBlockSignalReturned) return VmBlockSignalReturned;
                if (signal == VmBlockSignalComplete) break;
                PD("invalid signal state (%d)", signal);
            }
            case OPCODE_SETUP_CATCH_BLOCK: {
                code_t* block_bytecode = (code_t*)get_memory(bytecode, ip);
                SAVE_FUNCTION(block_bytecode); // Slow!, optimize later
                object_t* closure = vm_to_heap(object_new_function(block_bytecode));
                vm_block_signal_t signal = VmBlockSignalPending;
                do_block(_env, closure, &signal);
                FORWARD(8);
                break;
            }
            case OPCODE_DUPTOP: {
                PUSH_REF(PEEK());
                break;
            }
            case OPCODE_ROT2: {
                // A B -> B A
                rotate2();
                break;
            }
            case OPCODE_ROT3: {
                // A B C -> C A B
                rotate3();
                break;
            }
            case OPCODE_ROT4: {
                // A B C D -> D A B C
                rotate4();
                break;
            }
            case OPCODE_SAVE_CAPTURES: {
                object_t* obj = PEEK();
                code_t* code = 
                    (code_t*)obj->value.opaque;
                /************/
                int capture_count = get_int(bytecode, ip);
                int length = 4;
                for (int i = 0; i < capture_count; i++) {
                    char* name = get_string(bytecode, ip+length);
                    // Store only if it's not in the environment
                    if (!env_has(_env, name, true)) {
                        // Handled dynamically by the name resolver
                        length += strlen(name) + 1;
                        free(name);
                        continue;
                    }
                    env_put(code->environment, name, env_get(_env, name));
                    length += strlen(name) + 1;
                    free(name);
                }
                FORWARD(length);
                break;
            }
            default: {
                decompile(_code, false);
                PD("unknown opcode 0x%02X at %02zu", opcode, ip-1);
            }
        }
    }
    return VmBlockSignalReturned;
}

// -----------------------------

DLLEXPORT void vm_init() {
    if (instance != NULL) {
        PD("VM already initialized");
    }
    instance = (vm_t *) malloc(sizeof(vm_t));
    ASSERTNULL(instance, "failed to allocate memory for vm");
    // evaluation stack
    instance->evaluation_stack = 
        (object_t **)malloc(sizeof(object_t*) * EVALUATION_STACK_SIZE);
    ASSERTNULL(instance->evaluation_stack, "failed to allocate memory for evaluation stack");
    instance->queque = (async_t**)malloc(sizeof(async_t*) * ASYNC_QUEUE_SIZE);
    ASSERTNULL(instance->queque, "failed to allocate memory for async queue");
    instance->sp = 0;
    instance->aq = 0;
    // function table
    instance->function_table_size = 0;
    instance->function_table_item = (code_t**)malloc(sizeof(code_t*));
    instance->function_table_item[0] = NULL;
    // counter
    instance->allocation_counter = 0;
    // name resolver
    instance->name_resolver = vm_name_resolver;
    // root object
    instance->root = object_new_object();
    instance->tail = instance->root;
    // singleton null
    instance->null = object_new(OBJECT_TYPE_NULL);
    // singleton boolean
    instance->tobj = object_new_bool(true);
    instance->fobj = object_new_bool(false);
    // env globals
    instance->env = env_new(NULL);
    // define panic function
    object_t* panic = object_new_native_function(1, do_panic);
    vm_define_global("panic", panic);
}

DLLEXPORT void vm_set_name_resolver(vm_name_resolver_t _resolver) {
    instance->name_resolver = _resolver;
}

DLLEXPORT void vm_name_resolver(env_t* _env, char* _name) {
    if (!env_has(_env, _name, true)) {
        char* message = string_format("variable \"%s\" not found", _name);
        PUSH(object_new_error(message, true));
        free(message);
        return;
    }
    PUSH_REF(env_get(_env, _name));
}

DLLEXPORT object_t* vm_to_heap(object_t* _obj) {
    if (_obj->next != NULL) {
        PD("Object is already in the root (%s)", object_to_string(_obj));
    }

    instance->allocation_counter++;
    _obj->next = instance->root;
    instance->root = _obj;

    return _obj;
}

DLLEXPORT void vm_push(object_t* _obj) {
    ASSERTNULL(instance->evaluation_stack, "Evaluation stack is not initialized");
    if (instance->sp >= EVALUATION_STACK_SIZE) {
        PD("Stackoverflow: TOP(%s)", object_to_string(PEEK()));
    } 
    if (_obj->next != NULL) {
        PD("Object is already in the root (%s)", object_to_string(_obj));
    }
    instance->allocation_counter++;
    instance->evaluation_stack[instance->sp++] = _obj;
    _obj->next = instance->root;
    instance->root = _obj;
}

DLLEXPORT object_t* vm_pop() {
    return POPP();
}

DLLEXPORT object_t* vm_peek() {
    return PEEK();
}

DLLEXPORT void vm_load_null() {
    PUSH_REF(instance->null);
}

DLLEXPORT void vm_load_bool(bool _value) {
    PUSH_REF((_value ? instance->tobj : instance->fobj));
}

DLLEXPORT void vm_define_global(char* _name, object_t* _value) {
    env_put(instance->env, _name, vm_to_heap(_value));
}

DLLEXPORT void vm_run_main(code_t* _bytecode) {
    ASSERTNULL(instance, "VM is not initialized");
    
    // Create a new environment for the main function
    env_t* env = env_new(instance->env);
    env->closure = _bytecode->environment;
    vm_execute(env, 0, _bytecode);
    env->closure = NULL;

    // Process async queue
    while (instance->aq > 0) {
        async_t* async = vm_dequeue();
        instance->sp = async->top;
        vm_block_signal_t signal = vm_execute(async->env, async->ip, async->code);
        do_resolve(signal);
        if (signal == VmBlockSignalReturned) {
            async_free(async);
        }
    }
    
    // Evaluation stack must contain exactly 1 object
    if (instance->sp != 1) {
        DUMP_STACK();
        PD("evaluation stack must contain 1 object, got %zu", instance->sp);
    }
    
    POPP();
    gc_collect_all(instance);
}   