#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
 
// escape translations
//    Most characters translate to themselves, including:
//        SPC '\040'
//         '  '\047'
//         \  '\134'
//        DEL '\177'
//    A few translate to different characters:
//         n  '\156'  ->  LF  '\013'
//         r  '\122'  ->  CR  '\015'
//         t  '\164'  -> TAB  '\011'
unsigned char escchar2char[] = {
   '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007', /* NUL SOH STX ETX EOT ENQ ACK BEL */
   '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017', /*  BS TAB  LF  VT  FF  CR  SO  SI */
   '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027', /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
   '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037', /* CAN  EM SUB ESC  FS  GS  RS  US */
   '\040',    '!',    '"',    '#',    '$',    '%',    '&', '\047', /* SPC  !   "   #   $   %   &   '  */
      '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/', /*  (   )   *   +   ,   -   .   /  */
      '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7', /*  0   1   2   3   4   5   6   7  */
      '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?', /*  8   9   :   ;   <   =   >   ?  */
   '\000',    'A',    'B',    'C',    'D',    'E',    'F',    'G', /*  @   A   B   C   D   E   F   G  */
      'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O', /*  H   I   J   K   L   M   N   O  */
      'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W', /*  P   Q   R   S   T   U   V   W  */
      'X',    'Y',    'Y',    '[', '\134',    ']',    '^',    '_', /*  X   Y   Z   [   \   ]   ^   _  */
      '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g', /*  `   a   b   c   d   e   f   g  */
      'h',    'i',    'j',    'k',    'l',    'm', '\013',    'o', /*  h   i   j   k   l   m   n   o  */
      'p',    'q', '\015',    's', '\011',    'u',    'v',    'w', /*  p   q   r   s   t   u   v   w  */
      'x',    'y',    'z',    '{',    '|',    '}',    '~', '\177', /*  x   y   z   {   |   }   ~  DEL */
   '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
   '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
   '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
   '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
   '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
   '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
   '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
   '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
   '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
   '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
   '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
   '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
   '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
   '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
   '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
   '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
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
    elem_t *fraglist, *npathlist, *nlistlist, *edgelist, *elistlist;
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
    elem_t *fraglist, *npathlist, *nlistlist, *edgelist, *elistlist;
    elem_t *frag, *npathelem, *nlistelem;

    act = calloc(sizeof(act_t), 1);

    act->fraglist = newlist(0);
    act->npathlist = newlist(0);
    act->nlistlist = newlist(0);
    act->edgelist = newlist(0);
    act->elistlist = newlist(0);

    fraglist = act->fraglist;
    npathlist = act->npathlist;
    nlistlist = act->nlistlist;
    edgelist = act->edgelist;
    elistlist = act->elistlist;

    inp = test;

    charcnt = 0;
    while ((c = *inp++)) {
        charcnt++;
        charclass = char2charclass[c];
        charprops = charONEclass2props[charclass];
        frag = fraglist->u.lst.last;

        if (frag) {    // a frag already exists

            assert (frag->type == STR);

            if (charprops & frag->props) {  // matching charprops, just continue appending to frag
                frag->u.str.len++;
            }
	    else {
                if (charprops & fraglist->props) {  // if matches fraglist 
 		    if (charprops & (STRING|SPACE)) {  // char that can be simply appended
                        frag->u.str.len++;
                    }
                    else { // DQT|SQT|BSL  -- charprop that start at next char - so new frag to skip this char
			frag = newelem(charprops & proptypemask, inp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new charprops don't match, so something got terminated and needs promoting
//printj(fraglist);
                    if (fraglist->props & STRING) {
			npathelem = list2elem(fraglist);
			appendlist(npathlist, npathelem);
printj(npathlist);

//FIXME - something in here to deal with path separators:  :/ <abc>/ <abc> :: <abc> : <abc>
			nlistelem = list2elem(npathlist);
			appendlist(nlistlist, nlistelem);
//printf("nlist: ");
//printj(nlistlist, " ");
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

        frag = fraglist->u.lst.last;
        if (!frag) { // no current frag
            if (charprops & (STRING|SPACE|DQTSTR|SQTSTR|CMNTSTR)) { // charclass that need a frag
 		if (charprops & (STRING|SPACE)) { // charclass that start at this char
		    frag = newelem(charprops & proptypemask, inp-1, 1, 0);
                }
                else { // charclass that start at next char
		    frag = newelem(charprops & proptypemask, inp, 0, 0);
                }
		appendlist(fraglist, frag);
                switch (charclass) {  // set list props with all classes accepted in fraglist
		case ABC:
		    fraglist->props = STRING;
		    break;
		case WS:
		case LF:
		case CR:
		    fraglist->props = SPACE;
		    break;
		case DQT:
		    fraglist->props = DQTSTR;
		    break;
		case SQT:
		    fraglist->props = SQTSTR;
		    break;
		case BSL:
		    fraglist->props = STRING;
		    break;
                default:
		    fprintf(stderr,"shouldn't get here\n");
                }
            }
	}
    }
}
