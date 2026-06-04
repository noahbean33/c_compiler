/**
 * @file stddef.c
 * @brief Static include handler for stddef-internal.h.
 *
 * Called when the compiler processes an include of stddef-internal.h.
 * Currently a placeholder for future native definitions related to
 * standard type definitions (size_t, ptrdiff_t, NULL, etc.).
 */

#include "compiler.h"

/** @brief Handler for stddef-internal.h inclusion. Currently no-op. */
void preprocessor_stddef_include(struct preprocessor* preprocessor, struct preprocessor_included_file* file)
{

}