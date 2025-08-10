/**
 * @file async.h
 * @brief Async API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#ifndef API_CORE_ASYNC_H
#define API_CORE_ASYNC_H

/*
 * Async state enum
 * 
 * @param ASYNC_STATE_PENDING The async is pending.
 * @param ASYNC_STATE_RESOLVED The async is resolved.
 * @param ASYNC_STATE_REJECTED The async is rejected.
 */
typedef enum async_state_enum {
    ASYNC_STATE_PENDING,
    ASYNC_STATE_RESOLVED,
    ASYNC_STATE_REJECTED
} async_state_t;

/*
 * Async struct
 * 
 * @param ip The instruction pointer.
 * @param top The top of the stack.
 * @param env The environment.
 * @param code The code.
 * @param promise The promise.
 */
typedef struct async_struct async_t;

#endif