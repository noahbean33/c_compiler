/**
 * @file main.c
 * @brief Entry point for the Peach C Compiler.
 *
 * Handles command-line argument parsing, initiates the compilation pipeline,
 * and optionally assembles the output using NASM and links with GCC.
 *
 * Usage: ./compiler [input_file] [output_file] [option]
 *   - input_file:  Path to the C source file (default: ./test.c)
 *   - output_file: Path for generated assembly output (default: ./test)
 *   - option:      "exec" for full executable, "object" for object file only
 */

#include <stdio.h>
#include "helpers/vector.h"
#include "compiler.h"

int main(int argc, char** argv)
{
    /* Default file paths and compilation option */
    const char* input_file = "./test.c";
    const char* output_file = "./test";
    const char* option = "exec";

    /* Parse command-line arguments if provided */
    if (argc > 1)
    {
        input_file = argv[1];
    }

    if (argc > 2)
    {
        output_file = argv[2];
    }

    if (argc > 3)
    {
        option = argv[3];
    }

    /* Configure compilation flags based on the specified option */
    int compile_flags = COMPILE_PROCESS_EXECUTE_NASM;
    if (S_EQ(option, "object"))
    {
        compile_flags |= COMPILE_PROCESS_EXPORT_AS_OBJECT;
    }

    /* Execute the full compilation pipeline (lex, preprocess, parse, validate, codegen) */
    int res = compile_file(input_file, output_file, compile_flags);
    if (res == COMPILER_FILE_COMPILED_OK)
    {
        printf("everything compiled fine\n");
    }
    else if(res == COMPILER_FAILED_WITH_ERRORS)
    {
        printf("Compile failed\n");
        return 1;
    }
    else
    {
        printf("Unknown response for compile time\n");
        return 1;
    }

    /* Assemble and link the generated assembly output using NASM and GCC */
    if (compile_flags & COMPILE_PROCESS_EXECUTE_NASM)
    {
        /* BUG: Potential buffer overflow. nasm_output_file is only 40 bytes but output_file
         * could be up to PATH_MAX characters from argv[2], easily exceeding the buffer.
         * FIX: Use snprintf(nasm_output_file, sizeof(nasm_output_file), "%s.o", output_file)
         * or increase the buffer size to PATH_MAX + 3.
         */
        char nasm_output_file[40];
        char nasm_cmd[512];
        sprintf(nasm_output_file, "%s.o", output_file);
        if (compile_flags & COMPILE_PROCESS_EXPORT_AS_OBJECT)
        {
            /* Object-only mode: assemble without linking */
            sprintf(nasm_cmd, "nasm -f elf32 %s -o %s", output_file, nasm_output_file);
        }
        else
        {
            /* Full executable mode: assemble and link into a 32-bit ELF binary */
            sprintf(nasm_cmd, "nasm -f elf32 %s -o %s && gcc -m32 %s -o %s", output_file, nasm_output_file, nasm_output_file, output_file);
        }

        printf("%s", nasm_cmd);
        int res = system(nasm_cmd);
        if (res < 0)
        {
            printf("Issue assembling the assembly file with NASM and linking with gcc");
            return res;
        }

    }
    return 0;
}