/**
 * @file node.h
 * @brief The AST node API (Do not modify this file).
 * @author Philipp Andrew Redondo
 * @date 2025-06-13
 * @version 0.1.0
 * @copyright MIT License
 * @note This file is part of the aorusvm project.
 */

#include "../core/global.h"
#include "position.h"

#ifndef API_AST_NODE_H
#define API_AST_NODE_H

/*
 * AST node.
 */
typedef struct ast_node_struct ast_node_t;

/*
 * Name node.
 * 
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The name node.
 */
DLLEXPORT ast_node_t* ast_name_node(position_t* _position, char* _value);

/*
 * Integer node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The integer node.
 */
DLLEXPORT ast_node_t* ast_int_node(position_t* _position, int _value);

/*
 * Long node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The long node.
 */
DLLEXPORT ast_node_t* ast_long_node(position_t* _position, long _value);

/*
 * Float node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The long node.
 */
DLLEXPORT ast_node_t* ast_float_node(position_t* _position, float _value);

/*
 * Double node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The double node.
 */
DLLEXPORT ast_node_t* ast_double_node(position_t* _position, double _value);

/*
 * Boolean node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The boolean node.
 */
DLLEXPORT ast_node_t* ast_boolean_node(position_t* _position, bool _value);

/*
 * String node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The string node.
 */
DLLEXPORT ast_node_t* ast_string_node(position_t* _position, char* _value);

/*
 * Null node.

 * @param _position The position of the node.
 * @return The null node.
 */
DLLEXPORT ast_node_t* ast_null_node(position_t* _position);

/*
 * Binary mul node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary mul node.
 */
DLLEXPORT ast_node_t* ast_binary_mul_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary div node.

 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary div node.
 */
DLLEXPORT ast_node_t* ast_binary_div_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary mod node.

 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary mod node.
 */
DLLEXPORT ast_node_t* ast_binary_mod_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary add node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary add node.
 */
DLLEXPORT ast_node_t* ast_binary_add_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary sub node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary sub node.
 */
DLLEXPORT ast_node_t* ast_binary_sub_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Logical and node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The logical and node.
 */
DLLEXPORT ast_node_t* ast_logical_and_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Logical or node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The logical or node.
 */
DLLEXPORT ast_node_t* ast_logical_or_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Variable statement node (Global variable declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_var_statement(position_t* _position, ast_node_t** _names, ast_node_t** _values);

/*
 * Constant statement node (Global|Local constant declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_const_statement(position_t* _position, ast_node_t** _names, ast_node_t** _values);

/*
 * Local variable statement node (Local variable declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_local_statement(position_t* _position, ast_node_t** _names, ast_node_t** _values);

/*
 * If statement node.
 *
 * @param _position The position of the node.
 * @param _cond The condition of the node.
 * @param _tvalue The true value of the node.
 * @param _fvalue The false value of the node.
 * @return The if statement node.
 */
DLLEXPORT ast_node_t* ast_if_statement(position_t* _position, ast_node_t* _cond, ast_node_t* tvalue, ast_node_t* fvalue);

/*
 * Return node.
 *
 * @param _position The position of the node.
 * @param _expr The expression of the node.
 * @return The return node.
 */
DLLEXPORT ast_node_t* ast_return_node(position_t* _position, ast_node_t* _expr);

/*
 * Statement expression node.
 *
 * @param _position The position of the node.
 * @param _expr The expression of the node.
 * @return The statement expression node.
 */
DLLEXPORT ast_node_t* ast_statement_expression(position_t* _position, ast_node_t* _expr);

/*
 * Program node.
 *
 * @param _position The position of the node.
 * @param _children The children of the node.
 * @return The program node.
 */
DLLEXPORT ast_node_t* ast_program_node(position_t* _position, ast_node_t** _children);

#endif