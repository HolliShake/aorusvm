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
    OPCODE_SET_INDEX                    = 95,   // No following bytes
    OPCODE_CALL_CONSTRUCTOR             = 96,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL                         = 97,   // Followed by 4 bytes (aka the number of arguments)
    OPCODE_CALL_METHOD                  = 98,   // Followed by N bytes (aka the method name) + 1 byte (for the null terminator) + 4 bytes (aka the number of arguments)
    OPCODE_INCREMENT                    = 99,   // No following bytes
    OPCODE_DECREMENT                    = 100,   // No following bytes
    OPCODE_UNARY_PLUS                   = 101,  // No following bytes
    OPCODE_UNARY_MINUS                  = 102,  // No following bytes
    OPCODE_NOT                          = 103,  // No following bytes
    OPCODE_BITWISE_NOT                  = 104,  // No following bytes
    OPCODE_MUL                          = 105,  // No following bytes
    OPCODE_DIV                          = 106,  // No following bytes
    OPCODE_MOD                          = 107,  // No following bytes
    OPCODE_ADD                          = 108,  // No following bytes
    OPCODE_SUB                          = 109,  // No following bytes
    OPCODE_SHL                          = 110,  // No following bytes
    OPCODE_SHR                          = 111,  // No following bytes
    OPCODE_CMP_LT                       = 112,  // No following bytes
    OPCODE_CMP_LTE                      = 113,  // No following bytes
    OPCODE_CMP_GT                       = 114,  // No following bytes
    OPCODE_CMP_GTE                      = 115,  // No following bytes
    OPCODE_CMP_EQ                       = 116,  // No following bytes
    OPCODE_CMP_NE                       = 117,  // No following bytes
    OPCODE_AND                          = 118,  // No following bytes
    OPCODE_OR                           = 119,  // No following bytes
    OPCODE_XOR                          = 120,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE            = 121,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE             = 122,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP         = 123,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP          = 124,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_NOT_ERROR            = 125,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD                 = 126,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP                = 127,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                       = 128,  // No following bytes
    OPCODE_NOP                          = 129,  // No following bytes
    OPCODE_SETUP_CLASS                  = 130,  // No following bytes
    OPCODE_BEGIN_CLASS                  = 131,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_EXTEND_CLASS                 = 132,  // No following bytes
    OPCODE_SETUP_FUNCTION               = 133,  // No following bytes
    OPCODE_BEGIN_FUNCTION               = 134,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_BLOCK                  = 135,  // No following bytes
    OPCODE_BEGIN_BLOCK                  = 136,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_CATCH_BLOCK            = 137,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_RETURN                       = 138,  // No following bytes
    OPCODE_COMPLETE_BLOCK               = 139,  // No following bytes
    OPCODE_DUPTOP                       = 140,  // No following bytes
    OPCODE_ROT2                         = 141,  // No following bytes
    OPCODE_ROT3                         = 142,  // No following bytes
    OPCODE_ROT4                         = 143,  // No following bytes
    OPCODE_SAVE_CAPTURES                = 144,  // Followed by 4 bytes (aka the number of captures) + N bytes separated by null terminators
    OPCODE_GET_ITERATOR_OR_JUMP         = 145,  // Followed by 4 bytes (aka jump offset)
    OPCODE_HAS_NEXT                     = 146,  // Followed by 4 bytes (aka jump offset)
    OPCODE_GET_NEXT_VALUE               = 147,  // No following bytes
    OPCODE_GET_NEXT_KEY_VALUE           = 148,  // No following bytes
    OPCODE_SET_PROPERTY                 = 149,  // Followed by the length of the property in bytes + 1 (for the null terminator)
    OPCODE_AWAIT                        = 150,  // No following bytes
    // NOTE: 255 is the last opcode
} opcode_t;

#endif