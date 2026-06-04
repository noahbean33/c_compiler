/**
 * @file buffer.c
 * @brief Dynamic character buffer implementation.
 *
 * Implements a growable byte buffer with automatic reallocation. Used for
 * building strings during lexing (identifiers, operators, numbers), formatted
 * assembly output during code generation, and preprocessor value construction.
 */

#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/** @brief Creates a new buffer with initial capacity of BUFFER_REALLOC_AMOUNT bytes. */
struct buffer* buffer_create()
{
    struct buffer* buf = calloc(sizeof(struct buffer), 1);
    buf->data = calloc(BUFFER_REALLOC_AMOUNT, 1);
    buf->len = 0;
    buf->msize = BUFFER_REALLOC_AMOUNT;
    return buf;
}

/** @brief Grows the buffer by the specified number of additional bytes. */
void buffer_extend(struct buffer* buffer, size_t size)
{
    buffer->data = realloc(buffer->data, buffer->msize+size);
    buffer->msize+=size;
}

/** @brief Ensures the buffer has enough capacity for 'size' additional bytes. */
void buffer_need(struct buffer* buffer, size_t size)
{
    if (buffer->msize <= (buffer->len+size))
    {
        size += BUFFER_REALLOC_AMOUNT;
        buffer_extend(buffer, size);
    }
}


/** @brief Writes formatted text to the buffer, including the null terminator in length. */
void buffer_printf(struct buffer* buffer, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int index = buffer->len;
    // Temporary, this is a limitation we are guessing the size is no more than 2048
    int len = 2048;
    buffer_extend(buffer, len);
    int actual_len = vsnprintf(&buffer->data[index], len, fmt, args);
    buffer->len += actual_len;
    va_end(args);
}

/** @brief Writes formatted text to the buffer, excluding the null terminator from length. */
void buffer_printf_no_terminator(struct buffer* buffer, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int index = buffer->len;
    // Temporary, this is a limitation we are guessing the size is no more than 2048
    int len = 2048;
    buffer_extend(buffer, len);
    int actual_len = vsnprintf(&buffer->data[index], len, fmt, args);
    buffer->len += actual_len-1;
    va_end(args);
}

/** @brief Appends a single character to the buffer, growing if necessary. */
void buffer_write(struct buffer* buffer, char c)
{
    buffer_need(buffer, sizeof(char));

    buffer->data[buffer->len] = c;
    buffer->len++;
}

/** @brief Returns a pointer to the raw buffer data. */
void* buffer_ptr(struct buffer* buffer)
{
    return buffer->data;
}

/** @brief Reads and returns the next byte from the buffer, advancing the read index. Returns -1 at end. */
char buffer_read(struct buffer* buffer)
{
    if (buffer->rindex >= buffer->len)
    {
        return -1;
    }
    char c = buffer->data[buffer->rindex];
    buffer->rindex++;
    return c;
}

/** @brief Peeks at the next byte without advancing the read index. Returns -1 at end. */
char buffer_peek(struct buffer* buffer)
{
    if (buffer->rindex >= buffer->len)
    {
        return -1;
    }
    char c = buffer->data[buffer->rindex];
    return c;
}

/** @brief Frees the buffer's data and the buffer struct itself. */
void buffer_free(struct buffer* buffer)
{
    free(buffer->data);
    free(buffer);
}