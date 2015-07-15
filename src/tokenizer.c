#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NUL=0,           // EOF
    ABC=1<<0,        // simple_string_character
     WS=1<<1,        // whitespace
     NL=1<<2&WS,     // newline
     CR=1<<3&WS,     // return
     QT=1<<4,
    DQT=1<<5&QT,     // '"'
    SQT=1<<6&QT,     // '''
   OPEN=1<<7,
  CLOSE=1<<8,
    LPN=1<<9&OPEN,   // '('
    RPN=1<<10&CLOSE, // ')'
    LAN=1<<11&OPEN,  // '<'
    RAN=1<<12&CLOSE, // '>'
    LBT=1<<13&OPEN,  // '['
    RBT=1<<14&CLOSE, // ']'
    LBE=1<<15&OPEN,  // '{'
    RBE=1<<16&CLOSE, // '}'
    FSL=1<<17,       // '/'
    BSL=1<<18,       // '\'
    OCT=1<<18,       // '#'
    AST=1<<20,       // '*'
    CLN=1<<19,       // ':'
    SCN=1<<21,       // ';'
    EQL=1<<22,       // '='
    TLD=1<<23,       // '~'
 DQTABC=ABC&WS&LPN&RPN&LAN&RAN&LBT&RBT&LBE&RBE&FSL&OCT&AST&CLN&SCN&EQL&TLD&SQT,
 SQTABC=ABC&WS&LPN&RPN&LAN&RAN&LBT&RBT&LBE&RBE&FSL&OCT&AST&CLN&SCN&EQL&TLD&DQT,
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

typedef struct elem_s elem_t;

struct elem_s {
    charclass_t charclass;
    void *beg;
    int len;
    int allocated;
    elem_t *next;
};

typedef struct {
    charclass_t fraglisttype, fragtermtype;
    elem_t *fraglist, *frag;
} act_t;

static void process_fraglist (act_t *act) {
    fprintf(stdout,"process fraglist\n");
}

unsigned char *test=(unsigned char*)"<aa bb cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp, c;
    act_t *act;

    act = calloc(sizeof(act_t), 1);

    inp = test;
    while ((c = *inp)) {
        charclass_t charclass;
	elem_t *frag = act->frag;

        charclass = char2charclass[c];
        if (frag) {
            if (charclass & frag->charclass) {
                frag->len++;
            }
	    else {
#if 0
                fragtermtype = charclass2fragtermtype[charclass];
            
                fraglisttype = charclass2fraglisttype[charclass];
		if (fraglisttype == act->fraglisttype) {
 		    firstcharoffset = charclass2firstcharoffset[charclass];
		    if (firstcharoffset == 0) {
                        frag->len++;
                    }
                    else {
			frag = calloc(sizeof(elem_t), 1);
                        frag->type = charclass;
                        frag->len = 0;
			frag->beg = inp + 1;
			act->frag->next = frag;
			act->frag = frag;
                    }
#endif
                }
 		else {
process_fraglist(act);
		}
            }
        }
        else {
            fraglisttype = charclass2fraglisttype[charclass];
	    if (act->fraglisttype != NUL) {
                frag = calloc(sizeof(elem_t), 1);
                frag->type = charclass;
 		firstcharoffset = charclass2firstcharoffset[charclass];
                frag->len = 1 - firstcharoffset;
                frag->beg = inp + firstcharoffset;
                frag->next = NULL;
                act->frag = frag;
                act->fraglist = frag;
                act->fraglisttype = fraglisttype;
            }
	}
        inp++;
    }
}
