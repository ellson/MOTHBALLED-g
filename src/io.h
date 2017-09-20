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

#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#define SIZEOFHASH 128
#define SUFFICIENTHASH 6

struct io_s {
    LIST_t LIST;               // LIST context. Maybe cast from TOKEN

    // FIXME - Why do we have these ?  Why not leave in PROCESS ?
    int *pargc;                // remaining filenames from command line
    char **argv;
    char *acts;                // g snippet from command line

    int flags;

    unsigned char buf[1024];   // output buffering
    int pos;

    void *out_chan;               // output FILE* or ikea_box_t*
    out_disc_t *out_disc;

    char contenthash[SIZEOFHASH]; // big enough for content hash
                               // checked by assert in ikea_box_open()

    FILE *out, *err;           // output files

    unsigned char *in;         // next character to be processed
    unsigned char *end;        // one past the last character
    char *filename;            // name of file currently being processed,
                               //   or "-" for stdin
    FILE *file;                // file handle of file being processed
    char *membuf;              // An externally memory-managed, in-memory,
                               // NUL-terminated string providing ACT(s),
                               // e.g. following -e on the commandline
   
    long linecount_at_start;   // line count when this file was opened.
                               //   -- used to calculate line # within file

    long stat_lfcount;         // various stats
    long stat_crcount;
    long stat_incharcount;
    long stat_infilecount;
};

success_t input(IO_t *IO);

out_disc_t stdout_disc;
out_disc_t file_disc;

#ifdef __cplusplus
}
#endif

#endif
