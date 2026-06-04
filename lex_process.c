/**
 * @file lex_process.c
 * @brief Lexer process management.
 *
 * Provides creation, destruction, and accessor functions for the lex_process
 * structure which encapsulates the state of a lexical analysis session.
 */

#include "compiler.h"
#include "helpers/vector.h"
#include <stdlib.h>

/**
 * @brief Creates and initializes a new lexer process.
 *
 * @param compiler The parent compile process
 * @param functions Function pointers for character I/O (next, peek, push)
 * @param private Opaque data accessible to the lexer function implementations
 * @return Pointer to the newly allocated lex_process
 */
struct lex_process* lex_process_create(struct compile_process* compiler, struct lex_process_functions* functions, void* private)
{
    struct lex_process* process = calloc(1, sizeof(struct lex_process));
    process->function = functions;
    process->token_vec = vector_create(sizeof(struct token));
    process->compiler = compiler;
    process->private = private;
    process->pos.line = 1;
    process->pos.col = 1;
    return process;
}

/**
 * @brief Frees the memory associated with a lexer process.
 */
void lex_process_free(struct lex_process* process)
{
    vector_free(process->token_vec);
    free(process);
}

/**
 * @brief Returns the private data associated with the lexer process.
 */
void* lex_process_private(struct lex_process* process)
{
    return process->private;
}

/**
 * @brief Returns the token vector produced by lexical analysis.
 */
struct vector* lex_process_tokens(struct lex_process* process)
{
    return process->token_vec;
}