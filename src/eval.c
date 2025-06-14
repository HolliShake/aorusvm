#include "eval.h"

INTERNAL double eval_coerce_to_double(eval_result_t _result) {
    switch (_result.type) {
        case EvalInt:
            return (double) _result.value.i32;
        case EvalLong:
            return (double) _result.value.i64;
        case EvalFloat:
            return (double) _result.value.f32;
        case EvalDouble:
            return _result.value.f64;
        case EvalString:
            if (!string_is_number((char*) _result.value.ptr)) break;
            return strtod((char*) _result.value.ptr, NULL);
        default:
            PD("cannot coerce %d to double", _result.type);
    }
    return 0.0;
}

INTERNAL long eval_coerce_to_long(eval_result_t _result) {
    switch (_result.type) {
        case EvalInt:
            return (long) _result.value.i32;
        case EvalLong:
            return _result.value.i64;
        case EvalFloat:
            return (long) _result.value.f32;
        case EvalDouble:
            return (long) _result.value.f64;
        case EvalString:
            if (!string_is_number((char*) _result.value.ptr)) break;
            return strtol((char*) _result.value.ptr, NULL, 10);
        default:
            PD("cannot coerce %d to long", _result.type);
    }
    return 0;
}

INTERNAL bool eval_is_number(eval_result_t _result) {
    switch (_result.type) {
        case EvalInt:
            return true;
        case EvalLong:
            return true;
        case EvalFloat:
            return true;
        case EvalDouble:
            return true;
        case EvalString:
            return string_is_number((char*) _result.value.ptr);
        default:
            return false;
    }
}

INTERNAL eval_result_t eval_eval_expression(ast_node_t* _expression) {
    eval_result_t result;
    switch (_expression->type) {
        case AstInt:
            result.type = EvalInt;
            result.value.i32 = _expression->value.i32;
            break;
        case AstLong:
            result.type = EvalLong;
            result.value.i64 = _expression->value.i64;
            break;
        case AstFloat:
            result.type = EvalFloat;
            result.value.f32 = _expression->value.f32;
            break;
        case AstDouble:
            result.type = EvalDouble;
            result.value.f64 = _expression->value.f64;
            break;
        case AstBoolean:
            result.type = EvalBoolean;
            result.value.i32 = _expression->value.i32;
            break;
        case AstString:
            result.type = EvalString;
            result.value.ptr = _expression->str0;
            break;
        case AstNull:
            result.type = EvalNull;
            result.value.ptr = NULL;
            break;
        case AstBinaryMul: {
            eval_result_t l = eval_eval_expression(_expression->ast0);
            eval_result_t r = eval_eval_expression(_expression->ast1);
            if (l.type == EvalError || r.type == EvalError) {
                result.type = EvalError;
                return result;
            }

            if (!eval_is_number(l) || !eval_is_number(r)) {
                result.type = EvalError;
                return result;
            }

            if (l.type == EvalInt && r.type == EvalInt) {
                int64_t product = (int64_t)l.value.i32 * r.value.i32;
                if (product <= INT32_MAX && product >= INT32_MIN) {
                    result.type = EvalInt;
                    result.value.i32 = (int32_t)product;
                } else {
                    result.type = EvalLong;
                    result.value.i64 = product;
                }
                return result;
            }

            double lvalue = eval_coerce_to_double(l);
            double rvalue = eval_coerce_to_double(r);
            double product = lvalue * rvalue;

            double intpart;
            if (modf(product, &intpart) == 0.0 && product <= INT32_MAX && product >= INT32_MIN) {
                result.type = EvalInt;
                result.value.i32 = (int32_t)product;
            } else {
                result.type = EvalDouble;
                result.value.f64 = product;
            }
            return result;
        }
        case AstBinaryDiv: {
            eval_result_t l = eval_eval_expression(_expression->ast0);
            eval_result_t r = eval_eval_expression(_expression->ast1);
            if (l.type == EvalError || r.type == EvalError) {
                result.type = EvalError;
                return result;
            }

            if (!eval_is_number(l) || !eval_is_number(r)) {
                result.type = EvalError;
                return result;
            }

            double rvalue = eval_coerce_to_double(r);
            if (rvalue == 0.0) {
                result.type = EvalError;
                return result;
            }

            double lvalue = eval_coerce_to_double(l);
            double quotient = lvalue / rvalue;

            double intpart;
            if (modf(quotient, &intpart) == 0.0 && quotient <= INT32_MAX && quotient >= INT32_MIN) {
                result.type = EvalInt;
                result.value.i32 = (int32_t)quotient;
            } else {
                result.type = EvalDouble;
                result.value.f64 = quotient;
            }
            return result;
        }
        case AstBinaryMod: {
            eval_result_t l = eval_eval_expression(_expression->ast0);
            eval_result_t r = eval_eval_expression(_expression->ast1);
            if (l.type == EvalError || r.type == EvalError) {
                result.type = EvalError;
                return result;
            }

            if (!eval_is_number(l) || !eval_is_number(r)) {
                result.type = EvalError;
                return result;
            }

            if (l.type == EvalInt && r.type == EvalInt) {
                if (r.value.i32 == 0) {
                    result.type = EvalError;
                    return result;
                }
                result.type = EvalInt;
                result.value.i32 = l.value.i32 % r.value.i32;
                return result;
            }

            double rvalue = eval_coerce_to_double(r);
            if (rvalue == 0.0) {
                result.type = EvalError;
                return result;
            }

            double lvalue = eval_coerce_to_double(l);
            double remainder = fmod(lvalue, rvalue);

            double intpart;
            if (modf(remainder, &intpart) == 0.0 && remainder <= INT32_MAX && remainder >= INT32_MIN) {
                result.type = EvalInt;
                result.value.i32 = (int32_t)remainder;
            } else {
                result.type = EvalDouble;
                result.value.f64 = remainder;
            }
            return result;
        }
        case AstBinaryAdd: {
            eval_result_t l = eval_eval_expression(_expression->ast0);
            eval_result_t r = eval_eval_expression(_expression->ast1);
            if (l.type == EvalError || r.type == EvalError) {
                result.type = EvalError;
                return result;
            }

            // Fast path for most common cases
            if (l.type == EvalInt) {
                if (r.type == EvalInt) {
                    // Check for overflow using unsigned arithmetic
                    int32_t a = l.value.i32;
                    int32_t b = r.value.i32;
                    int32_t sum = a + b;
                    
                    if (((a ^ sum) & (b ^ sum)) < 0) {
                        // Overflow occurred, promote to double
                        result.type = EvalDouble;
                        result.value.f64 = (double)a + (double)b;
                    } else {
                        result.type = EvalInt;
                        result.value.i32 = sum;
                    }
                    return result;
                }
                // Promote int to double for other numeric types
                l.value.f64 = (double)l.value.i32;
                l.type = EvalDouble;
            }

            if (l.type == EvalLong && r.type == EvalLong) {
                int64_t a = l.value.i64;
                int64_t b = r.value.i64;
                int64_t sum = a + b;
                
                if (((a ^ sum) & (b ^ sum)) < 0) {
                    result.type = EvalDouble;
                    result.value.f64 = (double)a + (double)b;
                } else {
                    result.type = EvalLong;
                    result.value.i64 = sum;
                }
                return result;
            }

            if (l.type == EvalString && r.type == EvalString) {
                char* str = string_allocate("");
                str = string_append(str, (char*) l.value.ptr);
                str = string_append(str, (char*) r.value.ptr);
                result.type = EvalString;
                result.value.ptr = str;
                return result;
            }

            // Fallback for other numeric combinations
            if (!eval_is_number(l) || !eval_is_number(r)) {
                result.type = EvalError;
                return result;
            }

            double lvalue = eval_coerce_to_double(l);
            double rvalue = eval_coerce_to_double(r);
            double sum = lvalue + rvalue;

            // Try to preserve integer type if possible
            double intpart;
            if (modf(sum, &intpart) == 0.0 && sum <= INT32_MAX && sum >= INT32_MIN) {
                result.type = EvalInt;
                result.value.i32 = (int32_t)sum;
            } else {
                result.type = EvalDouble;
                result.value.f64 = sum;
            }
            return result;
        }
        case AstBinarySub: {
            eval_result_t l = eval_eval_expression(_expression->ast0);
            eval_result_t r = eval_eval_expression(_expression->ast1);
            if (l.type == EvalError || r.type == EvalError) {
                result.type = EvalError;
                return result;
            }

            if (!eval_is_number(l) || !eval_is_number(r)) {
                result.type = EvalError;
                return result;
            }

            if (l.type == EvalInt && r.type == EvalInt) {
                int32_t a = l.value.i32;
                int32_t b = r.value.i32;
                int32_t diff = a - b;
                
                if (((a ^ b) & (a ^ diff)) < 0) {
                    result.type = EvalDouble;
                    result.value.f64 = (double)a - (double)b;
                } else {
                    result.type = EvalInt;
                    result.value.i32 = diff;
                }
                return result;
            }

            if (l.type == EvalLong && r.type == EvalLong) {
                int64_t a = l.value.i64;
                int64_t b = r.value.i64;
                int64_t diff = a - b;
                
                if (((a ^ b) & (a ^ diff)) < 0) {
                    result.type = EvalDouble;
                    result.value.f64 = (double)a - (double)b;
                } else {
                    result.type = EvalLong;
                    result.value.i64 = diff;
                }
                return result;
            }

            double lvalue = eval_coerce_to_double(l);
            double rvalue = eval_coerce_to_double(r);
            double diff = lvalue - rvalue;

            double intpart;
            if (modf(diff, &intpart) == 0.0 && diff <= INT32_MAX && diff >= INT32_MIN) {
                result.type = EvalInt;
                result.value.i32 = (int32_t)diff;
            } else {
                result.type = EvalDouble;
                result.value.f64 = diff;
            }
            return result;
        }
        default:
            result.type = EvalError;
    }
    return result;
}

eval_result_t eval_eval(ast_node_t* _expression) {
    return eval_eval_expression(_expression);
}