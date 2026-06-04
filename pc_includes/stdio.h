/**
 * @file stdio.h
 * @brief Standard I/O declarations for the Peach C Compiler runtime.
 *
 * Provides the FILE structure definition and declarations for common
 * standard I/O functions (fopen, fwrite, fread, fclose, printf).
 * These serve as forward declarations that allow compiled programs
 * to link against the system's C library.
 */

#ifndef STDIO_H
#define STDIO_H
#include <stdlib.h>

/* Internal file stream buffer structure (mirrors CRT implementation) */
typedef struct _iobuf
{
    char *_ptr;
    int _cnt;
    char *_base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char *_tmpfname;
} FILE;

/* Standard I/O function declarations */
FILE* fopen(const char* filename, const char* mode);
size_t fwrite(const char* ptr, size_t size, size_t nmemb, FILE* stream);
int fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
int printf(const char* fmt, ...);
#endif