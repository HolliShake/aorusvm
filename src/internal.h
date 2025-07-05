#include "api/core/global.h"
#include "api/core/internal.h"
#include "api/core/object.h"

#ifndef INTERNAL_H
#define INTERNAL_H

#pragma region StringH
// Handled in api/core/internal.h
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