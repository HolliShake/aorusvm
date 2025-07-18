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
    OPCODE_INCREMENT                    = 98,   // No following bytes (aka the number of arguments)
    OPCODE_MUL                          = 99,   // No following bytes
    OPCODE_DIV                          = 100,  // No following bytes
    OPCODE_MOD                          = 101,  // No following bytes
    OPCODE_ADD                          = 102,  // No following bytes
    OPCODE_SUB                          = 103,  // No following bytes
    OPCODE_SHL                          = 104,  // No following bytes
    OPCODE_SHR                          = 105,  // No following bytes
    OPCODE_CMP_LT                       = 106,  // No following bytes
    OPCODE_CMP_LTE                      = 107,  // No following bytes
    OPCODE_CMP_GT                       = 108,  // No following bytes
    OPCODE_CMP_GTE                      = 109,  // No following bytes
    OPCODE_CMP_EQ                       = 110,  // No following bytes
    OPCODE_CMP_NE                       = 111,  // No following bytes
    OPCODE_AND                          = 112,  // No following bytes
    OPCODE_OR                           = 113,  // No following bytes
    OPCODE_XOR                          = 114,  // No following bytes
    OPCODE_POP_JUMP_IF_FALSE            = 115,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POP_JUMP_IF_TRUE             = 116,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_FALSE_OR_POP         = 117,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_TRUE_OR_POP          = 118,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_IF_NOT_ERROR            = 119,  // Followed by 4 bytes (aka jump offset)
    OPCODE_JUMP_FORWARD                 = 120,  // Followed by 4 bytes (aka jump offset)
    OPCODE_ABSOLUTE_JUMP                = 121,  // Followed by 4 bytes (aka jump offset)
    OPCODE_POPTOP                       = 122,  // No following bytes
    OPCODE_NOP                          = 123,  // No following bytes
    OPCODE_SETUP_CLASS                  = 124,  // No following bytes
    OPCODE_BEGIN_CLASS                  = 125,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_EXTEND_CLASS                 = 126,  // No following bytes
    OPCODE_SETUP_FUNCTION               = 127,  // No following bytes
    OPCODE_BEGIN_FUNCTION               = 128,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_BLOCK                  = 129,  // No following bytes
    OPCODE_BEGIN_BLOCK                  = 130,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_SETUP_CATCH_BLOCK            = 131,  // Followed by 8 bytes (aka code_t pointer/serialized address)
    OPCODE_RETURN                       = 132,  // No following bytes
    OPCODE_COMPLETE_BLOCK               = 133,  // No following bytes
    OPCODE_DUPTOP                       = 134,  // No following bytes
    OPCODE_ROT2                         = 135,  // No following bytes
    OPCODE_SAVE_CAPTURES                = 136,  // Followed by 4 bytes (aka the number of captures) + N bytes separated by null terminators
    OPCODE_GET_ITERATOR_OR_JUMP         = 137,  // Followed by 4 bytes (aka jump offset)
    OPCODE_HAS_NEXT                     = 138,  // Followed by 4 bytes (aka jump offset)
    OPCODE_GET_NEXT_VALUE               = 139,  // No following bytes
    OPCODE_GET_NEXT_KEY_VALUE           = 140,  // No following bytes
    OPCODE_SET_PROPERTY                 = 141,  // Followed by the length of the property in bytes + 1 (for the null terminator)
    // NOTE: 255 is the last opcode
} opcode_t;

#endif