/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <openssl/evp.h>
#include <assert.h>

#include "libje_private.h"

/**
 * ikea.c   (the container store )
 *
 * Objective:  
 *        A container holds a single graph in a tree of graph containers.
 *
 *        This code maintains each container in a separate file.
 *
 *        During file write (before the contenthash has been determined),  it is kept in a temp dir.
 *
 *        At close (or flush) the file is moved (or copied) to a master content file is named by the hash
 *        of the content,  then possibly multiple, name files using the hash of the
 *        name {node, edge, pattern) are hardlinked to the master..
 *
 *        Separate directories are used for tempory, name and content hashes:
 *            - temporary/namehash
 *            - content/contenthash
 *            - name/namehash
 *            - name/namehash
 *            - name/namehash
 *                      ...
 *
 *         A container store is a file collection of all the containers from a tree of graph containers
 *         described by a 'g' file
 *
 *         When g is stopped, the container store is archived to a compressed tar file.
 *         When g is running, the tree is unpacked into a private tree under $HOME/.g/`pid`/              
 */


// private struct
struct ikea_s {
    ikea_t *next;
    FILE *fh;
    EVP_MD_CTX ctx;   // context for content hash accumulation
    char namehash[(((EVP_MAX_MD_SIZE+1)*8/6)+1)]; // big enough for base64 encode largest possible digest, plus \0
    char contenthash[(((EVP_MAX_MD_SIZE+1)*8/6)+1)]; // big enough for base64 encode largest possible digest, plus \0
    char mode[4];
};

// forward mapping
const static char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

// reverse base_mapping
const static char un_b64[128] = {
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, -1,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
     -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
     -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

/**
 * Encode a byte array to a NUL terminated printable base64 string,
 * suitable for use as filename.
 *
 * @param *ip - pointer to input data byte array
 * @param ic  - number of bytes of idata
 * @param *op - pointer to caller provided buffer for NUL terminated result
 * @param oc  - size of buffer provided for result 
 *      - buffer will not be overrun, but result will be truncated if too small
 *      - to avoid trucation, oc = (ic+2)/3))*4)+1;
 */
static void base64(unsigned char *ip, size_t ic, char *op, size_t oc)
{
    // 64 ascii chars that are safe in filenames
    uint32_t d;
    int pad1=0, pad2=0;

    if (!op || !oc || !ip || !ic) return;

    oc--; // leave room for NUL
    while (ic-- && oc--) {
        // input 1 to 3 bytes each with 8-bits of value
                  d  = ((uint32_t)*ip++) << 16;
        if (ic) { d |= ((uint32_t)*ip++) <<  8; ic--; } else pad2++;
        if (ic) { d |=  (uint32_t)*ip++       ; ic--; } else pad1++;
        // output 1 to 4 chars each representing 6-bits of value
                           *op++ = b64[(d >> 18 & 0x3F)]; oc--;
        if (oc)          { *op++ = b64[(d >> 12 & 0x3F)]; oc--; }
        if (oc && !pad2) { *op++ = b64[(d >>  6 & 0x3F)]; oc--; }
        if (oc && !pad1) { *op++ = b64[(d       & 0x3F)]; }
    }
    *op = '\0';   
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

    if ((elem = list->first)) {
        switch ((elemtype_t) elem->type) {
        case FRAGELEM:
            while (elem) {
                if (1 != EVP_DigestUpdate(ctx, ((frag_elem_t*)elem)->frag, ((frag_elem_t*)elem)->len))
                    fatal_perror("Error - EVP_DigestUpdate() ");
                elem = elem->next;
            }
            break;
        case LISTELEM:
            while (elem) {
                hash_list_r(ctx, elem);    // recurse
                elem = elem->next;
            }
            break;
        default:
            assert(0);  // should not be here
            break;
        }
    }
}

/**
 * open a named container
 *
 * if it already exists then it is opened ro then later,
 *        if written to, copied and reopened a+r before the write
 * else if it does not exist, then it is opened a+r
 */
ikea_t *ikea_open( context_t * C, elem_t * name )  
{
    ikea_t *ikea, **ikea_open_p;

    EVP_MD_CTX ctx = {0}; // context for namehash - don't need to keep around so use stack
    unsigned char digest[EVP_MAX_MD_SIZE];  // namehash digest 
    unsigned int digest_len = sizeof(digest); 
    char bucket;

    assert(name);

    // allocate handle
    if ((ikea = calloc(1, sizeof(ikea_t))) == NULL)
        fatal_perror("Error - calloc() ");

    // init namehash accumulation
    if (1 != EVP_DigestInit_ex(&ctx, EVP_sha1(), NULL))
        fatal_perror("Error - EVP_DigestInit_ex() ");

    //  accumulate namehash
    hash_list_r(&ctx, name);

    //  finalize namehash
    if (1 != EVP_DigestFinal_ex(&ctx, digest, &digest_len))
        fatal_perror("Error - EVP_DigestFinal_ex() ");

    //  convert to string suitable for filename
    base64(digest, digest_len, ikea->namehash, sizeof(ikea->namehash));

    // default mode for new files
    strcpy(ikea->mode,"a+r");

    //  see if this file already exists in the bucket list
    bucket = un_b64[ikea->namehash[0]];
    ikea_open_p = &(C->namehash_buckets[bucket]);
    while(*ikea_open_p) {
        if (strcmp((*ikea_open_p)->namehash, ikea->namehash) == 0)
            break;
    }
    if (*ikea_open_p) { // found - use state from bucket_list
        free (ikea);
        ikea = *ikea_open_p;
    }
    else { // append bucketlist
        *ikea_open_p = ikea;
    }

#if 0        
    fprintf(stdout, "%s %2x\n", ikea->namehash, bucket);
#endif

//    FIXME
//    compose full pathname:   <base>/name/namehash

    if (! ikea->fh ) { // if not already open
        // open the file
        if ((ikea->fh = fopen(ikea->namehash,ikea->mode)) == NULL)
            fatal_perror("Error - fopen() ");
        // init content hash accumulation
        if (1 != EVP_DigestInit_ex(&(ikea->ctx), EVP_sha1(), NULL))
            fatal_perror("Error - EVP_DigestInit_ex() ");
        //    FIXME
        //    log file open:   namehash path name
    }

    // return handle
    return ikea;
}

success_t ikea_append(ikea_t* ikea, unsigned char *data, size_t data_len)
{
    if (fwrite(data, data_len, 1, ikea->fh) != data_len)
        fatal_perror("Error - fwrite() ");
    if (1 != EVP_DigestUpdate(&(ikea->ctx), data, data_len))
        fatal_perror("Error - EVP_DigestUpdate() ");
    return SUCCESS;
}

success_t ikea_flush(ikea_t* ikea) 
{
    // content hash left in allocated ikea_t struct.
    return SUCCESS;
}

success_t ikea_close(ikea_t* ikea) 
{
    unsigned char digest[EVP_MAX_MD_SIZE];  // contenthash digest 
    unsigned int digest_len = sizeof(digest); 

    // close file
    if (fclose(ikea->fh))
        fatal_perror("Error - fclose() ");
    // finalize namehash
    if (1 != EVP_DigestFinal_ex(&(ikea->ctx), digest, &digest_len))
        fatal_perror("Error - EVP_DigestFinal_ex() ");
    
    // convert to string suitable for filename
    base64(digest, digest_len, ikea->contenthash, sizeof(ikea->contenthash));

    // FIXME  // contenthash is left in allocated ikea_t struct.
    // free(ikea);
    return SUCCESS;
}
