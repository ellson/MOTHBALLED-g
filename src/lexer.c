#include "grammar.c"

// Every character is grouped into exactly one state_t
// All the characters in a state_t are treated identially by the parser
//
// state_t defaults to int, which would make this table 768 bytes larger
unsigned char char2state[] = {
    NUL,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* NUL SOH STX ETX EOT ENQ ACK BEL */
    ABC, WS, LF,ABC,ABC, CR,ABC,ABC, /*  BS TAB  LF  VT  FF  CR  SO  SI */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* CAN  EM SUB ESC  FS  GS  RS  US */
     WS,ABC,DQT,OCT,ABC,ABC,ABC,SQT, /* SPC  !   "   #   $   %   &   '  */
    LPN,RPN,AST,ABC,ABC,ABC,ABC,FSL, /*  (   )   *   +   ,  -   .   /  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  0   1   2   3   4   5   6   7  */
    ABC,ABC,CLN,SCN,LAN,EQL,RAN,ABC, /*  8   9   :   ;   <   =   >   ?  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  @   A   B   C   D   E   F   G  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  H   I   J   K   L   M   N   O  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  P   Q   R   S   T   U   V   W  */
    ABC,ABC,ABC,LBR,BSL,RBR,HAT,ABC, /*  X   Y   Z   [   \   ]   ^   _  */
    TIC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  `   a   b   c   d   e   f   g  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  h   i   j   k   l   m   n   o  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  p   q   r   s   t   u   v   w  */
    ABC,ABC,ABC,LBE,ABC,RBE,TLD,ABC, /*  x   y   z   {   |   }   ~  DEL */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
};
