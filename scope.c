/**
 * @file scope.c
 * @brief Scope management for variable and symbol resolution.
 *
 * Implements a hierarchical scope system where each scope has a parent pointer.
 * Scopes are used during parsing and validation to track variable visibility
 * and ensure correct name resolution within nested blocks.
 */

#include "compiler.h"
#include "helpers/vector.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

/** @brief Allocates a new scope with an empty entity vector (iterates in reverse). */
struct scope* scope_alloc()
{
    struct scope* scope = calloc(1, sizeof(struct scope));
    scope->entities = vector_create(sizeof(void*));
    vector_set_peek_pointer_end(scope->entities);
    vector_set_flag(scope->entities, VECTOR_FLAG_PEEK_DECREMENT);
    return scope;
}

/** @brief Deallocates a scope. Currently a no-op placeholder for future cleanup. */
void scope_dealloc(struct scope* scope)
{
    // Do nothing for now.
}

/** @brief Creates the root (global) scope for a compile process. Must be called once. */
struct scope* scope_create_root(struct compile_process* process)
{
    assert(!process->scope.root);
    assert(!process->scope.current);

    struct scope* root_scope = scope_alloc();
    process->scope.root = root_scope;
    process->scope.current = root_scope;
    return root_scope;
}

/** @brief Frees the root scope and resets all scope pointers to NULL. */
void scope_free_root(struct compile_process* process)
{
    scope_dealloc(process->scope.root);
    process->scope.root = NULL;
    process->scope.current = NULL;
}

/** @brief Creates a new child scope under the current scope and makes it active. */
struct scope* scope_new(struct compile_process* process, int flags)
{
    assert(process->scope.root);
    assert(process->scope.current);

    struct scope* new_scope = scope_alloc();
    new_scope->flags = flags;
    new_scope->parent = process->scope.current;
    process->scope.current = new_scope;
    return new_scope;
}

/** @brief Resets the peek pointer for iterating over scope entities. */
void scope_iteration_start(struct scope* scope)
{
    vector_set_peek_pointer(scope->entities, 0);
    if (scope->entities->flags & VECTOR_FLAG_PEEK_DECREMENT)
    {
        vector_set_peek_pointer_end(scope->entities);
    }


}

/** @brief Ends scope iteration. Currently a no-op placeholder. */
void scope_iteration_end(struct scope* scope)
{

}

/** @brief Returns the next entity during reverse iteration, or NULL if empty. */
void* scope_iterate_back(struct scope* scope)
{
    if (vector_count(scope->entities) == 0)
        return NULL;

    return vector_peek_ptr(scope->entities);
}

/** @brief Returns the most recently added entity in the given scope. */
void* scope_last_entity_at_scope(struct scope* scope)
{
     if (vector_count(scope->entities) == 0)
        return NULL;

    return vector_back_ptr(scope->entities); 
}

/** @brief Walks up the scope chain to find the last entity, stopping at stop_scope. */
void* scope_last_entity_from_scope_stop_at(struct scope* scope, struct scope* stop_scope)
{
    if (scope == stop_scope)
    {
        return NULL;
    }

    void* last = scope_last_entity_at_scope(scope);
    if (last)
    {
        return last;
    }

    struct scope* parent = scope->parent;
    if (parent)
    {
        return scope_last_entity_from_scope_stop_at(parent, stop_scope);
    }

    return NULL;
}

/** @brief Returns the last entity from the current scope up to stop_scope. */
void* scope_last_entity_stop_at(struct compile_process* process, struct scope* stop_scope)
{
    return scope_last_entity_from_scope_stop_at(process->scope.current, stop_scope);
}

/** @brief Returns the last entity visible from the current scope (searches all parents). */
void* scope_last_entity(struct compile_process* process)
{
   return scope_last_entity_stop_at(process, NULL);
}

/** @brief Pushes an entity pointer into the current scope and updates its size. */
void scope_push(struct compile_process* process, void* ptr, size_t elem_size)
{
    vector_push(process->scope.current->entities, &ptr);
    process->scope.current->size += elem_size;
}

/** @brief Finishes the current scope, restoring the parent as active. */
void scope_finish(struct compile_process* process)
{
    struct scope* new_current_scope = process->scope.current->parent;
    scope_dealloc(process->scope.current);
    process->scope.current = new_current_scope;
    if (process->scope.root && !process->scope.current)
    {
        process->scope.root = NULL;
    }
}

/** @brief Returns a pointer to the currently active scope. */
struct scope* scope_current(struct compile_process* process)
{
    return process->scope.current;
}
