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
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#if 0
#include "libg_session.h"
#else
// Dummy API for testing main()
#define PACKAGE_NAME "g-fake"
#define PACKAGE_VERSION "1234-1"
static void process_version(void) {
    fprintf(stderr,"%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
}
static void process_input(int thread, char type, char *input, long sequence)
{
    printf("inp: -%c\"%s\"`%ld par: %d\n", type, input, sequence, thread);
}
static void process_output(char *output, char *layout, char* format)
{
    printf("out: -o\"%s\" -K%s -T%s\n", output, layout, format);
}
static void process_dom_start(char dom)
{
    printf("dom: -D%c\n", dom);
}
#endif


// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

#if 0
// FIXME - once we know what we  really want from command line interrupts
    interrupt(THREAD);
#endif
    exit (EXIT_FAILURE);
}

static void usage(char *prog) {
    fprintf(stderr,"\n\
Usage: %s [-vh]\n\
       %s [<source>]* [<dest>]\n\
\n\
where <source> is:\n\
       [-p] [[-e<str>] | [-f<fn>...] | [-s<skt>]]\n\
\n\
and where <dest> is:\n\
       [-D<dom>][-T<fmt>][K<lay>][-o<fn>\n\
\n\
 -v        - Print version and exit\n\
 -h        - Print this help text and exit\n\
\n\
 -p        - start new parallel thread for following inputs\n\
 -e<str>   - input from string\n\
 -f<fn>... - input from file(s).  Use '-' for stdin. (default stdin)\n\
 -s<skt>   - input from socket\n\
\n\
 -D<dom>   - Select DOM type:\n\
               <dom> = 'g'  - one time graph (default)\n\
               <dom> = 'G'  - persistent graph\n\
               <dom> = 'd'  - document (retains ordering and duplicates)\n\
               <dom> = '0'  - null dom for: translator, prettyprinter\n\
 -T<fmt>   - output format:  g (default),  or any supported by dot\n\
 -K<lay>   - output layout:  none (default for -Tg or -Tgv),\n\
                             or any layout supported by dot\n\
 -o<fn>    - output to file (default stdout)\n\
\n", prog, prog);
}

int main(int argc, char *argv[])
{
    int opt, optnum, rc;

    char multiopt = 'f';
    int inpcnt = 0;
    int outcnt = 0;

// initial source values (defaults)
    int thread = 0;
    long sequence = 0;

// initial dest values (defaults)
    char dom = 'g';
    char *layout = "";
    char *format = "g";

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "-efspK:T:D:ohv")) != -1) {
        if (optarg) {
            optnum = atoi(optarg);
        } else {
            optnum = 0;
        }
        switch (opt) {
        case 1: 
            switch (multiopt) {
            case 's':
            case 'f':  
            case 'e':   
                process_input(thread, multiopt, optarg, sequence++);
                inpcnt++;
                break;
            case 'o':    
                process_output(optarg, layout, format);
                outcnt++;
                break;
            default:
                assert(0);
                break;
            }
            break;
        case 's':
        case 'f':  
        case 'e':   
        case 'o':    
            multiopt = opt;
            break;
        case 'p':    
            thread++;
            break;
        case 'D':    
            switch (*optarg) {
            case 'g':
            case 'G':
            case 'd':
            case '0':
                dom = *optarg;
                break;
            default:

                fprintf(stderr,"%s: invalid argument to -D -- \'%c\'\n",
                        argv[0], *optarg);
                exit(EXIT_FAILURE);
                break;
            }
            break;
        case 'v':    
            process_version();
            exit(EXIT_SUCCESS);
            break;
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;
        case '?':
        default:
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (inpcnt + outcnt) {
        thread = malloc((inpcnt + outcnt) * sizeof(pthread_t));
    }
    for (i - 0; i < outcount; i++) {
        if ( (rc=pthread_create( &thread[i], NULL,
                        &process_output, NULL)) ) {
            printf("Thread creation failed: %d\n", rc1);
        }
    }
    for (i - 0; i < inpcnt; i++) {
        if ( (rc=pthread_create( &thread[outcount + i], NULL,
                        &process_input, NULL)) ) {
            printf("Thread creation failed: %d\n", rc1);
        }
    }
#if 0
    if (!inpcnt) {
        (void)process_input(thread, 'f', "-", sequence++);
    }
    if (!outcnt) {
        (void)process_output("-", "", "g");
    }
    process_dom_start(dom);
#endif

    for (i - 0; i < (inpcnt + outcount); i++) {
       pthread_join( thread[i], NULL);
    }
    exit(EXIT_SUCCESS);
}
