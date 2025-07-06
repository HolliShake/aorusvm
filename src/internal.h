#include "api/core/global.h"
#include "api/core/internal.h"
#include "api/core/object.h"

#ifndef INTERNAL_H
#define INTERNAL_H

#pragma region StringH
// Handled in api/core/internal.h
#pragma endregion

#pragma region NumberH
/*
 * Coerce an object to an int.
 *
 * @param _obj The object to coerce.
 * @return The int value.
 */
int number_coerce_to_int(object_t* _obj);

/*
 * Coerce an object to a long.
 *
 * @param _obj The object to coerce.
 * @return The long value.
 */
long number_coerce_to_long(object_t* _obj);

/*
 * Coerce an object to a double.
 *
 * @param _obj The object to coerce.
 * @return The double value.
 */
double number_coerce_to_double(object_t* _obj);
#pragma endregion

#pragma region PathH
/*
 * Get the file name of a path.
 *
 * @param _path The path.
 * @return The file name.
 */
char* path_get_file_name(char* _path);
#pragma endregion

#pragma region HashH
/*
 * Hash a string.
 *
 * @param _str The string to hash.
 * @return The hash of the string.
 */
size_t hash64(char* _str);
#pragma endregion

#endif