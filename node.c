/**
 * @file node.c
 * @brief AST node creation and management.
 *
 * Implements factory functions for creating all AST node types (expressions,
 * statements, functions, variables, structs, unions, etc.) and provides
 * utilities for querying and manipulating nodes on the node stack.
 */

#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>

/* Global node vectors used during parsing */
struct vector *node_vector = NULL;
struct vector *node_vector_root = NULL;

/* Tracks the current body and function context during parsing */
struct node *parser_current_body = NULL;
struct node *parser_current_function = NULL;

/** @brief Sets the global node vectors used during parsing. */
void node_set_vector(struct vector *vec, struct vector *root_vec)
{
    node_vector = vec;
    node_vector_root = root_vec;
}

/** @brief Pushes a node pointer onto the current node stack. */
void node_push(struct node *node)
{
    vector_push(node_vector, &node);
}

/** @brief Returns the top node without popping, or NULL if empty. */
struct node *node_peek_or_null()
{
    return vector_back_ptr_or_null(node_vector);
}

/** @brief Returns the top node without popping. Asserts non-empty. */
struct node *node_peek()
{
    return *(struct node **)(vector_back(node_vector));
}

/** @brief Pops and returns the top node; also removes from root vector if it's the same. */
struct node *node_pop()
{
    struct node *last_node = vector_back_ptr(node_vector);
    struct node *last_node_root = vector_empty(node_vector) ? NULL : vector_back_ptr_or_null(node_vector_root);

    vector_pop(node_vector);

    if (last_node == last_node_root)
    {
        vector_pop(node_vector_root);
    }

    return last_node;
}

/** @brief Returns true if the node type can appear in an expression context. */
bool node_is_expressionable(struct node *node)
{
    return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHESES || node->type == NODE_TYPE_UNARY || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_STRING;
}

/** @brief Returns the top node if it's expressionable, otherwise NULL. */
struct node *node_peek_expressionable_or_null()
{
    struct node *last_node = node_peek_or_null();
    return node_is_expressionable(last_node) ? last_node : NULL;
}

void make_default_node()
{
    node_create(&(struct node){.type=NODE_TYPE_STATEMENT_DEFAULT});
}
void make_cast_node(struct datatype *dtype, struct node *operand_node)
{
    node_create(&(struct node){.type = NODE_TYPE_CAST, .cast.dtype = *dtype, .cast.operand = operand_node});
}

void make_tenary_node(struct node *true_node, struct node *false_node)
{
    node_create(&(struct node){.type = NODE_TYPE_TENARY, .tenary.true_node = true_node, .tenary.false_node = false_node});
}

void make_case_node(struct node *exp_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_CASE, .stmt._case.exp = exp_node});
}

void make_goto_node(struct node *label_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_GOTO, .stmt._goto.label = label_node});
}

void make_label_node(struct node *name_node)
{
    node_create(&(struct node){.type = NODE_TYPE_LABEL, .label.name = name_node});
}
void make_continue_node()
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_CONTINUE});
}

void make_break_node()
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_BREAK});
}

/** @brief Creates a binary expression node with left operand, right operand, and operator. */
void make_exp_node(struct node *left_node, struct node *right_node, const char *op)
{
    assert(left_node);
    assert(right_node);
    node_create(&(struct node){.type = NODE_TYPE_EXPRESSION, .exp.left = left_node, .exp.right = right_node, .exp.op = op});
}

void make_exp_parentheses_node(struct node *exp_node)
{
    node_create(&(struct node){.type = NODE_TYPE_EXPRESSION_PARENTHESES, .parenthesis.exp = exp_node});
}

void make_bracket_node(struct node *node)
{
    node_create(&(struct node){.type = NODE_TYPE_BRACKET, .bracket.inner = node});
}

void make_body_node(struct vector *body_vec, size_t size, bool padded, struct node *largest_var_node)
{
    node_create(&(struct node){.type = NODE_TYPE_BODY, .body.statements = body_vec, .body.size = size, .body.padded = padded, .body.largest_var_node = largest_var_node});
}

void make_struct_node(const char *name, struct node *body_node)
{
    int flags = 0;
    if (!body_node)
    {
        flags |= NODE_FLAG_IS_FORWARD_DECLARATION;
    }

    node_create(&(struct node){.type = NODE_TYPE_STRUCT, ._struct.body_n = body_node, ._struct.name = name, .flags = flags});
}

void make_union_node(const char *name, struct node *body_node)
{
    int flags = 0;
    if (!body_node)
    {
        flags |= NODE_FLAG_IS_FORWARD_DECLARATION;
    }

    node_create(&(struct node){.type = NODE_TYPE_UNION, ._union.body_n = body_node, ._union.name = name, .flags = flags});
}

void make_function_node(struct datatype *ret_type, const char *name, struct vector *arguments, struct node *body_node)
{
    struct node* function_node = node_create(&(struct node){.type = NODE_TYPE_FUNCTION, .func.name = name, .func.args.vector = arguments, .func.body_n = body_node, .func.rtype = *ret_type, .func.args.stack_addition = DATA_SIZE_DDWORD});
    function_node->func.frame.elements = vector_create(sizeof(struct stack_frame_element));
}

void make_switch_node(struct node *exp_node, struct node *body_node, struct vector *cases, bool has_default_case)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_SWITCH, .stmt.switch_stmt.exp = exp_node, .stmt.switch_stmt.body = body_node, .stmt.switch_stmt.cases = cases, .stmt.switch_stmt.has_default_case = has_default_case});
}

void make_do_while_node(struct node *body_node, struct node *exp_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_DO_WHILE, .stmt.do_while_stmt.body_node = body_node, .stmt.do_while_stmt.exp_node = exp_node});
}

void make_while_node(struct node *exp_node, struct node *body_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_WHILE, .stmt.while_stmt.exp_node = exp_node, .stmt.while_stmt.body_node = body_node});
}

void make_for_node(struct node *init_node, struct node *cond_node, struct node *loop_node, struct node *body_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_FOR, .stmt.for_stmt.init_node = init_node, .stmt.for_stmt.cond_node = cond_node, .stmt.for_stmt.loop_node = loop_node, .stmt.for_stmt.body_node = body_node});
}

void make_return_node(struct node *exp_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_RETURN, .stmt.return_stmt.exp = exp_node});
}

void make_else_node(struct node *body_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_ELSE, .stmt.else_stmt.body_node = body_node});
}

void make_if_node(struct node *cond_node, struct node *body_node, struct node *next_node)
{
    node_create(&(struct node){.type = NODE_TYPE_STATEMENT_IF, .stmt.if_stmt.cond_node = cond_node, .stmt.if_stmt.body_node = body_node, .stmt.if_stmt.next = next_node});
}

void make_unary_node(const char* op, struct node* operand_node, int flags)
{
    node_create(&(struct node){.type=NODE_TYPE_UNARY,.unary.op=op, .unary.operand=operand_node, .unary.flags=flags});
}

/** @brief Returns the node stored in a symbol, or NULL if not a NODE-type symbol. */
struct node *node_from_sym(struct symbol *sym)
{
    if (sym->type != SYMBOL_TYPE_NODE)
    {
        return NULL;
    }

    return sym->data;
}

/** @brief Looks up a node by name in the symbol table. */
struct node *node_from_symbol(struct compile_process *current_process, const char *name)
{
    struct symbol *sym = symresolver_get_symbol(current_process, name);
    if (!sym)
    {
        return NULL;
    }
    return node_from_sym(sym);
}

/** @brief Retrieves a struct node by name from the symbol table. */
struct node *struct_node_for_name(struct compile_process *current_process, const char *name)
{
    struct node *node = node_from_symbol(current_process, name);
    if (!node)
        return NULL;

    if (node->type != NODE_TYPE_STRUCT)
        return NULL;

    return node;
}

/** @brief Retrieves a union node by name from the symbol table. */
struct node *union_node_for_name(struct compile_process *current_process, const char *name)
{
    struct node *node = node_from_symbol(current_process, name);
    if (!node)
        return NULL;

    if (node->type != NODE_TYPE_UNION)
        return NULL;

    return node;
}

/**
 * @brief Allocates a new node, copies the provided template, binds it to the
 * current function/body context, and pushes it onto the node stack.
 */
struct node *node_create(struct node *_node)
{
    struct node *node = malloc(sizeof(struct node));
    memcpy(node, _node, sizeof(struct node));
    node->binded.owner = parser_current_body;
    node->binded.function = parser_current_function;
    node_push(node);
    return node;
}

/** @brief Returns true if the node is a struct or union definition. */
bool node_is_struct_or_union(struct node* node)
{
    return node->type == NODE_TYPE_STRUCT || node->type == NODE_TYPE_UNION;
}

/** @brief Returns true if the node is a variable whose type is a struct or union. */
bool node_is_struct_or_union_variable(struct node *node)
{
    if (node->type != NODE_TYPE_VARIABLE)
    {
        return false;
    }

    return datatype_is_struct_or_union(&node->var.type);
}

/** @brief Extracts the variable node from a variable, struct, or union node. */
struct node *variable_node(struct node *node)
{

    struct node *var_node = NULL;
    switch (node->type)
    {
    case NODE_TYPE_VARIABLE:
        var_node = node;
        break;

    case NODE_TYPE_STRUCT:
        var_node = node->_struct.var;
        break;

    case NODE_TYPE_UNION:
        var_node = node->_union.var;
        break;
    }

    return var_node;
}

/** @brief Returns true if the variable node has a primitive datatype. */
bool variable_node_is_primitive(struct node *node)
{
    assert(node->type == NODE_TYPE_VARIABLE);
    return datatype_is_primitive(&node->var.type);
}

/** @brief Returns the variable node, or the node itself if it's a variable list. */
struct node *variable_node_or_list(struct node *node)
{
    if (node->type == NODE_TYPE_VARIABLE_LIST)
    {
        return node;
    }

    return variable_node(node);
}

/** @brief Returns the stack space added by function arguments (default 8 bytes for EBP+EIP). */
size_t function_node_argument_stack_addition(struct node *node)
{
    assert(node->type == NODE_TYPE_FUNCTION);
    return node->func.args.stack_addition;
}

/** @brief Returns the total stack size allocated for the function's local variables. */
size_t function_node_stack_size(struct node* node)
{
    assert(node->type == NODE_TYPE_FUNCTION);
    return node->func.stack_size;
}

/** @brief Returns true if the function node is a prototype (no body). */
bool function_node_is_prototype(struct node* node)
{
    return node->func.body_n == NULL;
}

/** @brief Returns the vector of argument nodes for a function. */
struct vector* function_node_argument_vec(struct node* node)
{
    assert(node->type == NODE_TYPE_FUNCTION);
    return node->func.args.vector;
}
/** @brief Returns true if the node is an expression or parenthesized expression. */
bool node_is_expression_or_parentheses(struct node *node)
{
    return node->type == NODE_TYPE_EXPRESSION_PARENTHESES || node->type == NODE_TYPE_EXPRESSION;
}

/** @brief Returns true if the node represents a value (expression, identifier, number, etc.). */
bool node_is_value_type(struct node *node)
{
    return node_is_expression_or_parentheses(node) || node->type == NODE_TYPE_IDENTIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_UNARY || node->type == NODE_TYPE_TENARY || node->type == NODE_TYPE_STRING;
}

/** @brief Returns true if the node is an expression with the given operator. */
bool node_is_expression(struct node *node, const char *op)
{
    return node->type == NODE_TYPE_EXPRESSION && S_EQ(node->exp.op, op);
}

/** @brief Returns true if the node is an assignment expression (=, +=, -=, etc.). */
bool is_node_assignment(struct node *node)
{
    if (node->type != NODE_TYPE_EXPRESSION)
        return false;

    return S_EQ(node->exp.op, "=") ||
           S_EQ(node->exp.op, "+=") ||
           S_EQ(node->exp.op, "-=") ||
           S_EQ(node->exp.op, "/=") ||
           S_EQ(node->exp.op, "*=") ||
           S_EQ(node->exp.op, ">>=") ||
           S_EQ(node->exp.op, "<<=");
}

/** @brief Returns true if the node pointer is non-NULL and not a blank node. */
bool node_valid(struct node* node)
{
    return node && node->type != NODE_TYPE_BLANK;
}