#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NUL,	// EOF
    ABC,	// simple_string_character
     WS,	// whitespace
     NL,  CR,   // newline return
    DQT, SQT,   // '"'  '''
    LPN, RPN,   // '('  ')'
    LAN, RAN,   // '<'  '>'
    LBT, RBT,   // '['  ']'
    LBE, RBE,   // '{'  '}'
    FSL, BSL,   // '/'  '\' 
    OCT, AST,   // '#'  '*'
    CLN, SCN,   // ':'  ';'
    EQL, TLD    // '='  '~'
} fragtype_t;
 
fragtype_t char2fragtype[] = {
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

fragtype_t fragtype2fraglisttype[] = {
    NUL,      // NUL,        // EOF
    ABC,      // ABC,        // simple_string_character
     WS,      //  WS,        // whitespace
     WS,  WS, //  NL,  CR,   // newline return
    DQT, SQT, // DQT, SQT,   // '"'  '''
    NUL, NUL, // LPN, RPN,   // '('  ')'
    NUL, NUL, // LAN, RAN,   // '<'  '>'
    NUL, NUL, // LBT, RBT,   // '['  ']'
    NUL, NUL, // LBE, RBE,   // '{'  '}'
    NUL, ABC, // FSL, BSL,   // '/'  '\' 
    NUL, ABC, // OCT, AST,   // '#'  '*'
    NUL, NUL, // CLN, SCN,   // ':'  ';'
    NUL, NUL, // EQL, TLD    // '='  '~'
};

int fragtype2firstcharoffset[] = {
    0,    // NUL,        // EOF
    0,    // ABC,        // simple_string_character
    0,    //  WS,        // whitespace
    0, 0, //  NL,  CR,   // newline return
    1, 1, // DQT, SQT,   // '"'  '''
    0, 0, // LPN, RPN,   // '('  ')'
    0, 0, // LAN, RAN,   // '<'  '>'
    0, 0, // LBT, RBT,   // '['  ']'
    0, 0, // LBE, RBE,   // '{'  '}'
    0, 1, // FSL, BSL,   // '/'  '\' 
    0, 0, // OCT, AST,   // '#'  '*'
    0, 0, // CLN, SCN,   // ':'  ';'
    0, 0, // EQL, TLD    // '='  '~'
};

fragtype_t fragtype2fragtermtype[] = {
    NUL,      // NUL,        // EOF
    NUL,      // ABC,        // simple_string_character
    NUL,      //  WS,        // whitespace
    NUL, NUL, //  NL,  CR,   // newline return
    DQT, SQT, // DQT, SQT,   // '"'  '''
    RPN, NUL, // LPN, RPN,   // '('  ')'
    RAN, NUL, // LAN, RAN,   // '<'  '>'
    RBT, NUL, // LBT, RBT,   // '['  ']'
    RBE, NUL, // LBE, RBE,   // '{'  '}'
    NUL, NUL, // FSL, BSL,   // '/'  '\' 
    NUL, NUL, // OCT, AST,   // '#'  '*'
    NUL, NUL, // CLN, SCN,   // ':'  ';'
    NUL, NUL, // EQL, TLD    // '='  '~'
};

typedef struct elem_s elem_t;

struct elem_s {
    fragtype_t type;
    void *beg;
    int len;
    int allocated;
    elem_t *next;
};

typedef struct {
    fragtype_t fraglisttype, fragtermtype;
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
        fragtype_t fragtype, fraglisttype, fragtermtype;
        int firstcharoffset;
	elem_t *frag = act->frag;

        fragtype = char2fragtype[c];
        if (frag) {
            if (fragtype == frag->type) {
                frag->len++;
            }
	    else {
                fragtermtype = fragtype2fragtermtype[fragtype];
            
                fraglisttype = fragtype2fraglisttype[fragtype];
		if (fraglisttype == act->fraglisttype) {
 		    firstcharoffset = fragtype2firstcharoffset[fragtype];
		    if (firstcharoffset == 0) {
                        frag->len++;
                    }
                    else {
			frag = calloc(sizeof(elem_t), 1);
                        frag->type = fragtype;
                        frag->len = 0;
			frag->beg = inp + 1;
			act->frag->next = frag;
			act->frag = frag;
                    }
                }
 		else {
process_fraglist(act);
		}
            }
        }
        else {
            fraglisttype = fragtype2fraglisttype[fragtype];
	    if (act->fraglisttype != NUL) {
                frag = calloc(sizeof(elem_t), 1);
                frag->type = fragtype;
 		firstcharoffset = fragtype2firstcharoffset[fragtype];
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
