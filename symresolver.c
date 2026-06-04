/**
 * @file symresolver.c
 * @brief Symbol table management and lookup.
 *
 * Implements a symbol resolver that registers and retrieves symbols (variables,
 * functions, structs, unions) by name. Supports stacked symbol tables for
 * nested scope management and separate lookup for native (built-in) functions.
 */

#include "compiler.h"
#include "helpers/vector.h"

/** @brief Pushes a symbol pointer onto the current active symbol table. */
static void symresolver_push_symbol(struct compile_process* process, struct symbol* sym)
{
    vector_push(process->symbols.table, &sym);
}

/** @brief Initializes the symbol table stack for a compile process. */
void symresolver_initialize(struct compile_process* process)
{
    process->symbols.tables = vector_create(sizeof(struct vector*));
}


/** @brief Creates a new symbol table, saving the current one on the table stack. */
void symresolver_new_table(struct compile_process* process)
{
    // Save the current table
    vector_push(process->symbols.tables, &process->symbols.table);

    // Overwrite the active table
    process->symbols.table = vector_create(sizeof(struct symbol*));
}

/** @brief Restores the previous symbol table from the stack. */
void symresolver_end_table(struct compile_process* process)
{
    struct vector* last_table = vector_back_ptr(process->symbols.tables);
    process->symbols.table = last_table;
    vector_pop(process->symbols.tables);
}

/** @brief Looks up a symbol by name in the current symbol table. Returns NULL if not found. */
struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name)
{
    vector_set_peek_pointer(process->symbols.table, 0);
    struct symbol* symbol = vector_peek_ptr(process->symbols.table);
    while(symbol)
    {
        if (S_EQ(symbol->name, name))
        {
            break;
        }

        symbol = vector_peek_ptr(process->symbols.table);
    }

    return symbol;
}


/** @brief Looks up a native function symbol by name. Returns NULL if not found or wrong type. */
struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name)
{
    struct symbol* sym = symresolver_get_symbol(process, name);
    if (!sym)
    {
        return NULL;
    }

    if (sym->type != SYMBOL_TYPE_NATIVE_FUNCTION)
    {
        return NULL;
    }

    return sym;
}


/** @brief Registers a new symbol. Returns NULL if the name is already registered. */
struct symbol* symresolver_register_symbol(struct compile_process* process, const char* sym_name, int type, void* data)
{
    if (symresolver_get_symbol(process, sym_name))
    {
        return NULL;
    }

    struct symbol* sym = calloc(1, sizeof(struct symbol));
    sym->name = sym_name;
    sym->type = type;
    sym->data = data;
    symresolver_push_symbol(process, sym);
    return sym;
}

/** @brief Returns the AST node associated with a NODE-type symbol, or NULL. */
struct node* symresolver_node(struct symbol* sym)
{
    if (sym->type != SYMBOL_TYPE_NODE)
    {
        return NULL;
    }

    return sym->data;
}

/** @brief Registers a variable node in the symbol table. */
void symresolver_build_for_variable_node(struct compile_process* process, struct node* node)
{
    symresolver_register_symbol(process, node->var.name, SYMBOL_TYPE_NODE, node);
}

/** @brief Registers a function node in the symbol table. */
void symresolver_build_for_function_node(struct compile_process* process, struct node* node)
{
    symresolver_register_symbol(process, node->func.name, SYMBOL_TYPE_NODE, node);
}

/** @brief Registers a struct node in the symbol table (skips forward declarations). */
void symresolver_build_for_structure_node(struct compile_process* process, struct node* node)
{
    if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION)
    {
        // We do not register forward declarations.
        return;
    }

    symresolver_register_symbol(process, node->_struct.name, SYMBOL_TYPE_NODE, node);
}

/** @brief Registers a union node in the symbol table (skips forward declarations). */
void symresolver_build_for_union_node(struct compile_process* process, struct node* node)
{
    if (node->flags & NODE_FLAG_IS_FORWARD_DECLARATION)
    {
        // We do not register forward declarations.
        return;
    }

    symresolver_register_symbol(process, node->_union.name, SYMBOL_TYPE_NODE, node);
}

/** @brief Dispatches symbol registration based on node type (variable, function, struct, union). */
void symresolver_build_for_node(struct compile_process* process, struct node* node)
{
    switch(node->type)
    {
        case NODE_TYPE_VARIABLE:
        symresolver_build_for_variable_node(process, node);
        break;

        case NODE_TYPE_FUNCTION:
        symresolver_build_for_function_node(process, node);
        break;

        case NODE_TYPE_STRUCT:
        symresolver_build_for_structure_node(process, node);
        break;

        case NODE_TYPE_UNION:
        symresolver_build_for_union_node(process, node);
        break;

        // Ignore all other node types, because they cant become symbols.

    }
}