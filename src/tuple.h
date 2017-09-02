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

#ifndef TUPLE_H
#define TUPLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {state_t type; elem_t* elem;} field_t;

elem_t * tuple(LIST_t* LIST, state_t type, size_t count, ...);

#ifdef __cplusplus
}
#endif

#endif
