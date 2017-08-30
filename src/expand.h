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

#ifndef EXPAND_H
#define EXPAND_H

#ifdef __cplusplus
extern "C" {
#endif

void expand(CONTAINER_t * CONTAINER, elem_t *elem, elem_t *nodes, elem_t *edges);

#ifdef __cplusplus
}
#endif

#endif
