/**
 * @file buffer.h
 * @brief Dynamic character buffer interface.
 *
 * Provides a growable byte buffer used extensively throughout the compiler
 * for accumulating strings during lexical analysis, building formatted
 * assembly output, and constructing preprocessor values. Supports both
 * sequential writing and formatted printf-style output.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

/** Number of bytes to allocate when the buffer needs to grow. */
#define BUFFER_REALLOC_AMOUNT 2000

struct buffer
{
    char* data;       /* Pointer to the underlying byte array */
    int rindex;       /* Current read index for sequential reading */
    int len;          /* Number of bytes currently written */
    int msize;        /* Total allocated memory size */
};

/** @brief Allocates and returns a new empty buffer. */
struct buffer* buffer_create();

/** @brief Reads and returns the next byte, advancing the read index. */
char buffer_read(struct buffer* buffer);
/** @brief Peeks at the next byte without advancing the read index. */
char buffer_peek(struct buffer* buffer);

/** @brief Extends the buffer capacity by the specified number of bytes. */
void buffer_extend(struct buffer* buffer, size_t size);
/** @brief Writes formatted text into the buffer (includes null terminator in length). */
void buffer_printf(struct buffer* buffer, const char* fmt, ...);
/** @brief Writes formatted text into the buffer (excludes null terminator from length). */
void buffer_printf_no_terminator(struct buffer* buffer, const char* fmt, ...);
/** @brief Writes a single byte to the buffer. */
void buffer_write(struct buffer* buffer, char c);
/** @brief Returns a pointer to the buffer's raw data. */
void* buffer_ptr(struct buffer* buffer);
/** @brief Frees the buffer and its underlying data. */
void buffer_free(struct buffer* buffer);


#endif