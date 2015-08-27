#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
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
	    case HASHELEM:
		    assert(0);  // should not be here
            break;
	    }
	}
}

// 64 ascii chars that are safe in filenames
// each character used only once
// must match reverse mapping table
static char b64[64] = {
    '0','1','2','3','4','5','6','7','8','9',
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
    'P','Q','R','S','T','U','V','W','X','Y','Z',
        'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
    'p','q','r','s','t','u','v','w','x','y','z',
    ',','_'
};

static char un_b64[128] = {
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1,
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
     -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, 63,
     -1, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
     51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1
};

void je_long_to_base64(char hashname[], unsigned long *phash)
{
    int i;
    unsigned long hash;

    hash = *phash;
    for (i = 0; i < 11; i++) {
        hashname[i] = b64[(hash & 0x3F)];
        hash >>= 6;
    }
    hashname[i] = '\0';
}

success_t je_base64_to_long(char *b64string, unsigned long *phash)
{
    unsigned long hash;
    char c;
    size_t len;

    if ((len = strlen(b64string)) != 11) {
        return FAIL;
    }
    while (len-- > 0) {
        c = b64string[len];
        if (c < 0) {
            return FAIL;
        }
        if ((c = un_b64[(int)c]) < 0) {
            return FAIL;
        }
        hash <<= 6;
        hash |= c;
    }
    *phash = hash;
    return SUCCESS;
}

void je_hash_list(unsigned long *hash, elem_t *list)
{
    assert(sizeof(long) == 8);
    assert(list);

    *hash = SEED;
    hash_list_r(hash, list);
}

elem_t *je_hash_bucket(context_t * C, unsigned long hash)
{
    elem_t *elem, **next;

    next = &(C->hash_buckets[(hash & 0x3F)]);
    while(*next) {
        elem  = *next;
        if (elem->u.hash.hash == hash) {
            return elem;
        }
        next = &(elem->next);
    }
    elem = new_hash(C, hash);
    *next = elem;
    return elem;
}

