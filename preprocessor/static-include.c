/**
 * @file static-include.c
 * @brief Static include handler dispatch.
 *
 * Maps internal header filenames to their corresponding handler functions.
 * Static includes are headers that don't exist as actual files but instead
 * register native functions or preprocessor definitions programmatically
 * when "included" by user code.
 */

#include "compiler.h"

void preprocessor_stddef_include(struct preprocessor* preprocessor, struct preprocessor_included_file* file);
void preprocessor_stdarg_internal_include(struct preprocessor* preprocessor, struct preprocessor_included_file* file);

/**
 * @brief Returns the post-creation handler for a given static include filename.
 * @return Function pointer to the handler, or NULL if not a static include.
 */
PREPROCESSOR_STATIC_INCLUDE_HANDLER_POST_CREATION preprocessor_static_include_handler_for(const char* filename)
{
    if (S_EQ(filename, "stddef-internal.h"))
    {
        return preprocessor_stddef_include;
    }
    else if(S_EQ(filename, "stdarg-internal.h"))
    {
        return preprocessor_stdarg_internal_include;
    }

    return NULL;
}