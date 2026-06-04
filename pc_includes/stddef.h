/**
 * @file stddef.h
 * @brief Standard definitions for the Peach C Compiler runtime.
 *
 * Provides the offsetof macro which calculates the byte offset of a
 * structure member from the beginning of the structure.
 */

#ifndef STDDEF_H
#define STDDEF_H

#include "stddef-internal.h"

/* Calculates the byte offset of MEMBER within TYPE by casting NULL to TYPE* */
#define offsetof(TYPE, MEMBER) &((TYPE*)0x00)->MEMBER
#endif