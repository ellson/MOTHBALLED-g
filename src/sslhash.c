/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <assert.h>

#include "thread.h"
#include "sslhash.h"

static void hash_list_r(EVP_MD_CTX *ctx, elem_t *list);

/**
 * Objective:
 *    - the names of nodes and edges can be long, produce a hash of the name for ???
 *    - hash all the frags from all the strings from a tree of elem_t
 *      into a hashname that has ~0 chance of collision
 *
 * @param hash place for resulting hash
 * @param list - fraglist or list of fraglist to be hashed
 */
void sslhash_list(uint64_t *hash, elem_t *list)
{
    EVP_MD_CTX *ctx;

#ifndef HAVE_EVP_MD_CTX_NEW
    if ((ctx = malloc(sizeof(EVP_MD_CTX))) == NULL)
        FATAL("malloc()");
#else
    if ((ctx = EVP_MD_CTX_new()) == NULL)
        FATAL("EVP_MD_CTX_new()");
#endif
    unsigned char digest[64];
    unsigned int digest_len=64;
    assert(list);

    if ((EVP_DigestInit_ex(ctx, EVP_sha1(), NULL)) != 1)
        FATAL("EVP_DigestInit_ex()");
    hash_list_r(ctx, list);
    if ((EVP_DigestFinal_ex(ctx, digest, &digest_len)) != 1)
        FATAL("EVP_DigestFinal_ex()");
}
/**
 * Recursive hash accumulation of a fraglist, or list of fraglists
 *
 * @param ctx  - hashing context
 * @param list - fraglist or list of fraglist to be hashed
 */
static void hash_list_r(EVP_MD_CTX *ctx, elem_t *list)
{
    elem_t *elem;

    assert(list->type == (char)LISTELEM);

    if ((elem = list->u.l.first)) {
        switch ((elemtype_t) elem->type) {
        case FRAGELEM:
            while (elem) {
                if ((EVP_DigestUpdate(ctx, elem->u.f.frag, elem->len)) != 1)
                    FATAL("EVP_DigestUpdate()");
                elem = elem->u.l.next;
            }
            break;
        case LISTELEM:
            while (elem) {
                hash_list_r(ctx, elem);    // recurse
                elem = elem->u.l.next;
            }
            break;
        default:
            assert(0);  // should not be here
            break;
        }
    }
}
