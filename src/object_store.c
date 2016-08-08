#include <stdio.h>
#include <inttypes.h>
//#include <openssl/conf.h>
#include <openssl/evp.h>
// #include <openssl/err.h>
//#include <openssl/bio.h>

// link with -lcrypto


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

    EVP_MD_CTX myctx = {0};
    EVP_MD_CTX *ctx = &myctx;

//    if((ctx = EVP_MD_CTX_create()) == NULL)
//        handleErrors();

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


    char buf[64];

    base64(digest, digest_len, buf, sizeof(buf));

    fprintf(stdout, "%s\n", buf);


//    EVP_MD_CTX_destroy(ctx);

    return 0;
}
