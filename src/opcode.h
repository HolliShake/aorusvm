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
    OPCODE_STORE_CLASS                  = 88,   // Followed by the length of the class name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME                     = 89,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_RANGE                        = 90,   // No following bytes
    OPCODE_GET_PROPERTY                 = 91,   // Followed by the length of the property in bytes + 1 (for the null terminator)
    OPCODE_INDEX                        = 92,   // No following bytes
    OPCODE_CALL_CONSTRUCTOR             = 93,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL                         = 94,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL_METHOD                  = 95,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT                    = 96,   // No following bytes (aka the number of arguments)
    OPCODE_MUL                          = 97,   // No following bytes
    OPCODE_DIV                          = 98,   // No following bytes
    OPCODE_MOD                          = 99,   // No following bytes
    OPCODE_ADD                          = 100,   // No following bytes
    OPCODE_SUB                          = 101,   // No following bytes
    OPCODE_SHL                          = 102,  // No following bytes
    OPCODE_SHR                          = 103,  // No following bytes
    OPCODE_CMP_LT                       = 104,  // No following bytes
    OPCODE_CMP_LTE                      = 105,  // No following bytes
    OPCODE_CMP_GT                       = 106,  // No following bytes
    OPCODE_CMP_GTE                      = 107,  // No following bytes
    OPCODE_CMP_EQ                       = 108,  // No following bytes
    OPCODE_CMP_NE                       = 109,  // No following bytes
    OPCODE_AND                          = 110,  // No following bytes
    OPCODE_OR                           = 111,  // No following bytes
    OPCODE_XOR                          = 112,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE            = 113,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE             = 114,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP         = 115,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP          = 116,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_NOT_ERROR            = 117,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD                 = 118,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP                = 119,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                       = 120,  // No following bytes
    OPCODE_NOP                          = 121,  // No following bytes
    OPCODE_SETUP_CLASS                  = 122,  // No following bytes
    OPCODE_BEGIN_CLASS                  = 123,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_FUNCTION               = 124,  // No following bytes
    OPCODE_BEGIN_FUNCTION               = 125,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_BLOCK                  = 126,  // No following bytes
    OPCODE_BEGIN_BLOCK                  = 127,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_CATCH_BLOCK            = 128,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_RETURN                       = 129,  // No following bytes
    OPCODE_COMPLETE_BLOCK               = 130,  // No following bytes
    OPCODE_DUPTOP                       = 131,  // No following bytes
    OPCODE_ROT2                         = 132,  // No following bytes
    OPCODE_SAVE_CAPTURES                = 133,  // Followed by 4 bytes (aka the number of captures) + N bytes separated by null terminators
    OPCODE_GET_ITERATOR_OR_JUMP         = 134,  // Followed by 4 bytes (aka jump offset)
    OPCODE_HAS_NEXT                     = 135,  // Followed by 4 bytes (aka jump offset)
    OPCODE_GET_NEXT_VALUE               = 136,  // No following bytes
    OPCODE_GET_NEXT_KEY_VALUE           = 137,  // No following bytes
    // NOTE: 255 is the last opcode
} opcode_t;

#endif