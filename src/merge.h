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

#ifndef MERGE_H
#define MERGE_H

#ifdef __cplusplus
extern "C" {
#endif

void merge(THREAD_t *THREAD, char *hash_A, char *hash_B);

#ifdef __cplusplus
}
#endif

#endif
