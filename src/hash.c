#include <stdio.h>
#include <assert.h>

#include "hash.h"

#define MSB_LONG (8*(sizeof(long))-1)

void hashfrag (unsigned long *phash, int len, unsigned char *frag) {
    unsigned long hash;

    assert(phash);
    assert(len > 0);

    hash = *phash;
    while (len--) {
	hash = ((hash ^ *frag++) << 1) ^ (hash >> MSB_LONG);
    }
    *phash = hash;
}


int main (int argc, char *argv[]) {

    unsigned long hash = 0; // probably want to seed the hash in real use
    int len = 2;
    char frag[] = {'a', 'b', '\0'};

    hashfrag(&hash, len, frag);

    fprintf(stdout,"hash of \"%s\" is %lx\n", frag, hash);

    return 0;
}
