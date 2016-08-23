/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>

#include "libje_private.h"

#define OUT stdout
#define ERR stderr

#define styleLAN sstyle?"< ":"<"
#define styleRAN sstyle?"\n>\n":">"
#define styleLPN sstyle?"\n  ( ":"("
#define styleRPN sstyle?"\n  )\n":")"
#define styleLBR sstyle?"\n  [ ":"["
#define styleRBR sstyle?"\n  ]\n":"]"
#define styleLBE sstyle?"\n  { ":"{"
#define styleRBE sstyle?"\n  }\n":"}"

static int sstyle = 0;

void set_sstyle(void)
{
    sstyle = 1;
}

static state_t oleg1 = sizeof_state_machine, oleg2 = sizeof_state_machine;
static unsigned char *sleg1, *sleg2;

static unsigned char SAME[] = { 1, '=' };

static void print_next(state_t leg1, state_t leg2)
{
    if (leg1 != oleg1) {
        oleg1 = leg1;
        sleg1 = NAMEP(leg1);

        oleg2 = sizeof_state_machine;
    } else {
        sleg1 = SAME;
    }
    if (leg2 != oleg1) {
        oleg2 = leg2;
        sleg2 = NAMEP(leg2);
    } else {
        sleg2 = SAME;
    }
    fprintf(OUT, "%s", styleLAN);
    print_len_frag(OUT, sleg1);
    putc(' ', OUT);
    print_len_frag(OUT, sleg2);
    fprintf(OUT, "%s", styleRAN);
}

static void print_attr(char attr, char *attrid, int *inlist)
{
    if (attr) {
        if ((*inlist)++)
            putc(' ', OUT);
        fprintf(OUT, "%s", attrid);
    }
}

static void print_prop(unsigned char prop)
{
    int inlist;

    if (prop & (ALT | OPT | SREP | REP)) {
        inlist = 0;
        fprintf(OUT, "%s", styleLBR);
        print_attr(prop & ALT, "ALT", &inlist);
        print_attr(prop & OPT, "OPT", &inlist);
        print_attr(prop & SREP, "SREP", &inlist);
        print_attr(prop & REP, "REP", &inlist);
        fprintf(OUT, "%s", styleRBR);
    }
}

static void printg_r(state_t si, int indent)
{
    char so;        // state offset, signed
    state_t ti, ni;
    int i;

    ti = si;        // index within alts or sequrnce
    while ((so = state_machine[ti])) {
        ni = ti + so;
        for (i = indent; i--;)
            putc(' ', OUT);
        print_next(si, ni);
        print_prop(state_props[ti]);
        fprintf(OUT, "%s\n", styleLBE);
        if (ni != CONTENTS) {    // stop recursion
            printg_r(ni, indent + 2);
        }
        for (i = indent; i--;)
            putc(' ', OUT);
        fprintf(OUT, "%s\n", styleRBE);

        ti++;
    }
}

// recursively walk the grammar - tests all possible transitions
void printg(void)
{
    printg_r(CONTENTS, 0);
}

static void print_chars(state_t si)
{
    int i, cnt;

    cnt = 0;
    for (i = 0; i < 0x100; i++) {
        if (si == char2state[i]) {
            if (cnt++) {
                putc(' ', OUT);
            } else {
                fprintf(OUT, "%s", styleLBE);
            }
            fprintf(OUT, "%02x", i);
        }
    }
    if (cnt++) {
        fprintf(OUT, "%s", styleRBE);
    }
}

// just dump the grammar linearly,  should result in same
// logical graph as printg()
void dumpg(void)
{
    char so;
    state_t si, ni, ti;

    si = CONTENTS;
    while (si < sizeof_state_machine) {
        if (state_machine[si]) {    // non-terminal
            ti = si;
            while ((so = state_machine[ti])) {
                ni = ti + so;
                print_next(si, ni);
                print_prop(state_props[ti]);
                ti++;
            }
            si = ti;
        } else {    // else terminal
            print_len_frag(OUT, NAMEP(si));
            print_chars(si);
        }
        putc('\n', OUT);
        si++;
    }
}
