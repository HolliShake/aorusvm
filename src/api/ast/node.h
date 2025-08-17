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
 * AST node list.
 */
typedef ast_node_t** ast_node_list_t;

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
 * This node.

 * @param _position The position of the node.
 * @return The this node.
 */
DLLEXPORT ast_node_t* ast_this_node(position_t* _position);

/*
 * Super node.

 * @param _position The position of the node.
 * @return The super node.
 */
DLLEXPORT ast_node_t* ast_super_node(position_t* _position);

/*
 * Array node.
 *
 * @param _position The position of the node.
 * @param _elements The elements of the array.
 * @return The array node.
 */
DLLEXPORT ast_node_t* ast_array_node(position_t* _position, ast_node_list_t _elements);

/*
 * Object property node.

 * @param _position The position of the node.
 * @param _key The key of the property.
 * @param _value The value of the property.
 * @return The object property node.
 */
DLLEXPORT ast_node_t* ast_object_property_node(position_t* _position, ast_node_t* _key, ast_node_t* _value);

/*
 * Object node.
 *
 * @param _position The position of the node.
 * @param _properties The properties of the object.
 * @return The object node.
 */
DLLEXPORT ast_node_t* ast_object_node(position_t* _position, ast_node_list_t _properties);

/*
 * Function expression node.

 * @param _position The position of the node.
 * @param _parameters The parameters of the function.
 * @param _body The body of the function.
 * @return The function expression node.
 */
DLLEXPORT ast_node_t* ast_function_expression_node(position_t* _position, ast_node_list_t _parameters, ast_node_list_t _body);

/*
 * New node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @param _arguments The arguments of the node.
 * @return The new node.
 */
DLLEXPORT ast_node_t* ast_new_node(position_t* _position, ast_node_t* _value, ast_node_list_t _arguments);

/*
 * Member access node.
 *
 * @param _position The position of the node.
 * @param _obj The object of the node.
 * @param _member The member of the node.
 * @return The member access node.
 */
DLLEXPORT ast_node_t* ast_member_access_node(position_t* _position, ast_node_t* _obj, ast_node_t* _member);

/*
 * Index node.
 *
 * @param _position The position of the node.
 * @param _obj The object of the node.
 * @param _index The index of the node.
 * @return The index node.
 */
DLLEXPORT ast_node_t* ast_index_node(position_t* _position, ast_node_t* _obj, ast_node_t* _index);

/*
 * Call node.
 *
 * @param _position The position of the node.
 * @param _function The function of the node.
 * @param _arguments The arguments of the node.
 * @return The call node.
 */
DLLEXPORT ast_node_t* ast_call_node(position_t* _position, ast_node_t* _function, ast_node_list_t _arguments);

/*
 * Postfix plus plus node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The postfix plus plus node.
 */
DLLEXPORT ast_node_t* ast_postfix_plus_plus_node(position_t* _position, ast_node_t* _value);

/*
 * Postfix minus minus node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The postfix minus minus node.
 */
DLLEXPORT ast_node_t* ast_postfix_minus_minus_node(position_t* _position, ast_node_t* _value);

/*
 * Unary plus plus node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary plus plus node.
 */
DLLEXPORT ast_node_t* ast_unary_plus_plus_node(position_t* _position, ast_node_t* _value);

/*
 * Unary minus minus node.

 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary minus minus node.
 */
DLLEXPORT ast_node_t* ast_unary_minus_minus_node(position_t* _position, ast_node_t* _value);

/*
 * Unary plus node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary plus node.
 */
DLLEXPORT ast_node_t* ast_unary_plus_node(position_t* _position, ast_node_t* _value);

/*
 * Unary minus node.

 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary minus node.
 */
DLLEXPORT ast_node_t* ast_unary_minus_node(position_t* _position, ast_node_t* _value);

/*
 * Unary not node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary not node.
 */
DLLEXPORT ast_node_t* ast_unary_not_node(position_t* _position, ast_node_t* _value);

/*
 * Unary bitnot node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary bitnot node.
 */
DLLEXPORT ast_node_t* ast_unary_bitnot_node(position_t* _position, ast_node_t* _value);

/*
 * Unary spread node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The unary spread node.
 */
DLLEXPORT ast_node_t* ast_unary_spread_node(position_t* _position, ast_node_t* _value);

/*
 * Await node.
 *
 * @param _position The position of the node.
 * @param _value The value of the node.
 * @return The await node.
 */
DLLEXPORT ast_node_t* ast_await_node(position_t* _position, ast_node_t* _value);

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
 * Binary shift left node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary shift left node.
 */
DLLEXPORT ast_node_t* ast_binary_shl_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary shift right node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary shift right node.
 */
DLLEXPORT ast_node_t* ast_binary_shr_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare less than node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare less than node.
 */
DLLEXPORT ast_node_t* ast_cmp_lt_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare less than or equal to node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare less than or equal to node.
 */
DLLEXPORT ast_node_t* ast_cmp_lte_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare greater than node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare greater than node.
 */
DLLEXPORT ast_node_t* ast_cmp_gt_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare greater than or equal to node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare greater than or equal to node.
 */
DLLEXPORT ast_node_t* ast_cmp_gte_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare equal to node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare equal to node.
 */
DLLEXPORT ast_node_t* ast_cmp_eq_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Compare not equal to node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The compare not equal to node.
 */
DLLEXPORT ast_node_t* ast_cmp_ne_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary and node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary and node.
 */
DLLEXPORT ast_node_t* ast_binary_and_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary or node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary or node.
 */
DLLEXPORT ast_node_t* ast_binary_or_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Binary xor node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The binary xor node.
 */
DLLEXPORT ast_node_t* ast_binary_xor_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

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
 * Range node.
 *
 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The range node.
 */
DLLEXPORT ast_node_t* ast_range_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Assign node.

 * @param _position The position of the node.
 * @param _lhs The left hand side of the node.
 * @param _rhs The right hand side of the node.
 * @return The assign node.
 */
DLLEXPORT ast_node_t* ast_assign_node(position_t* _position, ast_node_t* _lhs, ast_node_t* _rhs);

/*
 * Ternary node.

 * @param _position The position of the node.
 * @param _condition The condition of the node.
 * @param _tvalue The true value of the node.
 * @param _fvalue The false value of the node.
 * @return The ternary node.
 */
DLLEXPORT ast_node_t* ast_ternary_node(position_t* _position, ast_node_t* _cond, ast_node_t* _tvalue, ast_node_t* _fvalue);

/*
 * Switch expression node.

 * @param _position The position of the node.
 * @param _condition The condition of the node.
 * @param _patterns The patterns of the node.
 * @param _values The values of the node.
 * @param _default The default of the node.
 * @return The switch expression node.
 */
DLLEXPORT ast_node_t* ast_switch_expression_node(position_t* _position, ast_node_t* _condition, ast_node_list_t _patterns, ast_node_list_t _values, ast_node_t* _default);

/*
 * Catch node.

 * @param _position The position of the node.
 * @param _error The error of the node.
 * @param _placeholder The placeholder of the node.
 * @param _body The body of the node.
 * @return The catch node.
 */
DLLEXPORT ast_node_t* ast_catch_node(position_t* _position, ast_node_t* _error, ast_node_t* _placeholder, ast_node_list_t _body);

/*
 * Variable statement node (Global variable declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_var_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values);

/*
 * Constant statement node (Global|Local constant declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_const_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values);

/*
 * Local variable statement node (Local variable declaration).
 *
 * @param _position The position of the node.
 * @return The variable statement node.
 */
DLLEXPORT ast_node_t* ast_local_statement_node(position_t* _position, ast_node_list_t _names, ast_node_list_t _values);

/*
 * If statement node.
 *
 * @param _position The position of the node.
 * @param _cond The condition of the node.
 * @param _tvalue The true value of the node.
 * @param _fvalue The false value of the node.
 * @return The if statement node.
 */
DLLEXPORT ast_node_t* ast_if_statement_node(position_t* _position, ast_node_t* _cond, ast_node_t* tvalue, ast_node_t* fvalue);

/*
 * While statement node.
 *
 * @param _position The position of the node.
 * @param _cond The condition of the node.
 * @param _body The body of the node.
 * @return The while statement node.
 */
DLLEXPORT ast_node_t* ast_while_statement_node(position_t* _position, ast_node_t* _cond, ast_node_t* _body);

/*
 * Do while statement node.
 *
 * @param _position The position of the node.
 * @param _cond The condition of the node.
 * @param _body The body of the node.
 * @return The do while statement node.
 */
DLLEXPORT ast_node_t* ast_do_while_statement_node(position_t* _position, ast_node_t* _cond, ast_node_t* _body);

/*
 * For statement node.
 *
 * @param _position The position of the node.
 * @param _initializer The initializer of the node.
 * @param _iterable The iterable of the node.
 * @param _body The body of the node.
 * @return The for statement node.
 */
DLLEXPORT ast_node_t* ast_for_statement_node(position_t* _position, ast_node_t* _initializer, ast_node_t* _iterable, ast_node_t* _body);

/*
 * For initializer node.
 *
 * @param _position The position of the node.
 * @param _k The k of the node.
 * @param _v The v of the node.
 * @return The for initializer node.
 */
DLLEXPORT ast_node_t* ast_for_multiple_initializer_node(position_t* _position, ast_node_t* _k, ast_node_t* _v);

/*
 * Return statement node.
 *
 * @param _position The position of the node.
 * @param _expr The expression of the node.
 * @return The return node.
 */
DLLEXPORT ast_node_t* ast_return_statement_node(position_t* _position, ast_node_t* _expr);

/*
 * Statement expression node.
 *
 * @param _position The position of the node.
 * @param _expr The expression of the node.
 * @return The statement expression node.
 */
DLLEXPORT ast_node_t* ast_expression_statement_node(position_t* _position, ast_node_t* _expr);

/*
 * Function node.
 *
 * @param _position The position of the node.
 * @param _name The name of the function.
 * @param _parameters The parameters of the function.
 * @param _body The body of the function.
 * @return The function node.
 */
DLLEXPORT ast_node_t* ast_function_node(position_t* _position, ast_node_t* _name, ast_node_list_t _parameters, ast_node_list_t _body);

/*
 * Async function node.
 * @param _position The position of the node.
 * @param _name The name of the function.
 * @param _parameters The parameters of the function.
 * @param _body The body of the function.
 * @return The async function node.
 */
DLLEXPORT ast_node_t* ast_async_function_node(position_t* _position, ast_node_t* _name, ast_node_list_t _parameters, ast_node_list_t _body);

/*
 * Async function node.
 *
 * @param _position The position of the node.
 * @param _name The name of the function.
 * @param _parameters The parameters of the function.
 * @param _body The body of the function.
 * @return The async function node.
 */
DLLEXPORT ast_node_t* ast_async_function_node(position_t* _position, ast_node_t* _name, ast_node_list_t _parameters, ast_node_list_t _body);

/*
 * Class node.
 *
 * @param _position The position of the node.
 * @param _name The name of the class.
 * @param _super The super class of the class.
 * @param _body The body of the class.
 * @return The class node.
 */
DLLEXPORT ast_node_t* ast_class_node(position_t* _position, ast_node_t* _name, ast_node_t* _super, ast_node_list_t _body);

/*
 * Block statement node.
 *
 * @param _position The position of the node.
 * @param _statements The statements of the node.
 * @return The block statement node.
 */
DLLEXPORT ast_node_t* ast_block_statement_node(position_t* _position, ast_node_list_t _statements);

/*
 * Program node.
 *
 * @param _position The position of the node.
 * @param _children The children of the node.
 * @return The program node.
 */
DLLEXPORT ast_node_t* ast_program_node(position_t* _position, ast_node_list_t _children);

/*
 * Get the position of the node.
 *
 * @param _node The node.
 * @return The position of the node.
 */
DLLEXPORT position_t* ast_position(ast_node_t* _node);

#endif