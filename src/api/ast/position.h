/**
 * @file position.h
 * @brief The position API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include "../core/global.h"

#ifndef API_AST_POSITION_H
#define API_AST_POSITION_H

/*
 * Position.
 */
typedef struct position_struct position_t;

/*
 * Create a new position.
 * @param _line_start The start line.
 * @param _colm_start The start column.
 * @param _line_ended The ended line.
 * @param _colm_ended The ended column.
 * @return The new position.
 */
DLLEXPORT position_t* position_new(int _line_start, int _colm_start, int _line_ended, int _colm_ended);

/*
 * Create a new position from a line and a column.
 * @param _line The line.
 * @param _colm The column.
 * @return The new position.
 */
DLLEXPORT position_t* position_from_line_and_colm(int _line, int _colm);

/*
 * Merge two positions.
 * @param _position0 The first position.
 * @param _position1 The second position.
 * @return The merged position.
 */
DLLEXPORT position_t* position_merge(position_t* _position0, position_t* _position1);

#endif