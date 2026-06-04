/**
 * @file native.c
 * @brief Native preprocessor definitions and built-in function registration.
 *
 * Implements built-in preprocessor macros (e.g., __LINE__) and provides the
 * mechanism for registering native (compiler-intrinsic) functions that are
 * handled directly by the code generator rather than being linked externally.
 */

#include "compiler.h"
#include <stdlib.h>

/**
 * @brief Evaluates the __LINE__ macro to the current source line number.
 * Used in preprocessor #if expressions.
 */
int preprocessor_line_macro_evaluate(struct preprocessor_definition* definition, struct preprocessor_function_arguments* arguments)
{
    struct preprocessor* preprocessor = definition->preprocessor;
    struct compile_process* compiler = preprocessor->compiler;

    if (arguments)
    {
        compiler_error(compiler, "__LINE__ macro expects no arguments");
    }   

    struct token* previous_token = preprocessor_previous_token(compiler);
    return previous_token->pos.line;
}

/**
 * @brief Returns the __LINE__ macro value as a token vector for substitution.
 */
struct vector* preprocessor_line_macro_value(struct preprocessor_definition* definition, struct preprocessor_function_arguments* arguments)
{
    struct preprocessor* preprocessor = definition->preprocessor;
    struct compile_process* compiler = preprocessor->compiler;

    if (arguments)
    {
        compiler_error(compiler, "__LINE__ macro expects no arguments");
    }   
    struct token* previous_token = preprocessor_previous_token(compiler);
    return preprocessor_build_value_vector_for_integer(previous_token->pos.line);
}

/** @brief Registers all built-in preprocessor macro definitions (e.g., __LINE__). */
void preprocessor_create_definitions(struct preprocessor* preprocessor)
{
    preprocessor_definition_create_native("__LINE__", preprocessor_line_macro_evaluate, preprocessor_line_macro_value, preprocessor);   
}

/**
 * @brief Registers a native (compiler-intrinsic) function in the symbol table.
 * Native functions are handled directly by the code generator at call sites.
 *
 * @param compiler The compile process to register the function in
 * @param name The function name as it appears in source code
 * @param callbacks Function pointers invoked during code generation
 * @return The registered symbol, or NULL if the name already exists
 */
struct symbol* native_create_function(struct compile_process* compiler, const char* name,
 struct native_function_callbacks* callbacks)
{
    struct native_function* func = calloc(1, sizeof(struct native_function));
    memcpy(&func->callbacks, callbacks, sizeof(func->callbacks));
    func->name = name;
    return symresolver_register_symbol(compiler, name, SYMBOL_TYPE_NATIVE_FUNCTION, func);
}

/**
 * @brief Retrieves a registered native function by name.
 * @return Pointer to the native_function struct, or NULL if not found.
 */
struct native_function* native_function_get(struct compile_process* compiler, const char* name)
{
    struct symbol* sym = symresolver_get_symbol_for_native_function(compiler, name);
    if (!sym)
    {
        return NULL;
    }

    return sym->data;
}
