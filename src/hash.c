#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "hash.h"

#define MSB_LONG (8*(sizeof(long))-1)

static void hashfrag(unsigned long *phash, int len, unsigned char *frag)
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

static void hash_subject_r(unsigned long *phash, elem_t *list)
{
}

// 64 ascii chars that are safe in filenames
// each character used only once
// order doesn't matter
static char b64[64] = {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p',
    'q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','6','7','8','9','_',',',
};

static void base64(char *hashname, unsigned long hash)
{
    int i;

    for (i = 0; i < 11; i++) {
        *hashname++ = b64[(hash & 0x3F)];
        hash >>= 6;
    }
    *hashname = '0';
}

void hash_list(char *hashname, elem_t *list)
{
    unsigned long hash;

    hash_subject_r (&hash, list);
    base64(hashname, hash);
}
