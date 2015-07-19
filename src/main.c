# include "parse.h"

unsigned char *test=(unsigned char*)"<a\\Aa 'bb' cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp;
    int rc;

    printg();
    return 0;

    inp = test;
    
    rc = parse(inp);

    return rc;
}
