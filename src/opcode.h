#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME                    = 75,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT                     = 76,   // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE                  = 77,   // Followed by 8 bytes
    OPCODE_LOAD_BOOL                    = 78,   // Followed by 1 byte
    OPCODE_LOAD_STRING                  = 79,   // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL                    = 80,   // No following bytes
    OPCODE_LOAD_ARRAY                   = 81,   // Followed by 4 bytes (aka the number of elements)
    OPCODE_EXTEND_ARRAY                 = 82,   // No following bytes
    OPCODE_APPEND_ARRAY                 = 83,   // No following bytes
    OPCODE_LOAD_OBJECT                  = 84,   // Followed by 4 bytes (aka the number of properties)
    OPCODE_EXTEND_OBJECT                = 85,   // No following bytes
    OPCODE_PUT_OBJECT                   = 86,   // No following bytes
    OPCODE_STORE_NAME                   = 87,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME                     = 88,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_RANGE                        = 89,   // No following bytes
    OPCODE_INDEX                        = 90,   // No following bytes
    OPCODE_CALL                         = 91,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT                    = 92,   // No following bytes (aka the number of arguments)
    OPCODE_MUL                          = 93,   // No following bytes
    OPCODE_DIV                          = 94,   // No following bytes
    OPCODE_MOD                          = 95,   // No following bytes
    OPCODE_ADD                          = 96,   // No following bytes
    OPCODE_SUB                          = 97,   // No following bytes
    OPCODE_SHL                          = 98,   // No following bytes
    OPCODE_SHR                          = 99,   // No following bytes
    OPCODE_CMP_LT                       = 100,  // No following bytes
    OPCODE_CMP_LTE                      = 101,  // No following bytes
    OPCODE_CMP_GT                       = 102,  // No following bytes
    OPCODE_CMP_GTE                      = 103,  // No following bytes
    OPCODE_CMP_EQ                       = 104,  // No following bytes
    OPCODE_CMP_NE                       = 105,  // No following bytes
    OPCODE_AND                          = 106,  // No following bytes
    OPCODE_OR                           = 107,  // No following bytes
    OPCODE_XOR                          = 108,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE            = 109,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE             = 110,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP         = 111,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP          = 112,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_NOT_ERROR            = 113,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD                 = 114,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP                = 115,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                       = 116,  // No following bytes
    OPCODE_NOP                          = 117,  // No following bytes
    OPCODE_MAKE_FUNCTION                = 118,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_MAKE_ASYNC_FUNCTION          = 119,  // Followed by 4 bytes (aka parameter count) + 8 bytes (aka bytecode size)
    OPCODE_SETUP_BLOCK                  = 120,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_SETUP_CATCH_BLOCK            = 121,  // Followed by 8 bytes (aka bytecode size)
    OPCODE_RETURN                       = 122,  // No following bytes
    OPCODE_COMPLETE_BLOCK               = 123,  // No following bytes
    OPCODE_DUPTOP                       = 124,  // No following bytes
    OPCODE_ROT2                         = 125,  // No following bytes
    OPCODE_SAVE_CAPTURES                = 126,  // Followed by 4 bytes (aka the number of captures) + N bytes separated by null terminators
    OPCODE_GET_ITERATOR_OR_JUMP         = 127,  // Followed by 4 bytes (aka jump offset)
    OPCODE_HAS_NEXT                     = 128,  // Followed by 4 bytes (aka jump offset)
    OPCODE_GET_NEXT_VALUE               = 129,  // No following bytes
    OPCODE_GET_NEXT_KEY_VALUE           = 130,  // No following bytes
    // NOTE: 255 is the last opcode
} opcode_t;

#endif