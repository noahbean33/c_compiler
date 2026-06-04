/**
 * @file cprocess.c
 * @brief Compile process creation and character I/O for the lexer.
 *
 * Manages the creation of compile_process instances, sets up include
 * directories, and provides the character-level read/peek/push operations
 * that the lexer uses to consume source file input.
 */

#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "helpers/vector.h"

/* Default search paths for #include directives */
const char* default_include_dirs[] = {"./pc_includes", "../pc_includes", "/usr/include/peach-includes", "/usr/include"};

/** @brief Resets the include directory iterator and returns the first directory. */
const char* compiler_include_dir_begin(struct compile_process* process)
{
    vector_set_peek_pointer(process->include_dirs, 0);
    const char* dir = vector_peek_ptr(process->include_dirs);
    return dir;
}

/** @brief Advances the include directory iterator and returns the next directory. */
const char* compiler_include_dir_next(struct compile_process* process)
{
    const char* dir =vector_peek_ptr(process->include_dirs);
    return dir;
}

/** @brief Populates the include directory vector with default search paths. */
void compiler_setup_default_include_directories(struct vector* include_vec)
{
    size_t total = sizeof(default_include_dirs) / sizeof(const char*);
    for (int i = 0; i < total; i++)
    {
        vector_push(include_vec, &default_include_dirs[i]);
    }
}

/**
 * @brief Creates and initializes a new compile_process for a source file.
 *
 * Opens the input/output files, allocates token and node vectors, initializes
 * the code generator, resolver, symbol table, and preprocessor. Child processes
 * (for #include) inherit the preprocessor and include dirs from the parent.
 *
 * @param filename Input source file path
 * @param filename_out Output assembly file path (NULL for include-only processes)
 * @param flags Compilation flags
 * @param parent_process Parent process for includes, or NULL for top-level compilation
 * @return Pointer to the new compile_process, or NULL on failure
 */
struct compile_process *compile_process_create(const char *filename, const char *filename_out, int flags, struct compile_process* parent_process)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        return NULL;
    }

    FILE *out_file = NULL;
    if (filename_out)
    {
        out_file = fopen(filename_out, "w");
        if (!out_file)
        {
            return NULL;
        }
    }

    struct compile_process* process = calloc(1, sizeof(struct compile_process));
    process->token_vec = vector_create(sizeof(struct token));
    process->token_vec_original = vector_create(sizeof(struct token));
    process->node_vec = vector_create(sizeof(struct node*));
    process->node_tree_vec = vector_create(sizeof(struct node*));
    
    process->flags = flags;
    process->cfile.fp = file;
    process->ofile = out_file;
    process->generator = codegenerator_new(process);
    process->resolver = resolver_default_new_process(process);

    symresolver_initialize(process);
    symresolver_new_table(process);
    
    if (parent_process)
    {
        /* Inherit preprocessor state and include paths from parent */
        process->preprocessor = parent_process->preprocessor;
        process->include_dirs = parent_process->include_dirs;
    }
    else
    {
        /* Top-level process: create a fresh preprocessor and load defaults */
        process->preprocessor = preprocessor_create(process);
        process->include_dirs = vector_create(sizeof(const char*));
        compiler_setup_default_include_directories(process->include_dirs);
    }
    
    char* path = malloc(PATH_MAX);
    realpath(filename, path);
    process->cfile.abs_path = path;
    node_set_vector(process->node_vec, process->node_tree_vec);
    return process;
}

/**
 * @brief Reads and returns the next character from the source file.
 * Updates the compiler position (line and column tracking).
 */
char compile_process_next_char(struct lex_process* lex_process)
{
    struct compile_process* compiler = lex_process->compiler;
    compiler->pos.col += 1;
    char c = getc(compiler->cfile.fp);
    if (c == '\n')
    {
        compiler->pos.line +=1 ;
        compiler->pos.col = 1;
    }

    return c;
}

/**
 * @brief Peeks at the next character without consuming it from the stream.
 */
char compile_process_peek_char(struct lex_process* lex_process)
{
    struct compile_process* compiler = lex_process->compiler;
    char c = getc(compiler->cfile.fp);
    ungetc(c, compiler->cfile.fp);
    return c;
}

/**
 * @brief Pushes a character back onto the input stream for re-reading.
 */
void compile_process_push_char(struct lex_process* lex_process, char c)
{
    struct compile_process* compiler = lex_process->compiler;
    ungetc(c, compiler->cfile.fp);
}