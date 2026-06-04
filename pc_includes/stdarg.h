/**
 * @file stdarg.h
 * @brief Variadic argument support for the Peach C Compiler runtime.
 *
 * Provides va_list type and va_arg macro for accessing variadic function
 * arguments. Relies on the compiler's internal stdarg implementation
 * (stdarg-internal.h) which registers native functions for va_start,
 * va_arg, and va_end.
 */

#ifndef STDARG_H
#define STDARG_H
#include <stdarg-internal.h>

typedef int __builtin_va_list;
typedef __builtin_va_list va_list;

/* Expands to a native call that advances the va_list by sizeof(type) bytes */
#define va_arg(list, type) __builtin_va_arg(list, sizeof(type))
#endif