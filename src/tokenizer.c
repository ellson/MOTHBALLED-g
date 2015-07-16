#include <stdio.h>
#include <stdlib.h>

#include "list.h"

enum charclass_t {
    NUL,       // EOF
    ABC,       // simple_string_character
     WS,       // whitespace
     LF,       // newline
     CR,       // return
    DQT,       // '"'
    SQT,       // '''
    LPN,       // '('
    RPN,       // ')'
    LAN,       // '<'
    RAN,       // '>'
    LBT,       // '['
    RBT,       // ']'
    LBE,       // '{'
    RBE,       // '}'
    FSL,       // '/'
    BSL,       // '\'
    OCT,       // '#'
    AST,       // '*'
    CLN,       // ':'
    SCN,       // ';'
    EQL,       // '='
    TLD,       // '~'
};
 
unsigned char char2charclass[] = {
   NUL, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
   ABC,  WS,  LF, ABC, ABC,  CR, ABC, ABC,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
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
};
 
enum charclassprops_t {
   NONE         = 0,
   STRING       = 1<<0,
   TWO          = 1<<1,
   SPACE        = 1<<2,
   OPEN         = 1<<3,
   CLOSE        = 1<<4,
   PARENTHESIS  = 1<<5,
   ANGLEBRACKET = 1<<6,
   BRACKET      = 1<<7,
   BRACE        = 1<<8,
   DQTSTR       = 1<<9,
   SQTSTR       = 1<<10,
   EOL          = 1<<11,
   CMNTBEG      = 1<<12,
   CMNTEND      = 1<<13,
   CMNTEOLBEG   = 1<<14,
   CMNTEOLEND   = 1<<15,
   CMNTSTR      = 1<<16,
   ESCAPE       = 1<<17,
   DISAMBIG     = 1<<18,
   ANCESTOR     = 1<<19,
   NPATH        = 1<<20,
   NLIST        = 1<<21,
};

unsigned int charONEclass2props[] = {
    /* NUL  */  NONE,
    /* ABC  */  STRING,
    /* WSP  */  SPACE,
    /*  LF  */  TWO | EOL | CMNTEOLEND | SPACE,
    /*  CR  */  TWO | EOL | CMNTEOLEND | SPACE,
    /* DQT  */  OPEN | CLOSE,
    /* SQT  */  OPEN | CLOSE,
    /* LPN  */  OPEN | PARENTHESIS,
    /* RPN  */  CLOSE | PARENTHESIS,
    /* LAN  */  OPEN | ANGLEBRACKET,
    /* RAN  */  OPEN | ANGLEBRACKET,
    /* LBR  */  OPEN | BRACKET,
    /* RBR  */  OPEN | BRACKET,
    /* LBE  */  OPEN | BRACE,
    /* RBE  */  OPEN | BRACE,
    /* OCT  */  NONE,
    /* AST  */  TWO | CMNTEND,
    /* FSL  */  TWO | CMNTBEG | CMNTEOLBEG,
    /* BSL  */  TWO | ESCAPE,
    /* CLN  */  TWO | DISAMBIG  | ANCESTOR,
    /* SCN  */  NONE,
    /* EQL  */  NONE,
    /* TLD  */  NONE,
};

#define proptypemask (STRING | SPACE | EOL | PARENTHESIS | ANGLEBRACKET | BRACKET | BRACE | DQTSTR | SQTSTR | CMNTSTR)

unsigned int charTWOclass2props[] = {
    /* NUL  */  ESCAPE | ABC,
    /* ABC  */  ESCAPE | ABC, 
    /* WSP  */  ESCAPE | ABC,
    /*  LF  */  ESCAPE | EOL,
    /*  CR  */  ESCAPE | EOL,             // FIXME = what about '\' CR LF or '\' LF CR
    /* DQT  */  ESCAPE | ABC,
    /* SQT  */  ESCAPE | ABC,
    /* LPN  */  ESCAPE | ABC,
    /* RPN  */  ESCAPE | ABC,
    /* LAN  */  ESCAPE | ABC,
    /* RAN  */  ESCAPE | ABC,
    /* LBR  */  ESCAPE | ABC,
    /* RBR  */  ESCAPE | ABC,
    /* LBE  */  ESCAPE | ABC,
    /* RBE  */  ESCAPE | ABC,
    /* OCT  */  ESCAPE | ABC,
    /* AST  */  ESCAPE | CMNTBEG,
    /* FSL  */  ESCAPE | CMNTEND | CMNTEOLBEG,
    /* BSL  */  ESCAPE | ABC,
    /* CLN  */  ESCAPE | DISAMBIG,
    /* SCN  */  ESCAPE | ABC,
    /* EQL  */  ESCAPE | ABC,
    /* TLD  */  ESCAPE | ABC,
};

typedef struct {
    elemlist_t fraglist, npathlist, nlistlist, edgelist, elistlist;
} act_t;

static int opencloseblock(act_t *act, unsigned char charclass) {
    return 0;
}

unsigned char *test=(unsigned char*)"<aa bb cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp, c;
    int rc, charcnt;
    unsigned char charclass;
    unsigned int  charprops;
    act_t *act;
    elemlist_t *fraglist, *npathlist, *nlistlist, *edgelist, *elistlist;
    elem_t *frag, *npathelem, *nlistelem;

    act = calloc(sizeof(act_t), 1);

    fraglist = &(act->fraglist);
    npathlist = &(act->npathlist);
    nlistlist = &(act->nlistlist);
    edgelist = &(act->edgelist);
    elistlist = &(act->elistlist);

    inp = test;

    charcnt = 0;
    while ((c = *inp++)) {
        charcnt++;
        charclass = char2charclass[c];
        charprops = charONEclass2props[charclass];
        frag = fraglist->last;
        if (frag) {    // a frag already exists
            if (charprops & frag->type) {  // matching charprops, just continue appending to frag
                frag->len++;
            }
	    else {
                if (charprops & fraglist->type) {  // if matches fraglist 
 		    if (charprops & (STRING|SPACE)) {  // char that can be simply appended
                        frag->len++;
                    }
                    else { // DQT|SQT|BSL  -- charprop that start at next char - so new frag to skip this char
			frag = newelem(charprops & proptypemask, inp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new charprops don't match, so something got terminated and needs promoting
                    if (fraglist->type & STRING) {
			npathelem = joinlist2elem(fraglist, NPATH);
			appendlist(npathlist, npathelem);
fprintf(stdout,"npathelem: %s\n",npathelem->buf);

//FIXME - something in here to deal with path separators:  :/ <abc>/ <abc> :: <abc> : <abc>
			nlistelem = joinlist2elem(npathlist, NLIST);
			appendlist(nlistlist, nlistelem);
fprintf(stdout,"nlistelem: %s\n",nlistelem->buf);
		    } 
                    freelist(fraglist);  // free anything that hasn't been promoted,  e.g. WS
		}
            }
        }

        if (charprops & (OPEN|CLOSE)) {
	    rc = opencloseblock(act, charclass);
	    if (rc) {
		fprintf(stderr, "parser error at char number: %d   \"%c\"\n", charcnt, c);
                exit(rc);
            }
        }
// FIXME - deal with OPEN and CLOSE
//     NPATH | NLIST --> EDGE
//     EDGE --> ELIST

        frag = fraglist->last;
        if (!frag) { // no current frag
            if (charprops & (STRING|SPACE|DQTSTR|SQTSTR|CMNTSTR)) { // charclass that need a frag
 		if (charprops & (STRING|SPACE)) { // charclass that start at this char
		    frag = newelem(charprops & proptypemask, inp-1, 1, 0);
                }
                else { // charclass that start at next char
		    frag = newelem(charprops & proptypemask, inp, 0, 0);
                }
		appendlist(fraglist, frag);
                switch (charclass) {  // set list type with all classes accepted in fraglist
		case ABC:
		    fraglist->type = STRING;
		    break;
		case WS:
		case LF:
		case CR:
		    fraglist->type = SPACE;
		    break;
		case DQT:
		    fraglist->type = DQTSTR;
		    break;
		case SQT:
		    fraglist->type = SQTSTR;
		    break;
		case BSL:
		    fraglist->type = STRING;
		    break;
                default:
		    fprintf(stderr,"shouldn't get here\n");
                }
            }
	}
    }
}
