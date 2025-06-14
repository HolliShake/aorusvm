#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME   = 75, // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT    = 76, // Followed by 4 bytes
    OPCODE_LOAD_LONG   = 77, // Followed by 8 bytes
    OPCODE_LOAD_FLOAT  = 78, // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE = 79, // Followed by 8 bytes
    OPCODE_LOAD_BOOL   = 80, // Followed by 1 byte
    OPCODE_LOAD_STRING = 81, // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL   = 82, // No following bytes
    OPCODE_STORE_NAME  = 83, // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_MUL         = 83, // No following bytes
    OPCODE_DIV         = 84, // No following bytes
    OPCODE_MOD         = 85, // No following bytes
    OPCODE_ADD         = 86, // No following bytes
    OPCODE_SUB         = 87, // No following bytes
    OPCODE_POP_JUMP_IF_FALSE = 88, // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE  = 89, // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD    = 90, // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP      = 91, // No following bytes
    OPCODE_RETURN      = 92  // No following bytes
} opcode_t;

#endif