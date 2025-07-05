/**
 * @file internal.h
 * @brief Internal API for string, number, and path operations (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */


#include "global.h"

#ifndef API_CORE_INTERNAL_H
#define API_CORE_INTERNAL_H

#pragma region StringH
/*
 * Allocate a string.
 * @param _str The string to allocate.
 * @return The allocated string.
 */
char* string_allocate(const char* _str);

/*
 * Append a character to a string.
 * @param _dst The destination string.
 * @param _c The character to append.
 * @return The destination string.
 */
char* string_append_char(char* _dst, char _c);

/*
 * Append a character to a string.
 * @param _dst The destination string.
 * @param _c The character to append.
 * @return The destination string.
 */
char* string_append(char* _dst, char* _src);

/*
 * Append a string to another string.
 * @param _dst The destination string.
 * @param _src The source string.
 * @return The destination string.
 */
char* string_append_str(char* _dst, char* _src);

/*
 * Format a string.
 * @param _format The format string.
 * @return The formatted string.
 */
char* string_format(char* _format, ...);

/*
 * Check if a string is a number.
 * @param _str The string to check.
 * @return True if the string is a number, false otherwise.
 */
bool string_is_number(char* _str);
#pragma endregion

#endif