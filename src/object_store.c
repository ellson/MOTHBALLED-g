#include <stdio.h>
#include <inttypes.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bio.h>

// link with -lcrypto


/**
 * Encode a byte array to a NUL terminated printable base64 string,
 * (suitable for filename).
 *
 * NB.  Non-std character set and does not pad with '='
 *
 * @param *buf - caller provided buffer for NUL terminated result
 * @param buf_len - size of buf provided for result 
 *      - buf will not be overrun, but result will be truncated if too small
 *      - to avoid trucation, buf_len = (data_len+2)/3))*4)+1;
 * @param data - input data byte array
 * @param data_len - number of bytes of data
 */
void je_base64(char *buf, const int buf_len, const unsigned char *data, const int data_len)
{
    // 64 ascii chars that are safe in filenames
    const static char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";
    int i, j;
    uint32_t d;
    int len=buf_len-1; /* leave room for NUL terminator */
    char *p = buf;

    for (i = 0; i < data_len; ) {
      d = (uint32_t)data[i++];
      if (i < data_len) d |= ((uint32_t)data[i++])<<8;
      if (i < data_len) d |= ((uint32_t)data[i++])<<16;
      for (j = len/4?4:len; j--;) {
          *p++ = b64[(d & 0x3F)];
          d >>= 6;
      }
      len -= 4;
    }
    *p = '\0';
}


int main(int arc, char *argv[])
{ 
     /* Load the human readable error strings for libcrypto */
     ERR_load_crypto_strings();

     /* Load all digest and cipher algorithms */
     OpenSSL_add_all_algorithms();

     /* Load config file, and other important initialisation */
     OPENSSL_config(NULL);

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


    BIO *bio, *b64;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stdout, BIO_NOCLOSE);
    BIO_push(b64, bio);
    BIO_write(b64, digest, digest_len);
    BIO_flush(b64);

    BIO_free_all(b64);

    char buf[64];

    je_base64(buf, sizeof(buf), digest, digest_len);

    fprintf(stdout, "%s\n", buf);



    EVP_MD_CTX_destroy(ctx);


    /* Removes all digests and ciphers */
    EVP_cleanup();

    /* if you omit the next, a small leak may be left when you make use of the BIO (low level API) for e.g. base64 transformations */
    CRYPTO_cleanup_all_ex_data();

    /* Remove error strings */
    ERR_free_strings();

    return 0;
}
