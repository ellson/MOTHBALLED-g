/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// #include "libg_session.h"

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "unknown"
#endif
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

// FIXME - add supports for (RESTful??) https access
//  e.g      -s<skt>   - input from socket\n\
//           [-p][[-f<fn>...]|[-s<skt>]|[-e<str>]]]\n\n\

static void usage(char *prog) {
    fprintf(stderr,"\
Usage: %s [-Vh?]\n\
       %s [<DOM_and_output>][<input_source_and_thread>]*\n\n\
", prog, prog);
fprintf(stderr,"\
where: <DOM_and_output>  is:\n\
       [-D<dom>][-o<fn>][-T<fmt>]\n\n\
  and: <input_source>\n\
       [-p][[-f<fn>...]|[-e<str>]]]\n\n\
 -V        - Print version and exit\n\
 -h        - Print this help text and exit\n\
 -?        - Print this help text and exit\n\
 -D<dom>   - Select DOM type:\n\
               <dom> = 'g'  - one time graph (default)\n\
               <dom> = 'G'  - persistent graph\n\
               <dom> = 'd'  - document\n\
               <dom> = '0'  - null dom for: translator, prettyprinter\n\
 -o<fn>    - output to file (default stdout)\n\
 -T<fmt>   - output format:  g, (more to come)\n\
 -t<tab>   - tabsize to use for output nesting (default 0)\n\
 -p        - start new parallel thread for following inputs\n\
 -f<fn>... - input from file(s).  Use '-' for stdin. (default stdin)\n\
 -e<str>   - input from string\n\
");
}

static void version(void) {
    fprintf(stderr,"Version: %s-%s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

// FIXME - move to libg_process. public api
static int
process_input(int thread, char inputsource, char *input, long sequence)
{
    printf("%d %c: %s\n", thread, inputsource, input);
    return 0;
}

// FIXME - move to libg_process. public api
static void
process_start_threads(void)
{
    printf("go Go GO\n");
}

int main(int argc, char *argv[])
{
    int opt, optnum, rc;

// defaults
    int thread = 0;
    char inputsource = 'f';
    char dom = 'g';
    char *outputfile = "-";
    char outputtype = 'g';
    int outputtab = 0;
    long sequence = 0;

    signal(SIGINT, intr);

// FIXME - reserce -s for socket ((is https a socket??)

    while ((opt = getopt(argc, argv, "-fepD:t:?hV")) != -1) {
        if (optarg) {
            optnum = atoi(optarg);
        } else {
            optnum = 0;
        }
        switch (opt) {
        case 1: 
            rc = process_input(thread, inputsource, optarg, sequence++);
            if (rc) {
                exit(EXIT_FAILURE);
            }
            break;
// FIXME - reserve:  case 's':
        case 'f':  
        case 'e':   
            inputsource = opt;
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
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
                break;
            }
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

    process_start_threads();

    exit(EXIT_SUCCESS);
}
