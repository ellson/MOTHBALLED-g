/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <inttypes.h>
#include <openssl/evp.h>

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


// Forward declarations
static void base64(unsigned char *ip, size_t ic, char *op, size_t oc);
static void hash_list_r(unsigned long *hash, elem_t *list);


#define handleErrors abort

typdef struct {
    FILE *fh;
    EVP_MD_CTX *ctx;
    char namehash[64];
    char contenthash[64];
} ikea_t;

/**
 * open a named container for write
 */
ikea_t* ikea_open( elem_t * name )  
{
    ikea_t *ikea;

    assert(name);

// allocate handle
    if ((ikea = malloc(sizeof(ikea_t))) == NULL)
        handle_errors()

// init namehash accumulation
    if((ikea->ctx = EVP_MD_CTX_create()) == NULL)
        handleErrors();
    if(1 != EVP_DigestInit_ex(ikea->ctx, EVP_sha1(), NULL))
        handleErrors();

// while name fragments ...
//        accumulate namehash
    hash_list_r(ikea, name);

//    finalize namehash

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = sizeof(digest);

    if(1 != EVP_DigestFinal_ex(ikea->ctx, digest, &digest_len))
        handleErrors();

//    convert to string suitable for filename
    char buf[64];

    base64(digest, digest_len, buf, sizeof(buf));

    fprintf(stdout, "%s\n", buf);

//    compose full pathname


    if ((ikea->fh = fopen(ikea->namehash,"a+b")) == NULL)
        handleErrors();



    // clean up from namehash generation
    EVP_MD_CTX_destroy(ctx);


// init content hash accumulation
//
//
//
//

// return handle
    return ikea;
}

success_t ikea_append(IKEA* ikea, unsigned char *data, size_t data_len)
{
    return SUCCESS;
}

success_t ikea_flush(IKEA* ikea) 
{
    // content hash left in allocated IKEA struct.
    return SUCCESS;
}

success_t ikea_close(IKEA* ikea) 
{
    // content hash left in allocated IKEA struct.
    return SUCCESS;
}


//================= support functions ====================

/**
 * Encode a byte array to a NUL terminated printable base64 string,
 * suitable for use as filename.
 *
 * NB.  c.v standard base64:
 *        - Non-std character set ('_' instead of '/')
 *        - Does not pad with '='
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
    const static char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";
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
 * Recursive hash function building on a previously initialized or
 * partially accumulated haahs result.
 *
 * @param hash place for resulting hash
 * @param list - fraglist or list of fraglist to be hashed
 */
static void hash_list_r(unsigned long *hash, elem_t *list)
{
    elem_t *elem; 
    unsigned char *cp;
    int len;

    if ((elem = list->u.list.first)) {
        switch ((elemtype_t) elem->type) {
        case FRAGELEM:
            while (elem) {
                cp = elem->u.frag.frag;
                len = elem->v.frag.len;
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
        case HASHELEM:
            assert(0);  // should not be here
            break;
        }
    }
}



//===================  test code =================================

int main(int arc, char *argv[])
{ 
     /* Load the human readable error strings for libcrypto */
//     ERR_load_crypto_strings();

     /* Load all digest and cipher algorithms */
//     OpenSSL_add_all_algorithms();

     /* Load config file, and other important initialisation */
//     OPENSSL_config(NULL);

     /* ... Do some crypto stuff here ... */


#define handleErrors abort

    EVP_MD_CTX *ctx;

    if((ctx = EVP_MD_CTX_create()) == NULL)
        handleErrors();

    if(1 != EVP_DigestInit_ex(ctx, EVP_sha1(), NULL))
        handleErrors();

    if(1 != EVP_DigestUpdate(ctx, "Hello, ", 7))
        handleErrors();

    if(1 != EVP_DigestUpdate(ctx, "World!\n", 7))
        handleErrors();

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = sizeof(digest);

    if(1 != EVP_DigestFinal_ex(ctx, digest, &digest_len))
        handleErrors();


//    BIO *bio, *b64;

//    b64 = BIO_new(BIO_f_base64());
//    bio = BIO_new_fp(stdout, BIO_NOCLOSE);
//    BIO_push(b64, bio);
//    BIO_write(b64, digest, digest_len);
//    BIO_flush(b64);
//
//    BIO_free_all(b64);

    char buf[64];

    je_base64(digest, digest_len, buf, sizeof(buf));

    fprintf(stdout, "%s\n", buf);



    EVP_MD_CTX_destroy(ctx);


    /* Removes all digests and ciphers */
//    EVP_cleanup();

    /* if you omit the next, a small leak may be left when you make use of the BIO (low level API) for e.g. base64 transformations */
//:w
//CRYPTO_cleanup_all_ex_data();

    /* Remove error strings */
//    ERR_free_strings();

    return 0;
}


/*
 pseudocode for maintaining shared containers


open named_object
    init namehash accumulation
    while name fragments
        accumulate namehash
    finalize namehash
    convert to string suitable for filename
    open temp_namehash

    init content hash accumulation
    return handle

write handle acts
    write act to temp_namehash
    accumulate contenthash

close handle tempfile
    close temp namehash

    finalize contenthash
       convert to string suitable for filename

    if contenthash exists
	    rm tempfile
    else
	    mv tempfile contenthash
    fi

    if namehash exists
        error if not a link
        get oldcontenthash from namehash->oldcontenthash (ls -i namehash; find . -inum <inum>)
        rm namehash
        if oldcontenthash != contenthash
            decrement refcount to oldcontenthash
            if refount == 0
                rm oldcontenthash
                rm all derivatives of oldcontenthash
            fi
        fi
    fi
    ln contenthash namehash



- stat() will give the inode of a namehash, and the number of hardlinks

- if all the contenthash are in one directory,  then searching with readdir() can find
  the contenthash from the inode
        - expensive ... every close where a namehash already exists require and iteration of all
		contents directory entries  (i.e. thru number of different containers)

	-- minimize cost by using scandir()
     

        names/namehash files
	contents/contenthash files   << hardlinked from namehash files
	derivatives/contenthash deivative files
            

- if softlinks were used instead then would need to implement reference counting outside of the file system


- ok, so using scandir() is likely low cost when compared to generating hashes...

*/
