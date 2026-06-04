/**
 * @file datatype.c
 * @brief Datatype query and manipulation utilities.
 *
 * Provides functions for determining datatype properties (size, pointer depth,
 * struct/union classification) and for constructing special datatypes used
 * during code generation (numeric literals, string literals).
 */

#include "compiler.h"

/** @brief Returns true if the datatype is a struct or union. */
bool datatype_is_struct_or_union(struct datatype* dtype)
{
    return dtype->type == DATA_TYPE_STRUCT || dtype->type == DATA_TYPE_UNION;
}

/** @brief Returns true if the given type name string is "struct" or "union". */
bool datatype_is_struct_or_union_for_name(const char* name)
{
    return S_EQ(name,"union") || S_EQ(name, "struct");
}

/**
 * @brief Returns the element size appropriate for array index access.
 * Accounts for struct pointers where pointer depth is exactly 1.
 */
size_t datatype_size_for_array_access(struct datatype* dtype)
{
    if (datatype_is_struct_or_union(dtype) && dtype->flags & DATATYPE_FLAG_IS_POINTER && 
        dtype->pointer_depth == 1)
    {
        // struct abc* abc; abc[0];
        return dtype->size;
    }

    return datatype_size(dtype);
}

/** @brief Returns true if the datatype is void and not a pointer. */
bool datatype_is_void_no_ptr(struct datatype* dtype)
{
    return S_EQ(dtype->type_str, "void") && !(dtype->flags & DATATYPE_FLAG_IS_POINTER);
}

/** @brief Sets a datatype to void with zero size. */
void datatype_set_void(struct datatype* dtype)
{
    dtype->type = DATA_TYPE_VOID;
    dtype->type_str = "void";
    dtype->size = 0;
}
/** @brief Returns the size of a single element (DWORD for pointers, raw size otherwise). */
size_t datatype_element_size(struct datatype* dtype)
{
    if (dtype->flags & DATATYPE_FLAG_IS_POINTER)
    {
        return DATA_SIZE_DWORD;
    }

    return dtype->size;
}

/** @brief Returns the datatype size ignoring pointer indirection, but respecting arrays. */
size_t datatype_size_no_ptr(struct datatype* dtype)
{
    if (dtype->flags & DATATYPE_FLAG_IS_ARRAY)
    {
        return dtype->array.size;
    }

    return dtype->size;
}

/** @brief Returns the effective size of the datatype (DWORD for pointers, array size for arrays). */
size_t datatype_size(struct datatype* dtype)
{
    if (dtype->flags & DATATYPE_FLAG_IS_POINTER && dtype->pointer_depth > 0)
    {
        return DATA_SIZE_DWORD;
    }

    if (dtype->flags & DATATYPE_FLAG_IS_ARRAY)
    {
        return dtype->array.size;
    }

    return dtype->size;
}

/** @brief Returns true if the datatype is a primitive (not struct/union). */
bool datatype_is_primitive(struct datatype* dtype)
{
    return !datatype_is_struct_or_union(dtype);
}

/** @brief Returns true if the datatype is a struct/union value type (not a pointer). */
bool datatype_is_struct_or_union_non_pointer(struct datatype* dtype)
{
    return dtype->type != DATA_TYPE_UNKNOWN && !datatype_is_primitive(dtype) && !(dtype->flags & DATATYPE_FLAG_IS_POINTER);  
}
