#include <stdio.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "hash.h"

#define MSB_LONG (8*(sizeof(long))-1)
#define SEED 0xA5A5A5A5A5A5A5A5

// Objective:
//    - produce names suitable for use as filenames
//    - hash all the frags from all the strings from a tree of elem_t
//      into a hashname that has ~0 chance of collision
//    - minimal cpu cost
// It is not an objective for this hash to be cryptographic.     

static void hash_list_r(unsigned long *phash, elem_t *list)
{
	elem_t *elem;
	unsigned long hash;
	unsigned char *cp;
    int len;

	if ((elem = list->u.list.first)) {
	    switch ((elemtype_t) elem->type) {
	    case FRAGELEM:
            while (elem) {
                cp = elem->u.frag.frag;
                len = elem->v.frag.len;
                assert(len > 0);
                hash = *phash;
                while (len--) {
                    // XOR character with hash, then rotate hash left one bit.
                    hash = ((hash ^ *cp++) << 1) ^ (hash >> MSB_LONG);
                }
                *phash = hash;
                elem = elem->next;
            }
		    break;
	    case LISTELEM:
		    while (elem) {
			    hash_list_r(phash, elem);	// recurse
			    elem = elem->next;
		    }
		    break;
	    }
	}
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
    *hashname = '\0';
}

void hash_list(char *hashname, elem_t *list)
{
    unsigned long hash;

    assert(sizeof(long) == 8);
    assert(list);

    hash = SEED;
    hash_list_r(&hash, list);
    base64(hashname, hash);
fprintf(stdout,"\n%08lx %s\n",hash, hashname);
}
