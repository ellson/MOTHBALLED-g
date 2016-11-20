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
Usage: %s [-Vh?]\n\
       %s [-D<dom>][-o<file>][-T<fmt>][[-p][[-f<file>...]|[-s<skt>]|[-e<str>]]]\n\n\
", prog, prog);
    fprintf(stderr,"\
 -V          - Print version and exit\n\
 -h          - Print this help text and exit\n\
 -?          - Print this help text and exit\n\
 -D<dom>     - Select DOM where: <dom> = 'g'  - one time graph (default)\n\
                                 <dom> = 'G'  - persistent graph\n\
                                 <dom> = 'c'  - document\n\
                                 <dom> = 'f'  - filter\n\
 -o<file>    - output to file (default stdout)\n\
 -T<fmt>     - output format:  g, (more to come)\n\
 -p          - start new parallel thread for following inputs\n\
 -f<file>... - input from file(s).  Use '-' for stdin. (default stdin)\n\
 -e<str>     - input from string\n\
 -s<skt>     - input from socket\n\
");
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
