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
} toktype_t;
 
toktype_t char2tok[] = {
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
    toktype_t type;
    void *beg;
    int len;
    int allocated;
    elem_t *next;
};

typedef struct {
    toktype_t fraglisttype;
    elem_t *fraglist, *frag;
} act_t;

typedef int (*tokfunc)(act_t *act);

static int tok_NUL (act_t *act);
static int tok_ABC (act_t *act);
static int tok_WS  (act_t *act);
static int tok_NL  (act_t *act);
static int tok_CR  (act_t *act);
static int tok_DQT (act_t *act);
static int tok_SQT (act_t *act);
static int tok_LPN (act_t *act);
static int tok_RPN (act_t *act);
static int tok_LAN (act_t *act);
static int tok_RAN (act_t *act);
static int tok_LBT (act_t *act);
static int tok_RBT (act_t *act);
static int tok_LBE (act_t *act);
static int tok_RBE (act_t *act);
static int tok_FSL (act_t *act);
static int tok_BSL (act_t *act);
static int tok_OCT (act_t *act);
static int tok_AST (act_t *act);
static int tok_CLN (act_t *act);
static int tok_SCN (act_t *act);
static int tok_EQL (act_t *act);
static int tok_TLD (act_t *act);

tokfunc tokproc[]={
    tok_NUL,   
    tok_ABC,
    tok_WS,   
    tok_NL,   
    tok_CR,   
    tok_DQT,
    tok_SQT,
    tok_LPN,
    tok_RPN,
    tok_LAN,
    tok_RAN,
    tok_LBT,
    tok_RBT,
    tok_LBE,
    tok_RBE,
    tok_FSL,
    tok_BSL,
    tok_OCT,
    tok_AST,
    tok_CLN,
    tok_SCN,
    tok_EQL,
    tok_TLD
};

static void process_fraglist (void) {
    fprintf(stdout,"process fraglist\n");
}

unsigned char *test=(unsigned char*)"<aa bb cc>";

int main (int argc, char *argv[]) {
    int transition;
    unsigned char *inp, c;
    act_t *act;

    act = calloc(sizeof(act_t), 1);

    act->frag = calloc(sizeof(elem_t), 1);
    act->fraglist = act->frag;

    inp = test;
    while ((c = *inp)) {
        toktype_t toktype;
        toktype = char2tok[c];
	elem_t *frag = act->frag;
        if (toktype == frag->type) {
            frag->len++;
        }
	else {
            transition = tokproc[toktype](act);
	    switch (transition) {
            case 0:  // add to current frag
		frag->len++;
	        break;
            case 1:  // append new frag to current fraglist, expecting to start on next character
                frag = calloc(sizeof(elem_t), 1);
                frag->type = toktype;
                frag->len = 0;
                frag->beg = inp + 1;
                act->frag->next = frag;
                act->frag = frag;
	        break;
            case 2:  // append new frag to current fraglist using this character
                frag = calloc(sizeof(elem_t), 1);
                frag->type = toktype;
                frag->len = 1;
                frag->beg = inp;
                act->frag->next = frag;
                act->frag = frag;
	        break;
            case 3:  // process current fraglist, and start new on next character
                process_fraglist();
                frag = calloc(sizeof(elem_t), 1);
                frag->type = toktype;
                frag->len = 0;
                frag->beg = inp + 1;
                act->frag = frag;
                act->fraglist = frag;
                act->fraglisttype = toktype;
	        break;
            case 4:  // process current fraglist, and start new using this character
                process_fraglist();
                frag = calloc(sizeof(elem_t), 1);
                frag->type = toktype;
                frag->len = 1;
                frag->beg = inp;
                act->frag = frag;
                act->fraglist = frag;
                act->fraglisttype = toktype;
	        break;
	    default:
		fprintf(stderr,"shouldn't happen\n");
	    }
	}
        inp++;
    }
}

static int tok_NUL (act_t *act) {
    fprintf(stdout,"null\n");
    return 1;
}

static int tok_ABC (act_t *act) {
    fprintf(stdout,"abc\n");
    return 0;
}

static int tok_WS (act_t *act) {
    fprintf(stdout,"whitespace\n");
    return 0;
}

static int tok_NL   (act_t *act) {
    // increment line count unless preceeded by CR
    fprintf(stdout,"newline\n");
    return 0;
}

static int tok_CR   (act_t *act) {
    // increment line count
    fprintf(stdout,"return\n");
    return 0;
}

static int tok_DQT (act_t *act) {
    fprintf(stdout,"double_quote\n");
    return 0;
}

static int tok_SQT (act_t *act) {
    fprintf(stdout,"single quote\n");
    return 0;
}

static int tok_LPN (act_t *act) {
    fprintf(stdout,"left paren\n");
    return 0;
}

static int tok_RPN (act_t *act) {
    fprintf(stdout,"right paren\n");
    return 0;
}

static int tok_LAN (act_t *act) {
    fprintf(stdout,"lessthan\n");
    return 0;
}

static int tok_RAN (act_t *act) {
    fprintf(stdout,"greaterthan\n");
    return 0;
}

static int tok_LBT (act_t *act) {
    fprintf(stdout,"left bracket\n");
    return 0;
}

static int tok_RBT (act_t *act) {
    fprintf(stdout,"right bracket\n");
    return 0;
}

static int tok_LBE (act_t *act) {
    fprintf(stdout,"left brace\n");
    return 0;
}

static int tok_RBE (act_t *act) {
    fprintf(stdout,"right brace\n");
    return 0;
}

static int tok_FSL (act_t *act) {
    fprintf(stdout,"forward slash\n");
    return 0;
}

static int tok_BSL (act_t *act) {
    fprintf(stdout,"backslash\n");
    return 0;
}

static int tok_OCT (act_t *act) {
    fprintf(stdout,"octothorpe\n");
    return 0;
}

static int tok_AST (act_t *act) {
    fprintf(stdout,"asterisk\n");
    return 0;
}

static int tok_CLN (act_t *act) {
    fprintf(stdout,"colon\n");
    return 0;
}

static int tok_SCN (act_t *act) {
    fprintf(stdout,"semicolon\n");
    return 0;
}

static int tok_EQL (act_t *act) {
    fprintf(stdout,"equals\n");
    return 0;
}

static int tok_TLD (act_t *act) {
    fprintf(stdout,"tilde\n");
    return 0;
}

