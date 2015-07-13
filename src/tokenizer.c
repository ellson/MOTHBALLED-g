#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

unsigned char characters[] = {
     0,  1,  1,  1,  1,  1,  1,  1,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
     1,  2,  2,  1,  1,  2,  1,  1,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
     1,  1,  1,  1,  1,  1,  1,  1,  /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
     1,  1,  1,  1,  1,  1,  1,  1,  /* CAN  EM SUB ESC  FS  GS  RS  US */
     2,  1,  3,  4,  1,  1,  1,  5,  /* SPC  !   "   #   $   %   &   '  */
     6,  7,  8,  1,  1,  1,  1,  9,  /*  (   )   *   +   ,   -   .   /  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  0   1   2   3   4   5   6   7  */
     1,  1, 10, 11, 12, 13, 14,  1,  /*  8   9   :   ;   <   =   >   ?  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  @   A   B   C   D   E   F   G  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  H   I   J   K   L   M   N   O  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  P   Q   R   S   T   U   V   W  */
     1,  1,  1, 15, 16, 17,  1,  1,  /*  X   Y   Z   [   \   ]   ^   _  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  `   a   b   c   d   e   f   g  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  h   i   j   k   l   m   n   o  */
     1,  1,  1,  1,  1,  1,  1,  1,  /*  p   q   r   s   t   u   v   w  */
     1,  1,  1, 18,  1, 19, 20,  1,  /*  x   y   z   {   |   }   ~  DEL */
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1,
     1,  1,  1,  1,  1,  1,  1,  1
};

unsigned char* char_null(unsigned char c){exit(0);}
unsigned char* char_not_special(unsigned char c){return((unsigned char*)"not special");}
unsigned char* char_whitespace(unsigned char c){return((unsigned char*)"whitespace");}
unsigned char* char_double_quote(unsigned char c){return((unsigned char*)"double_quote");}
unsigned char* char_octothorpe(unsigned char c){return((unsigned char*)"octothorpe");}
unsigned char* char_single_quote(unsigned char c){return((unsigned char*)"single quote");}
unsigned char* char_left_paren(unsigned char c){return((unsigned char*)"left paren");}
unsigned char* char_right_paren(unsigned char c){return((unsigned char*)"right paren");}
unsigned char* char_asterisk(unsigned char c){return((unsigned char*)"asterisk");}
unsigned char* char_forward_slash(unsigned char c){return((unsigned char*)"forward slash");}
unsigned char* char_colon(unsigned char c){return((unsigned char*)"colon");}
unsigned char* char_semicolon(unsigned char c){return((unsigned char*)"semicolon");}
unsigned char* char_lessthan(unsigned char c){return((unsigned char*)"lessthan");}
unsigned char* char_equals(unsigned char c){return((unsigned char*)"equals");}
unsigned char* char_greaterthan(unsigned char c){return((unsigned char*)"greaterthan");}
unsigned char* char_left_bracket(unsigned char c){return((unsigned char*)"left bracket");}
unsigned char* char_back_slash(unsigned char c){return((unsigned char*)"backslash");}
unsigned char* char_right_bracket(unsigned char c){return((unsigned char*)"right bracket");}
unsigned char* char_left_brace(unsigned char c){return((unsigned char*)"left brace");}
unsigned char* char_right_brace(unsigned char c){return((unsigned char*)"right brace");}
unsigned char* char_tilde(unsigned char c){return((unsigned char*)"tilde");}

typedef unsigned char* (*charfunc)(unsigned char);
charfunc charproc[]={
    char_null,   	/*  0 */
    char_not_special,	/*  1 */
    char_whitespace,   	/*  2 */
    char_double_quote,	/*  3 */
    char_octothorpe,	/*  4 */
    char_single_quote,	/*  5 */
    char_left_paren,	/*  6 */
    char_right_paren,	/*  7 */
    char_asterisk,	/*  8 */
    char_forward_slash,	/*  9 */
    char_colon,		/* 10 */
    char_semicolon,	/* 11 */
    char_lessthan,	/* 12 */
    char_equals,	/* 13 */
    char_greaterthan,	/* 14 */
    char_left_bracket,	/* 15 */
    char_back_slash,	/* 16 */
    char_right_bracket,	/* 18 */
    char_left_brace,	/* 19 */
    char_right_brace,	/* 20 */
    char_tilde		/* 21 */
};
    
int main (int argc, char *argv[]) {
    unsigned char *test=(unsigned char*)"<a b c>";
    unsigned char *p;

    p = test;
    while (true) {
        unsigned char c = *p++;
        fprintf(stdout,"%s\n",charproc[characters[c]](c));
    }
}
