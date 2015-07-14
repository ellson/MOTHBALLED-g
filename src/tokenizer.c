#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    NLL, ABC, WSP, DQT,
    OCT, SQT, LPN, RPN,
    AST, FSL, CLN, SCN,
    LAN, EQL, RAN, LBR,
    BSL, RBR, LBE, RBE,
    TLD
} tok_t;
 
tok_t char2tok[] = {
   NLL, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
   ABC, WSP, WSP, ABC, ABC, WSP, ABC, ABC,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* CAN  EM SUB ESC  FS  GS  RS  US */
   WSP, ABC, DQT, OCT, ABC, ABC, ABC, SQT,  /* SPC  !   "   #   $   %   &   '  */
   LPN, RPN, AST, ABC, ABC, ABC, ABC, FSL,  /*  (   )   *   +   ,   -   .   /  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  0   1   2   3   4   5   6   7  */
   ABC, ABC, CLN, SCN, LAN, EQL, RAN, ABC,  /*  8   9   :   ;   <   =   >   ?  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  @   A   B   C   D   E   F   G  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  H   I   J   K   L   M   N   O  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  P   Q   R   S   T   U   V   W  */
   ABC, ABC, ABC, LBR, BSL, RBR, ABC, ABC,  /*  X   Y   Z   [   \   ]   ^   _  */
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

typedef enum {
    BEG,
    APPEND,
    TERM_POP,
    TERM,
    TERM_AND_REUSE
} state_t;
 
void char_null (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"null\n");
    exit(0);
}

void char_not_special (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"not special\n");
}

void char_whitespace (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"whitespace\n");
}

void char_double_quote (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"double_quote\n");
}

void char_octothorpe (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"octothorpe\n");
}

void char_single_quote (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"single quote\n");
}

void char_left_paren (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"left paren\n");
}

void char_right_paren (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"right paren\n");
}

void char_asterisk (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"asterisk\n");
}

void char_forward_slash (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"forward slash\n");
}

void char_colon (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"colon\n");
}

void char_semicolon (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"semicolon\n");
}

void char_lessthan (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"lessthan\n");
}

void char_equals (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"equals\n");
}

void char_greaterthan (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"greaterthan\n");
}

void char_left_bracket (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"left bracket\n");
}

void char_back_slash (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"backslash\n");
}

void char_right_bracket (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"right bracket\n");
}

void char_left_brace (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"left brace\n");
}

void char_right_brace (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"right brace\n");
}

void char_tilde (state_t *state, tok_t *beg_tok) {
    fprintf(stdout,"tilde\n");
}

typedef void (*charfunc)(state_t *state, tok_t *beg_tok);
charfunc charproc[]={
    char_null,   	/* NLL */
    char_not_special,	/* ABC */
    char_whitespace,   	/* WSP */
    char_double_quote,	/* DQT */
    char_octothorpe,	/* OCT */
    char_single_quote,	/* SQT */
    char_left_paren,	/* LPN */
    char_right_paren,	/* RPN */
    char_asterisk,	/* AST */
    char_forward_slash,	/* FSL */
    char_colon,		/* CLN */
    char_semicolon,	/* SCN */
    char_lessthan,	/* LAN */
    char_equals,	/* EQL */
    char_greaterthan,	/* RAN */
    char_left_bracket,	/* LBR */
    char_back_slash,	/* BSL */
    char_right_bracket,	/* RBR */
    char_left_brace,	/* LBE */
    char_right_brace,	/* RBE */
    char_tilde		/* TLD */
};

int main (int argc, char *argv[]) {
    unsigned char *test=(unsigned char*)"<a b c>";
    unsigned char *p;
    state_t state;
    tok_t beg_tok;

    p = test;
    while (true) {
        unsigned char c = *p++;
        tok_t this = char2tok[c];
        charproc[this](&state, &beg_tok);
    }
}
