/**
 * @file error.h
 * @brief Error handling API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */


#include "../ast/position.h"
#include "global.h"
#include "internal.h"

#ifndef API_CORE_ERROR_H
#define API_CORE_ERROR_H

/*
 * Throw an error.
 * @param fpath The path to the file that threw the error.
 * @param fdata The data of the file that threw the error.
 * @param position The position of the error.
 * @param message The message of the error.
 */
#define __THROW_ERROR(fpath, fdata, position, message, ...) \
    throw_error(__FILE__, __func__, __LINE__, fpath, fdata, string_format(message, ##__VA_ARGS__), position);

/*
 * Throw an error.
 * @param _caller_file The file that called the function.
 * @param _caller_func The function that threw the error.
 * @param _caller_line The line that threw the error.
 * @param _fpath The path to the file that threw the error.
 */
void throw_error(const char* _caller_file, const char* _caller_func, int _caller_line, char* _fpath, char* _fdata, char* _message, position_t* _position);

#endif