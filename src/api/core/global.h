/**
 * @file global.h
 * @brief The global API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef API_CORE_GLOBAL_H
#define API_CORE_GLOBAL_H

#if defined(__WIN32__) || defined(_WIN32) || defined(__WIN64__) || defined(_WIN64)
    #define OS_WINDOWS 1
#elif defined(__APPLE__) || defined(__MACH__) || defined(__MACOS__)
    #define OS_MACOS 1
#elif defined(__linux__) || defined(__linux)
    #define OS_LINUX 1
#else
    #error "Unsupported OS"
#endif

#if OS_WINDOWS
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif

#if OS_WINDOWS
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT __attribute__((visibility("default")))
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    #define IS_64BIT 1
#else
    #define IS_64BIT 0
#endif

#define INTERNAL static

// Print and Die
#define PD(message, ...) { \
    fprintf(stderr, "[%s:%d]::%s: ", __FILE__, __LINE__, __func__); \
    fprintf(stderr, message, ##__VA_ARGS__); \
    fprintf(stderr, "\n"); \
    exit(EXIT_FAILURE); \
}

#define ASSERTNULL(ptr, message) { \
    if (ptr == NULL) { \
        PD(message); \
    } \
} \


#endif