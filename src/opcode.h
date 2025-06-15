#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME             = 75,  // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT              = 76,  // Followed by 4 bytes
    OPCODE_LOAD_LONG             = 77,  // Followed by 8 bytes
    OPCODE_LOAD_FLOAT            = 78,  // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE           = 79,  // Followed by 8 bytes
    OPCODE_LOAD_BOOL             = 80,  // Followed by 1 byte
    OPCODE_LOAD_STRING           = 81,  // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL             = 82,  // No following bytes
    OPCODE_STORE_NAME            = 83,  // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_MUL                   = 84,  // No following bytes
    OPCODE_DIV                   = 85,  // No following bytes
    OPCODE_MOD                   = 86,  // No following bytes
    OPCODE_ADD                   = 87,  // No following bytes
    OPCODE_SUB                   = 88,  // No following bytes
    OPCODE_SHL                   = 89,  // No following bytes
    OPCODE_SHR                   = 90,  // No following bytes
    OPCODE_CMP_LT                = 91,  // No following bytes
    OPCODE_CMP_LTE               = 92,  // No following bytes
    OPCODE_CMP_GT                = 93,  // No following bytes
    OPCODE_CMP_GTE               = 94,  // No following bytes
    OPCODE_CMP_EQ                = 95,  // No following bytes
    OPCODE_CMP_NE                = 96,  // No following bytes
    OPCODE_AND                   = 97,  // No following bytes
    OPCODE_OR                    = 98,  // No following bytes
    OPCODE_XOR                   = 99,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE     = 100, // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE      = 101, // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP  = 102, // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP   = 103, // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD          = 104, // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                = 105, // No following bytes
    OPCODE_NOP                   = 106, // No following bytes
    OPCODE_RETURN                = 107  // No following bytes
} opcode_t;

#endif