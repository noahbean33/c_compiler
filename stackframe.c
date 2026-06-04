/**
 * @file stackframe.c
 * @brief Stack frame tracking for code generation.
 *
 * Maintains a virtual representation of the x86 stack frame for each function
 * during code generation. Tracks pushed values, local variables, saved registers,
 * and their offsets from the base pointer (EBP). Used to ensure correct stack
 * alignment and to verify the stack is balanced at function exit.
 */

#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>

/** @brief Removes the top element from the function's stack frame. */
void stackframe_pop(struct node* func_node)
{
    struct stack_frame* frame = &func_node->func.frame;
    vector_pop(frame->elements);
}

/** @brief Returns the top element of the stack frame without removing it, or NULL if empty. */
struct stack_frame_element* stackframe_back(struct node* func_node)
{
    return vector_back_or_null(func_node->func.frame.elements);
}

/** @brief Returns the top element only if it matches the expected type and name, else NULL. */
struct stack_frame_element* stackframe_back_expect(struct node* func_node, int expecting_type, const char* expecting_name)
{
    struct stack_frame_element* element = stackframe_back(func_node);
    /* BUG: Operator precedence issue. && binds tighter than ||, so this is parsed as:
     *   (element && element->type != expecting_type) || (!S_EQ(element->name, expecting_name))
     * If element is NULL, the second condition still evaluates, dereferencing element->name (NULL dereference).
     * FIX: Add parentheses: if (element && (element->type != expecting_type || !S_EQ(element->name, expecting_name)))
     */
    if (element && element->type != expecting_type || !S_EQ(element->name, expecting_name))
    {
        return NULL;
    }
    return element;
}

/** @brief Pops the top element, asserting it matches the expected type and name. */
void stackframe_pop_expecting(struct node* func_node, int expecting_type, const char* expecting_name)
{
    struct stack_frame* frame = &func_node->func.frame;
    struct stack_frame_element* last_element = stackframe_back(func_node);
    assert(last_element);
    assert(last_element->type == expecting_type && S_EQ(last_element->name, expecting_name));
    stackframe_pop(func_node);
}

/** @brief Initializes peek iteration from the top of the stack frame downward. */
void stackframe_peek_start(struct node* func_node)
{
    struct stack_frame* frame = &func_node->func.frame;
    vector_set_peek_pointer_end(frame->elements);
    vector_set_flag(frame->elements, VECTOR_FLAG_PEEK_DECREMENT);
}

/** @brief Returns the next element during peek iteration through the stack frame. */
struct stack_frame_element* stackframe_peek(struct node* func_node)
{
    struct stack_frame* frame = &func_node->func.frame;
    return vector_peek(frame->elements);
}


/** @brief Pushes a new element onto the stack frame, calculating its EBP offset. */
void stackframe_push(struct node* func_node, struct stack_frame_element* element)
{
    struct stack_frame* frame = &func_node->func.frame;
    // The stack grows downwards 
    element->offset_from_bp = -(vector_count(frame->elements) * STACK_PUSH_SIZE);
    vector_push(frame->elements, element);
}
/** @brief Simulates a stack subtraction (SUB ESP) by pushing multiple DWORD entries. */
void stackframe_sub(struct node* func_node, int type, const char* name, size_t amount)
{
    assert((amount % STACK_PUSH_SIZE) == 0);
    size_t total_pushes = amount / STACK_PUSH_SIZE;
    for (size_t i = 0; i < total_pushes; i++)
    {
        stackframe_push(func_node, &(struct stack_frame_element){.type=type,.name=name});
    }
}

/** @brief Simulates a stack addition (ADD ESP) by popping multiple entries. */
void stackframe_add(struct node* func_node, int type, const char* name, size_t amount)
{
    assert((amount % STACK_PUSH_SIZE) == 0);
    size_t total_pushes = amount / STACK_PUSH_SIZE;
    for (size_t i = 0; i < total_pushes; i++)
    {
       stackframe_pop(func_node);
    }
}

/** @brief Asserts that the stack frame is empty (used at function exit). */
void stackframe_assert_empty(struct node* func_node)
{
    struct stack_frame* frame = &func_node->func.frame;
    assert(vector_count(frame->elements) == 0);
}