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

#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif

void hash_list(uint64_t *hash, elem_t *list);
void long_to_base64(char *b64string, const uint64_t *hash);
success_t base64_to_long(const char *b64string, uint64_t *hash);

#ifdef __cplusplus
}
#endif

#endif
