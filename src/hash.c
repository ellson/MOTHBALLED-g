/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "libje_private.h"

static void hash_list_r(uint64_t *hash, elem_t *list);

#define MSB_LONG (8*(sizeof(long))-1)
#define FNV_INIT  0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3

/**
 * Objective:
 *    - produce names suitable for use as filenames
 *    - hash all the frags from all the strings from a tree of elem_t
 *      into a hashname that has ~0 chance of collision
 *    - minimal cpu cost
 * It is not an objective for this hash to be cryptographic.     
 *
 * The resulting hashes are 64bits which can be represented in 11 filename-safe characters.
 *
 * Hash chosen is the FNV-1a for 64bits
 * (Ref: http://isthe.com/chongo/tech/comp/fnv/ )
 * The FNV_prime is: 2**40 + 2**8 + 0xb3 = 0x100000001b3 = 1099511628211
 *
 * @param hash place for resulting hash
 * @param list - fraglist or list of fraglist to be hashed
 */
void je_hash_list(uint64_t *hash, elem_t *list)
{
    assert(sizeof(long) == 8);
    assert(list);

    *hash = FNV_INIT;
    hash_list_r(hash, list);
}

/**
 * Recursive hash function building on a previously initialized or
 * partially accumulated haahs result.
 *
 * @param hash place for resulting hash
 * @param list - fraglist or list of fraglist to be hashed
 */
static void hash_list_r(uint64_t *hash, elem_t *list)
{
    elem_t *elem;
    unsigned char *cp;
    int len;

    if ((elem = list->first)) {
        switch ((elemtype_t) elem->type) {
        case FRAGELEM:
            while (elem) {
                cp = ((frag_elem_t*)elem)->frag;
                len = ((frag_elem_t*)elem)->len;
                assert(len > 0);
                while (len--) {
                    // XOR with current character
                    *hash ^= *cp++;
                    // multiply by the magic number
                    *hash *= FNV_PRIME;
                }
                elem = elem->next;
            }
            break;
        case LISTELEM:
            while (elem) {
                hash_list_r(hash, elem);    // recurse
                elem = elem->next;
            }
            break;
        default:
            assert(0);  // should not be here
            break;
        }
    }
}

// 64 ascii chars that are safe in filenames
// each character used only once
// must match reverse mapping table
const static unsigned char b64[64] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V',
    'W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l',
    'm','n','o','p','q','r','s','t','u','v','w','x','y','z',',','_'
};

// reverse mapping
const static unsigned char un_b64[128] = {
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1,
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
     -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, 63,
     -1, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
     51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1
};

/**
 * Encode a hash to 11 char+NUL printable base64 (suitable for filename).
 *
 * @param b64string result (caller-provided 12 character buffer for result)
 * @param hash = 64bit integer to be hashed
 */
void je_long_to_base64(unsigned char b64string[], const uint64_t *hash)
{
    int i;
    uint64_t h = *hash;
    char *p = b64string;

    for (i = 0; i < 11; i++) {
        *p++ = b64[(h & 0x3F)];
        h >>= 6;
    }
    *p = '\0';
}

/**
 * Decode base64 string to a hash value
 *
 * @param b64string 
 * @param hash result
 * @return success/fail
 */
success_t je_base64_to_long(const unsigned char b64string[], uint64_t *hash)
{
    uint64_t h = 0;
    char c;
    size_t len;

    if ((len = strlen((char*)b64string)) != 11) {
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
        h <<= 6;
        h |= c;
    }
    *hash = h;
    return SUCCESS;
}

// FIXME - or not,  but this is broken
#if 0
/**
 * Store the hash in a bucket-list
 *
 * Internally the 64bit hashes are stored in 64 hash-buckets, each containing a list of hashes.
 * The least-significant 6 bits of the 64bit hashes are used to index to the hash-bucket.
 *
 * @param C context
 * @param hash
 * @return a list element
 */
hash_elem_t *je_hash_bucket(CONTEXT_t * C, uint64_t hash)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t *elem, **next;

    next = &(C->hash_buckets[(hash & 0x3F)]);
    while(*next) {
        elem  = *next;
        if (((hash_elem_t*)elem)->hash == hash) {
            return (hash_elem_t*)elem;
        }
        next = &(elem->next);
    }
    elem = new_hash(LIST, hash);
    *next = elem;
    return (hash_elem_t*)elem;
}
#endif
