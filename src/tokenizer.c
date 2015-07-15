#include <stdio.h>
#include <stdlib.h>

#include "list.h"

typedef enum {
    NUL=0,           // EOF
    ABC=1<<0,        // simple_string_character
     WS=1<<1,        // whitespace
     NL=1<<2,        // newline
     CR=1<<3,        // return
    DQT=1<<4,        // '"'
    SQT=1<<5,        // '''
    LPN=1<<6,        // '('
    RPN=1<<7,        // ')'
    LAN=1<<8,        // '<'
    RAN=1<<9,        // '>'
    LBT=1<<10,       // '['
    RBT=1<<11,       // ']'
    LBE=1<<12,       // '{'
    RBE=1<<13,       // '}'
    FSL=1<<14,       // '/'
    BSL=1<<15,       // '\'
    OCT=1<<16,       // '#'
    AST=1<<17,       // '*'
    CLN=1<<18,       // ':'
    SCN=1<<19,       // ';'
    EQL=1<<20,       // '='
    TLD=1<<21,       // '~'
} charclass_t;
 
charclass_t char2charclass[] = {
   NUL, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
   ABC,  WS,  NL, ABC, ABC,  CR, ABC, ABC,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* CAN  EM SUB ESC  FS  GS  RS  US */
    WS, ABC, DQT, OCT, ABC, ABC, ABC, SQT,  /* SPC  !   "   #   $   %   &   '  */
   LPN, RPN, AST, ABC, ABC, ABC, ABC, FSL,  /*  (   )   *   +   ,   -   .   /  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  0   1   2   3   4   5   6   7  */
   ABC, ABC, CLN, SCN, LAN, EQL, RAN, ABC,  /*  8   9   :   ;   <   =   >   ?  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  @   A   B   C   D   E   F   G  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  H   I   J   K   L   M   N   O  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  P   Q   R   S   T   U   V   W  */
   ABC, ABC, ABC, LBT, BSL, RBT, ABC, ABC,  /*  X   Y   Z   [   \   ]   ^   _  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  `   a   b   c   d   e   f   g  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  h   i   j   k   l   m   n   o  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  p   q   r   s   t   u   v   w  */
   ABC, ABC, ABC, LBE, ABC, RBE, TLD, ABC,  /*  x   y   z   {   |   }   ~  DEL */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC
};

typedef struct {
    charclass_t fraglistcharclass;
    elem_t *fraglist, *frag;
} act_t;

unsigned char *test=(unsigned char*)"<aa bb cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp, c;
    act_t *act;

    act = calloc(sizeof(act_t), 1);

    inp = test;
    while ((c = *inp)) {
        charclass_t charclass;
	elem_t *frag, *elem;

        charclass = char2charclass[c];
        frag = act->frag;
        if (frag) {    // a frag already exists
            if (charclass & frag->type) {  // identical charclass, just continue appending to frag
                frag->len++;
            }
	    else {
                if (charclass & act->fraglistcharclass) {  // if matches fraglist charclass
 		    if (charclass & (ABC|WS|NL|CR)) {  // charclass that can be simply appended
                        frag->len++;
                    }
                    else { // DQT|SQT|BSL  -- charclass that start at next char - so new frag to skip this char
			frag = newelem(charclass, inp+1, 0, 0, NULL);
			act->frag->next = frag;
			act->frag = frag;
                    }
                }
 		else {
		    if (act->fraglist->type & (WS|NL|CR)) {
			freelist(act->fraglist);
		    }
                    else if (act->fraglist->type & (ABC|DQT|SQT|FSL)) {
			elem = joinlist2elem(act->fraglist, 1, 0);
fprintf(stdout,"%s\n",elem->buf);
                    }
                    act->frag = NULL;
		}
            }
        }

        if (charclass & (LPN|LAN|LBT|LBE)) { // charclass that need a frag
              // FIXME
        }

        frag = act->frag;
        if (!frag) { // no current frag
            if (charclass & (ABC|WS|NL|CR|DQT|SQT|FSL)) { // charclass that need a frag
 		if (charclass & (ABC|WS|NL|CR)) { // charclass that start at this char
		    frag = newelem(charclass, inp, 1, 0, NULL);
                }
                else { // DQT|SQT|BSL     --   charclass that start at next char
		    frag = newelem(charclass, inp+1, 0, 0, NULL);
                }
                act->frag = frag;
                act->fraglist = frag;
                switch (charclass) {  // set fraglistcharclass with all classes accepted in fraglist
		case ABC:
		    act->fraglistcharclass = ABC;
		    break;
		case WS:
		case NL:
		case CR:
		    act->fraglistcharclass = WS|NL|CR;
		    break;
		case DQT:
		    act->fraglistcharclass = ABC|WS|NL|CR|LPN|RPN|LAN|RAN|LBT|RBT|LBE|RBE|FSL|OCT|AST|CLN|SCN|EQL|TLD|SQT;
		    break;
		case SQT:
		    act->fraglistcharclass = ABC|WS|LPN|RPN|LAN|RAN|LBT|RBT|LBE|RBE|FSL|OCT|AST|CLN|SCN|EQL|TLD|DQT;
		    break;
		case BSL:
		    act->fraglistcharclass = ABC|WS|LPN|RPN|LAN|RAN|LBT|RBT|LBE|RBE|FSL|OCT|AST|CLN|SCN|EQL|TLD|DQT|SQT|FSL;
		    break;
                default:
		    fprintf(stderr,"shouldn't get here\n");
                }
            }
	}
        inp++;
    }
}
