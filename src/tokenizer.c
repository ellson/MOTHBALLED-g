
char characters[] = {
     0,  0,  0,  0,  0,  0,  0,  0,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
     0,  0,  0,  0,  0,  0,  0,  0,  /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
     0,  0,  0,  0,  0,  0,  0,  0,  /* CAN  EM SUB ESC  FS  GS  RS  US */
     0,  0,  1,  2,  0,  0,  0,  3,  /* SPC  !   "   #   $   %   &   '  */
     4,  5,  6,  0,  0,  0,  0,  7,  /*  (   )   *   +   ,   -   .   /  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  0   1   2   3   4   5   6   7  */
     0,  0,  8,  9, 10, 11, 12,  0,  /*  8   9   :   ;   <   =   >   ?  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  @   A   B   C   D   E   F   G  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  H   I   J   K   L   M   N   O  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  P   Q   R   S   T   U   V   W  */
     0,  0,  0, 13, 14, 15,  0,  0,  /*  X   Y   Z   [   \   ]   ^   _  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  `   a   b   c   d   e   f   g  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  h   i   j   k   l   m   n   o  */
     0,  0,  0,  0,  0,  0,  0,  0,  /*  p   q   r   s   t   u   v   w  */
     0,  0,  0, 16,  0, 17, 18,  0,  /*  x   y   z   {   |   }   ~  DEL */
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
}

void* charproc[] {
    char_not_special,	/*  0 */
    char_double_quote,	/*  1 */
    char_octothorpe,	/*  2 */
    char_single_quote,	/*  3 */
    char_left_paren,	/*  4 */
    char_right_paren,	/*  5 */
    char_asterisk,	/*  6 */
    char_forward_slash,	/*  7 */
    char_colon,		/*  8 */
    char_semicolon,	/*  9 */
    char_lessthan,	/* 10 */
    char_equals,	/* 11 */
    char_greaterthan,	/* 12 */
    char_left_bracket,	/* 13 */
    char_back_slash,	/* 14 */
    char_right_bracket,	/* 15 */
    char_left_brace,	/* 16 */
    char_right_brace,	/* 17 */
    char_tilde,		/* 18 */
}
    
