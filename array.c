/**
 * @file array.c
 * @brief Array bracket management and size calculation utilities.
 *
 * Handles the creation and manipulation of array bracket structures used to
 * represent multi-dimensional array declarations. Provides functions to
 * calculate total array sizes and individual dimension multipliers.
 */

#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>

/** @brief Allocates a new array_brackets structure with an empty bracket vector. */
struct array_brackets* array_brackets_new()
{
    struct array_brackets* brackets = calloc(1, sizeof(struct array_brackets));
    brackets->n_brackets = vector_create(sizeof(struct node*));
    return brackets;
}

/* BUG: Memory leak - brackets->n_brackets (created with vector_create) is never freed.
 * Only the outer struct is freed, leaking the internal vector and all its contents.
 * FIX: Call vector_free(brackets->n_brackets) before free(brackets).
 */
/** @brief Frees an array_brackets structure. */
void array_brackets_free(struct array_brackets* brackets)
{
    free(brackets);
}

/** @brief Adds a bracket node (e.g., [50]) to the array brackets list. */
void array_brackets_add(struct array_brackets* brackets, struct node* bracket_node)
{
    assert(bracket_node->type == NODE_TYPE_BRACKET);
    vector_push(brackets->n_brackets, &bracket_node);
}

/** @brief Returns the underlying vector of bracket nodes. */
struct vector* array_brackets_node_vector(struct array_brackets* brackets)
{
    return brackets->n_brackets;
}

/**
 * @brief Calculates the total array size starting from a given bracket index.
 * Multiplies the base datatype size by each bracket dimension from index onward.
 */
size_t array_brackets_calculate_size_from_index(struct datatype* dtype, struct array_brackets* brackets, int index)
{
    struct vector* array_vec = array_brackets_node_vector(brackets);
    size_t size = dtype->size;
    if (index >= vector_count(array_vec))
    {
        // char* abc;
        // return abc[0]; return abc[1];
        return size;
    }

    vector_set_peek_pointer(array_vec, index);
    struct node* array_bracket_node = vector_peek_ptr(array_vec);
    if (!array_bracket_node)
    {
        return 0;
    }

    while(array_bracket_node)
    {
        assert(array_bracket_node->bracket.inner->type == NODE_TYPE_NUMBER);
        /* BUG: Potential integer truncation - llnum is likely long long but is
         * stored in an int. Large array dimensions could silently overflow.
         * FIX: Use long long or size_t for 'number' to match the llnum type.
         */
        int number = array_bracket_node->bracket.inner->llnum;
        size *= number;
        array_bracket_node = vector_peek_ptr(array_vec);
    }

    return size;
}

/** @brief Returns the number of bracket dimensions in the array datatype. */
size_t array_brackets_count(struct datatype* dtype)
{
    return vector_count(dtype->array.brackets->n_brackets);
}

/** @brief Calculates the total size of an array (all dimensions multiplied by element size). */
size_t array_brackets_calculate_size(struct datatype* dtype, struct array_brackets* brackets)
{
    return array_brackets_calculate_size_from_index(dtype, brackets, 0);
}

/** @brief Returns the total number of index dimensions for an array datatype. */
int array_total_indexes(struct datatype* dtype)
{
    assert(dtype->flags & DATATYPE_FLAG_IS_ARRAY);
    struct array_brackets* brackets = dtype->array.brackets;
    return vector_count(brackets->n_brackets);
}
