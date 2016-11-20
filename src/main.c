/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "libg_session.h"

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
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

static void add_input(int thread, char inputtype, char *input)
{
    printf("%d %c: %s\n", thread, inputtype, input);
}

static void usage(char *prog) {
    fprintf(stderr,"\
Usage: %s [-h?V]\n\
       %s [[-p] [[-f <file> ... ] | [-s <socket>] | [-e <string>]]]\n\
", prog, prog);
}

static void version(void) {
    fprintf(stderr,"Version: %s\n", PACKAGE_VERSION);
}

int main(int argc, char *argv[])
{
    int opt, optnum;
    int thread = 0;
    char inputtype = 'f';

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "-fesph?V")) != -1) {
        switch (opt) {
        case 1: 
            add_input(thread, inputtype, optarg);
            break;
        case 'f':  
        case 'e':   
        case 's':    
            inputtype = opt;
            break;
        case 'p':    
            thread++;
            break;
        case 'V':    
            version();
            exit(EXIT_SUCCESS);
            break;
        case 'h':
        case '?':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;
        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
