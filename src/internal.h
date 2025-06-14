#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "api/core/global.h"
#include "api/core/object.h"

#ifndef INTERNAL_H
#define INTERNAL_H

#pragma region StringH
char* string_allocate(const char* _str);
char* string_append_char(char* _dst, char _c);
char* string_append(char* _dst, char* _src);
char* string_format(char* _format, ...);
bool string_is_number(char* _str);
#pragma endregion

#pragma region NumberH
int number_coerce_to_int(object_t* _obj);
long number_coerce_to_long(object_t* _obj);
float number_coerce_to_float(object_t* _obj);
double number_coerce_to_double(object_t* _obj);
#pragma endregion

#pragma region PathH
char* path_get_file_name(char* _path);
#pragma endregion

#pragma region HashH
size_t hash64(char* _str);
#pragma endregion

#endif