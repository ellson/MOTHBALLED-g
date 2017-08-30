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

#ifndef TREE_H
#define TREE_H

#ifdef __cplusplus
extern "C" {
#endif

elem_t * search_item(elem_t * p, elem_t * key);
elem_t * insert_item(LIST_t * LIST, elem_t * p, elem_t *key,
             elem_t * (*merge)(LIST_t* LIST, elem_t *key, elem_t *oldkey),
             elem_t ** newkey );
elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key);

#ifdef __cplusplus
}
#endif

#endif
