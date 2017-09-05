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
 * In g, everything is an elem_t,  but we lose the benefit of compile-time structs.
 * This function attempts to document and enforce the structure of tuples.
 *
 * @param LIST context for list functions
 * @param schema, where schema[0] is the state_t for the resulting tuple
 * @param count of fields
 * @param ... elem_t*
 * @return an elem containing the specified field elems
 *
 */
elem_t * tuple(LIST_t *LIST, state_t schema[], size_t count, ...)
{
    va_list args;
    elem_t *tuple = new_list(LIST, schema[0]);

    va_start(args, count);
    for (int i = 1; i < count; i++) {
        elem_t *field = va_arg(args, elem_t*);
        if (field) { // allow fields to be null - is this what we want??
            assert(schema[i] == (state_t)(field->state));
            append_transfer(tuple, field);
        }
    }
    va_end(args);

    return tuple;
}
