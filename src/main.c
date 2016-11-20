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

static void usage(char *prog) {
    fprintf(stderr,"\
Usage: %s [-Vh?]\n\
       %s [-D<dom>][-o<file>][-T<fmt>][[-p][[-f<file>...]|[-s<skt>]|[-e<str>]]]\n\n\
", prog, prog);
    fprintf(stderr,"\
 -V          - Print version and exit\n\
 -h          - Print this help text and exit\n\
 -?          - Print this help text and exit\n\
 -D<dom>     - Select DOM type:
               <dom> = 'g'  - one time graph (default)\n\
               <dom> = 'G'  - persistent graph\n\
               <dom> = 'd'  - document\n\
               <dom> = '0'  - null dom for: translator, prettyprinter\n\
 -o<file>    - output to file (default stdout)\n\
 -T<fmt>     - output format:  g, (more to come)\n\
 -t<tab>     - tabsize to use for output nesting (default 0)\n\
 -p          - start new parallel thread for following inputs\n\
 -f<file>... - input from file(s).  Use '-' for stdin. (default stdin)\n\
 -e<str>     - input from string\n\
 -s<skt>     - input from socket\n\
");
}

static void version(void) {
    fprintf(stderr,"Version: %s\n", PACKAGE_VERSION);
}

static void add_input(int thread, char inputsource, char *input)
{
    printf("%d %c: %s\n", thread, inputsource, input);
}

int main(int argc, char *argv[])
{
    int opt, optnum;

// defaults
    int thread = 0;
    char inputsource = 'f';
    char dom = "g";
    char outputfile = "-";
    char outputtype = "g";
    int outputtab = 0;

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "-fespD:t:?hV")) != -1) {
        if (optarg) {
            optnum = atoi(optarg);
        } else {
            optnum = 0;
        }
        switch (opt) {
        case 1: 
            add_input(thread, inputsource, optarg);
            break;
        case 'f':  
        case 'e':   
        case 's':    
            inputsource = opt;
            break;
        case 'p':    
            thread++;
            break;
        case 'D':    
            switch (optarg) {
            case 'g':
            case 'G':
            case 'd':
            case '0':
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