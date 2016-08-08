/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <inttypes.h>
#include <openssl/evp.h>

// link with -lcrypto

/**
 * ikea.c   (the container store )
 *
 * Objective:  
 *        A container holds a single graph in a tree of graph containers.
 *
 *        This code maintains each container in a separate file.
 *
 *        The file is hardlinked, a single content file is named by the hash
 *        of the content,  then, possibly multiple, name files a named by the
 *        hash of the name {node, edge, pattern).
 *
 *        Separate directories are used for name and content hashes:
 *            - content/contenthash
 *            - name/namehash1
 *            - name/namehash2
 *            - name/namehash3
 *                      ...
 */




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

typdef struct IKEA {
    FILE *fh;
    EVP_MD_CTX *ctx;
}


IKEA* ikea_open( elem_t * name )  
{
    return SUCCESS;
}

success_t ikea_write(IKEA* ikea, unsigned char *data, size_t data_len)
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
