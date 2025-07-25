#ifndef OPCODE_H
#define OPCODE_H

typedef enum opcode_enum {
    OPCODE_LOAD_NAME                    = 75,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_LOAD_INT                     = 76,   // Followed by 4 bytes
    OPCODE_LOAD_DOUBLE                  = 77,   // Followed by 8 bytes
    OPCODE_LOAD_BOOL                    = 78,   // Followed by 1 byte
    OPCODE_LOAD_STRING                  = 79,   // Followed by the length of the string in bytes + 1 (for the null terminator)
    OPCODE_LOAD_NULL                    = 80,   // No following bytes
    OPCODE_LOAD_THIS                    = 81,   // No following bytes
    OPCODE_LOAD_SUPER                   = 82,   // No following bytes
    OPCODE_LOAD_ARRAY                   = 83,   // Followed by 4 bytes (aka the number of elements)
    OPCODE_EXTEND_ARRAY                 = 84,   // No following bytes
    OPCODE_APPEND_ARRAY                 = 85,   // No following bytes
    OPCODE_LOAD_OBJECT                  = 86,   // Followed by 4 bytes (aka the number of properties)
    OPCODE_EXTEND_OBJECT                = 87,   // No following bytes
    OPCODE_PUT_OBJECT                   = 88,   // No following bytes
    OPCODE_STORE_NAME                   = 89,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_STORE_CLASS                  = 90,   // Followed by the length of the class name in bytes + 1 (for the null terminator)
    OPCODE_SET_NAME                     = 91,   // Followed by the length of the name in bytes + 1 (for the null terminator)
    OPCODE_RANGE                        = 92,   // No following bytes
    OPCODE_GET_PROPERTY                 = 93,   // Followed by the length of the property in bytes + 1 (for the null terminator)
    OPCODE_INDEX                        = 94,   // No following bytes
    OPCODE_CALL_CONSTRUCTOR             = 95,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL                         = 96,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL_METHOD                  = 97,   // Followed by N bytes (aka the method name) + 1 byte (for the null terminator) + 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT                    = 98,   // No following bytes
    OPCODE_DECREMENT                    = 99,   // No following bytes
    OPCODE_UNARY_PLUS                   = 100,  // No following bytes
    OPCODE_UNARY_MINUS                  = 101,  // No following bytes
    OPCODE_NOT                          = 102,  // No following bytes
    OPCODE_BITWISE_NOT                  = 103,  // No following bytes
    OPCODE_MUL                          = 104,  // No following bytes
    OPCODE_DIV                          = 105,  // No following bytes
    OPCODE_MOD                          = 106,  // No following bytes
    OPCODE_ADD                          = 107,  // No following bytes
    OPCODE_SUB                          = 108,  // No following bytes
    OPCODE_SHL                          = 109,  // No following bytes
    OPCODE_SHR                          = 110,  // No following bytes
    OPCODE_CMP_LT                       = 111,  // No following bytes
    OPCODE_CMP_LTE                      = 112,  // No following bytes
    OPCODE_CMP_GT                       = 113,  // No following bytes
    OPCODE_CMP_GTE                      = 114,  // No following bytes
    OPCODE_CMP_EQ                       = 115,  // No following bytes
    OPCODE_CMP_NE                       = 116,  // No following bytes
    OPCODE_AND                          = 117,  // No following bytes
    OPCODE_OR                           = 118,  // No following bytes
    OPCODE_XOR                          = 119,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE            = 120,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE             = 121,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP         = 122,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP          = 123,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_NOT_ERROR            = 124,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD                 = 125,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP                = 126,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                       = 127,  // No following bytes
    OPCODE_NOP                          = 128,  // No following bytes
    OPCODE_SETUP_CLASS                  = 129,  // No following bytes
    OPCODE_BEGIN_CLASS                  = 130,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_EXTEND_CLASS                 = 131,  // No following bytes
    OPCODE_SETUP_FUNCTION               = 132,  // No following bytes
    OPCODE_BEGIN_FUNCTION               = 133,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_BLOCK                  = 134,  // No following bytes
    OPCODE_BEGIN_BLOCK                  = 135,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_CATCH_BLOCK            = 136,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_RETURN                       = 137,  // No following bytes
    OPCODE_COMPLETE_BLOCK               = 138,  // No following bytes
    OPCODE_DUPTOP                       = 139,  // No following bytes
    OPCODE_ROT2                         = 140,  // No following bytes
    OPCODE_ROT3                         = 141,  // No following bytes
    OPCODE_SAVE_CAPTURES                = 142,  // Followed by 4 bytes (aka the number of captures) + N bytes separated by null terminators
    OPCODE_GET_ITERATOR_OR_JUMP         = 143,  // Followed by 4 bytes (aka jump offset)
    OPCODE_HAS_NEXT                     = 144,  // Followed by 4 bytes (aka jump offset)
    OPCODE_GET_NEXT_VALUE               = 145,  // No following bytes
    OPCODE_GET_NEXT_KEY_VALUE           = 146,  // No following bytes
    OPCODE_SET_PROPERTY                 = 147,  // Followed by the length of the property in bytes + 1 (for the null terminator)
    // NOTE: 255 is the last opcode
} opcode_t;

#endif