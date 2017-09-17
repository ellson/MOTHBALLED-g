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
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "merge.h"

/** 
 * Merge two sets of content, identified by hash
 *
 * @param THREAD context
 * @param hash_A - first content
 * @param hash_B - second content
 */
void merge(THREAD_t *THREAD, char *hash_A, char *hash_B)
{
    PROCESS_t *PROCESS = THREAD->PROCESS;

    assert(hash_A);
    assert(hash_B);

    int lenA = strlen(hash_A);
    int lenB = strlen(hash_B);

    assert(lenA && lenA == lenB);

    char *merge_hash = malloc(lenA + lenB + 1);
    if (!merge_hash)
        FATAL("malloc()")

    char *a = hash_A, *b = hash_B, *c = merge_hash;
    while (*a) {
        *c++ = *a++;
        *c++ = *b++;
    }
    *c = '\0';

    char *contenthash = NULL;
    elem_t *key = new_list(LIST(), NODE);
    elem_t *attr = new_list(LIST(), NODE);
    append_transfer(key, attr);
    elem_t *elem = new_shortstr(LIST(), NODE, merge_hash);
    append_transfer(attr, elem);
    elem_t *p = search_item(PROCESS->merge_cache, key->u.l.first);
    if (p) {
        // FIXME - Really?!!
        contenthash = (char*)p->u.t.key->u.l.next->u.l.last->u.s.str;
    } 
    else {
        // do merge
        elem_t *value = new_list(LIST(), NODE);
        append_transfer(key, value);
        contenthash = "cool!";
        elem = new_shortstr(LIST(), NODE, contenthash);
        append_transfer(value, elem);
        PROCESS->merge_cache =
            insert_item(LIST(), PROCESS->merge_cache, 
                    key->u.l.first, NULL, NULL);
    }
fprintf(stderr, "%s\n", contenthash);
    free_list(LIST(), key);
}
