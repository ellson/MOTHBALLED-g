#include <stdio.h>
#include <assert.h>

#include "hash.h"

#define MSB_LONG (8*(sizeof(long))-1)

void hashfrag(unsigned long *phash, int len, unsigned char *frag)
{
	unsigned long hash;
	unsigned char *cp;

	assert(phash);
	assert(len > 0);

	cp = frag;
	hash = *phash;
	while (len--) {
		hash = ((hash ^ *cp++) << 1) ^ (hash >> MSB_LONG);
	}
	*phash = hash;
}
