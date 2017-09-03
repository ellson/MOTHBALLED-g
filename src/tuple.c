/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "tuple.h"

/**
 * Create a new elem_t containing a tuple of elem_t
 *
 * In g, everything is an elem_t,  but we lose the benerfit of compile-time structs.
 * This function attempts to document and enforce the structure of tuples.
 *
 * @param LIST context for list functions
 * @param type of the returned elem_t
 * @param count of fields
 * @param ... some: struct{state_t; elem_t*;}   The last elem must be {0}
 * @return an elem containing the specified field elems
 *
 */
elem_t * tuple(LIST_t *LIST, state_t type, size_t count, ...)
{
    va_list args;
    field_t field;
    elem_t *tuple = new_list(LIST, type);

    va_start(args, count);
    while (count--) {
        field = va_arg(args, field_t);
        assert(field.type == (state_t)(field.elem->type));
        append_addref(tuple, field.elem);
    }
    va_end(args);

    return tuple;
}
