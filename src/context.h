typedef struct {
    int *pargc;            // remaining filenames from command line
    char **argv;
    char *filename;        // name of file currently being processed, or "-" for stdin
    FILE *file;            // open file handle for file currently being processed
    inbuf_t *inbuf;        // the active input buffer
    unsigned char *in;     // next charater to be processed
    state_t insi;          // state represented by last character read
    elem_t subject;        // header of subject stack for containment
			   //  (Current subject is first in list.  Parents follow.)
    elem_t sameend_legs;   // header of list of LEGS from previous ACT.
    elem_t sameend_legs_new; // header of accumulating list of LEGS in current ACT.
} context_t;
