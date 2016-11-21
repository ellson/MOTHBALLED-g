/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
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

static void version(void) {
    fprintf(stderr,"%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

/**************** FIXME - move to libg_process public api ***/
 
static int
process_input(int thread, char type, char *input, long sequence)
{
    printf("inp: -%c\"%s\"`%ld par: %d\n", type, input, sequence, thread);
    return 0;
}

static int
process_output(char *output, char *layout, char* format)
{
    printf("out: -o\"%s\" -K%s -T%s\n", output, layout, format);
    return 0;
}

static void
process_dom_start(char dom)
{
    printf("dom: -D%c\n", dom);
}
/***********************************************************/

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
                rc = process_input(thread, multiopt, optarg, sequence++);
                if (rc) {
                    exit(EXIT_FAILURE);
                }
                inpcnt++;
                break;
            case 'o':    
                rc = process_output(optarg, layout, format);
                if (rc) {
                    exit(EXIT_FAILURE);
                }
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
            version();
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

    if (!inpcnt) {
        (void)process_input(thread, 'f', "-", sequence++);
    }
    if (!outcnt) {
        (void)process_output("-", "", "g");
    }
    process_dom_start(dom);

    exit(EXIT_SUCCESS);
}
