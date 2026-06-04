/**
 * @file compiler.c
 * @brief Core compilation pipeline orchestration.
 *
 * Implements the top-level compilation flow including file inclusion,
 * error/warning reporting, and sequencing of lexical analysis, preprocessing,
 * parsing, validation, and code generation phases.
 */

#include "compiler.h"
#include <stdarg.h>
#include <stdlib.h>

/* Lexer function table for reading directly from the source file */
struct lex_process_functions compiler_lex_functions = {
    .next_char=compile_process_next_char,
    .peek_char=compile_process_peek_char,
    .push_char=compile_process_push_char
};

/**
 * @brief Reports a compilation error associated with a specific AST node.
 * Prints the error message with source location and terminates the process.
 */
void compiler_node_error(struct node* node, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, " on line %i, col %i in file %s\n", node->pos.line, node->pos.col, node->pos.filename);
    exit(-1);
}

/**
 * @brief Reports a fatal compilation error at the current compiler position.
 * Prints the formatted error message with file location and terminates.
 */
void compiler_error(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
    exit(-1);
}

/**
 * @brief Reports a non-fatal compilation warning at the current compiler position.
 * Prints the formatted warning message with file location but does not terminate.
 */
void compiler_warning(struct compile_process* compiler, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, " on line %i, col %i in file %s\n", compiler->pos.line, compiler->pos.col, compiler->pos.filename);
}

/**
 * @brief Attempts to include a file from a specific include directory.
 * Performs lexical analysis and preprocessing on the included file.
 *
 * @param include_dir The directory to search for the include file
 * @param filename The name of the file to include
 * @param parent_process The parent compilation process requesting the include
 * @return Pointer to the new compile_process, or NULL on failure
 */
struct compile_process* compile_include_for_include_dir(const char* include_dir, const char* filename, struct compile_process* parent_process)
{
    char tmp_filename[512];
    sprintf(tmp_filename, "%s/%s", include_dir, filename);
    if (file_exists(tmp_filename))
    {
        filename = tmp_filename;
    }
    struct compile_process* process = compile_process_create(filename, NULL, parent_process->flags, parent_process);
    if (!process)
    {
        return NULL;
    }

    struct lex_process* lex_process = lex_process_create(process, &compiler_lex_functions, NULL);
    if (!lex_process)
    {
        return NULL;
    }

    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
        return NULL;
    }

    process->token_vec_original = lex_process_tokens(lex_process);
    if (preprocessor_run(process) < 0)
    {
        return NULL;
    }

    return process;
}
/**
 * @brief Includes a file to be compiled, returns a new compile process that represents the file to be compiled
 * 
 * Note: Only lexical analysis, and preprocessing are done for compiler includes
 * Parsing and code generation are excluded.
 * @param filename 
 * @param parent_process 
 * @return struct compile_process* 
 */

struct compile_process* compile_include(const char* filename, struct compile_process* parent_process)
{
    struct compile_process* new_process = NULL;
    const char* include_dir = compiler_include_dir_begin(parent_process);
    while(include_dir && !new_process)
    {
        new_process = compile_include_for_include_dir(include_dir, filename, parent_process);
        include_dir = compiler_include_dir_next(parent_process);
    }

    return new_process;
}

/**
 * @brief Main compilation entry point. Runs the full pipeline on a source file.
 *
 * Pipeline stages:
 *   1. Lexical analysis - tokenizes the source file
 *   2. Preprocessing - handles macros, includes, and conditional compilation
 *   3. Parsing - builds the abstract syntax tree (AST)
 *   4. Validation - semantic analysis and type checking
 *   5. Code generation - emits x86 NASM assembly
 *
 * @param filename Path to the input C source file
 * @param out_filename Path for the generated assembly output file
 * @param flags Compilation flags (e.g., COMPILE_PROCESS_EXECUTE_NASM)
 * @return COMPILER_FILE_COMPILED_OK on success, COMPILER_FAILED_WITH_ERRORS on failure
 */
int compile_file(const char* filename, const char* out_filename, int flags)
{
    struct compile_process* process = compile_process_create(filename, out_filename, flags, NULL);
    if (!process)
        return COMPILER_FAILED_WITH_ERRORS;

    /* Stage 1: Lexical analysis - tokenize the source */
    struct lex_process* lex_process = lex_process_create(process, &compiler_lex_functions, NULL);
    if (!lex_process)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    if (lex(lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    /* Stage 2: Preprocessing - resolve macros and directives */
    process->token_vec_original = lex_process_tokens(lex_process);
    if (preprocessor_run(process) != 0)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }
    
    /* Stage 3: Parsing - construct the AST from tokens */
    if (parse(process) != PARSE_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    /* Stage 4: Validation - semantic checks */
    if (validate(process) != VALIDATION_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    /* Stage 5: Code generation - emit assembly output */
    if (codegen(process) != CODEGEN_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    fclose(process->ofile);
    return COMPILER_FILE_COMPILED_OK;
}