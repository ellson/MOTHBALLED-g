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

#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

struct token_s {
    IO_t IO;                   // IO context. Maybe cast from TOKEN

    state_t insi;              // state represented by last character read
    state_t state;             // last state entered
    state_t elem_has_ast;      // flag set if an '*' is found in any elem
                               //   -- reset by parse(), so parse defines "elem"
    state_t quote_type;        // DQT, or LPN, LBE, LAN, LBR
    int quote_state;           // 0 not in quotes
                               // 1 between DQT
                               // 2 char following BSL between DQT
    int quote_nest;            // paren nesting level, or tranparent char count
    int quote_length;          // length of transparent string

    long stat_infragcount;     // various stats
    long stat_instringshort;
    long stat_instringlong;
};

void token_error(TOKEN_t * TOKEN, char *message, state_t si);
success_t token_whitespace(TOKEN_t * TOKEN);
void token_pack_string(TOKEN_t *TOKEN, int slen, elem_t *string);

#ifdef __cplusplus
}
#endif

#endif
