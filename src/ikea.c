/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <errno.h>
#include <zlib.h>
#include <libtar.h>
#include <fcntl.h>
#include <glob.h>
#include <assert.h>

#include "ikea.h"
#include "fatal.h"

static char *tempdir_template="/tmp/g_XXXXXX";
static char *tempfile_template="/g_XXXXXX";

#if defined ( _MSC_VER) || defined(__WATCOMC__)
#include <io.h>
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

/**
 * ikea.c   (the container store )
 *
 * Objective:  
 *        A container (ikea_box_t)  holds a single flat graph from a tree of graphs described by a 'g' file.
 *        A store (ikea_store_t) holds the set of containers corresponding to a g file, in a persistent
 *        tar.gz format.
 *
 *        This code maintains each container in a separate file, but only once in the case that contents
 *        of multiple containers match.
 *
 *        The "contents" of each container are hashed, and used as the file name. This allows
 *        duplicates to be stored just once.
 *
 *        When g is stopped, the container store is archived to a compressed tar file.
 *        When g is running, the tree is unpacked into a private tree.
 */

// private structs
struct ikea_store_s {
    char tempdir[sizeof(tempdir_template)+1];     // place to keep tempdir string for mkdtemp() to compose
};
struct ikea_box_s {
    ikea_box_t *next;
    ikea_store_t *ikea_store;
    FILE *fh;
    EVP_MD_CTX ctx;   // context for content hash accumulation
    char tempfile[sizeof(tempdir_template)+sizeof(tempfile_template)+1]; // place to keep template for mkstemp()
};

// forward b64 mapping table (6-bits of data to 7-bit ASCII)
const static char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

// reverse b64 mappinag (7-bit ASCII to 6-bits of data)
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
 * open a container
 */
ikea_box_t *ikea_box_open( ikea_store_t * ikea_store, const char *appends_content )
{
    ikea_box_t *ikea_box;
    int fd;

    assert(ikea_store);

    if ((ikea_box = calloc(1, sizeof(ikea_box_t))) == NULL)
        fatal_perror("Error - calloc() ");

    ikea_box->ikea_store = ikea_store;

    // construct temporary file path, in the temporary directory
    strcpy(ikea_box->tempfile, ikea_store->tempdir);
    strcat(ikea_box->tempfile, tempfile_template);

    // make a temporary file 
    if ((fd = mkstemp(ikea_box->tempfile)) == -1)
        fatal_perror("Error - mkstemp(): ");

    // convert to FILE*
    if ((ikea_box->fh = fdopen(fd, "w")) == NULL)
        fatal_perror("Error - fdopen(): ");

    if (1 != EVP_DigestInit_ex(&(ikea_box->ctx), EVP_sha1(), NULL))
        fatal_perror("Error - EVP_DigestInit_ex() ");

    // return handle
    return ikea_box;
}

void ikea_box_append(ikea_box_t* ikea_box, unsigned char *data, size_t data_len)
{
    if (fwrite(data, data_len, 1, ikea_box->fh) != data_len)
        fatal_perror("Error - fwrite() ");
    if (1 != EVP_DigestUpdate(&(ikea_box->ctx), data, data_len))
        fatal_perror("Error - EVP_DigestUpdate() ");
}

char * ikea_box_close(ikea_box_t* ikea_box) 
{
    unsigned char digest[EVP_MAX_MD_SIZE];  // contenthash digest 
    unsigned int digest_len = sizeof(digest); 
    static char contenthash[(((EVP_MAX_MD_SIZE+1)*8/6)+1)];
        // big enough for base64 encode largest possible digest, plus \0
    char contentpathname[sizeof(tempdir_template) +1 +sizeof(contenthash) +1];
    ikea_store_t * ikea_store = ikea_box->ikea_store;

    if (fclose(ikea_box->fh))
        fatal_perror("Error - fclose() ");
    ikea_box->fh = NULL;
    if (1 != EVP_DigestFinal_ex(&(ikea_box->ctx), digest, &digest_len))
        fatal_perror("Error - EVP_DigestFinal_ex() ");
    // convert to string suitable for filename
    base64(digest, digest_len, contenthash, sizeof(contenthash));

    //compose the new filename
    strcpy(contentpathname, ikea_store->tempdir);
    strcat(contentpathname, "/");
    strcat(contentpathname, contenthash);

    // rename the file
    if ((rename(ikea_box->tempfile, contentpathname)) == -1)
        fatal_perror("Error - rename() ");

    // free resources
    free(ikea_box);

    return contenthash;  // return the contenthash string - good until next call
}


//  FIXME - avoid globals
static gzFile gzf;

/**
 * open a gz compressed file
 *
 * @param pathname
 * @param oflags
 * @param VARARGS
 * @return filedescriptor, or -1=fail
 */
static int ikea_gzopen(const char *pathname, int oflags, ...)
{
    char *gzoflags;
    int fd;
    mode_t mode;
    va_list ap;

    // PITA !
    va_start (ap, oflags);
    mode = va_arg(ap, mode_t);
    va_end(ap);

    switch (oflags & O_ACCMODE) {
    case O_WRONLY:
        gzoflags = "wb";
        break;
    case O_RDONLY:
        gzoflags = "rb";
        break;
    default:
        case O_RDWR:
        errno = EINVAL;
        return -1;
    }

    fd = open(pathname, oflags, mode);

    if (fd == -1) {
        return -1;
    }

#if defined(__BEOS__) && !defined(__ZETA__)  /* no fchmod on BeOS...do pathname instead. */
    if ((oflags & O_CREAT) && chmod(pathname, mode & 07777)) {
        return -1;
    }
#elif !defined(_WIN32) || defined(__CYGWIN__)
    if ((oflags & O_CREAT) && fchmod(fd, mode & 07777)) {
        return -1;
    }
#endif

    gzf = gzdopen(fd, gzoflags);
    if (!gzf) {
        errno = ENOMEM;
        return -1;
    }

    return fd;
}

/**
 * close a gz compressed file
 *
 * @param fd (but not used - uses gzf global instead)
 * @return -1 = fail
 */
static int ikea_gzclose(int fd)
{
    (void) fd; // NOTUSED
    return gzclose(gzf);
}

/**
 * read from a gz compressed file
 * 
 * @param fd  open file descriptor (not used - uses gzf global instead)
 * @param buf caller provided buffer pointer to next available pos 
 * @param count available space
 * @return characters read into buffer 
 */
static ssize_t ikea_gzread(int fd, void* buf, size_t count)
{
    (void) fd; // NOTUSED
    return gzread(gzf, buf, (unsigned int)count);
}

/**
 * write to a gz compressed file
 * 
 * @param fd  open file descriptor (not used - uses gzf global instead)
 * @param buf caller provided buffer pointer to next pos 
 * @param count available for write
 * @return characters written
 */
static ssize_t ikea_gzwrite(int fd, const void* buf, size_t count)
{
    (void) fd; // NOTUSED
    return gzwrite(gzf, (void*)buf, (unsigned int)count);
}

// discipline
static tartype_t gztype = { 
    ikea_gzopen,
    ikea_gzclose,
    ikea_gzread,
    ikea_gzwrite
};

/**
 * open a gz compressed tar file for a set of per-container graphs
 *
 * @param C context
 * @return list of containers ??
 */
ikea_store_t * ikea_store_open( const char * oldstore )
{
    ikea_store_t * ikea_store;
    int fd;
    char *td;

    // allocate a new store
    if ((ikea_store = calloc(1, sizeof(ikea_store_t))) == NULL)
        fatal_perror("Error - calloc() ");

    strcpy(ikea_store->tempdir, tempdir_template);

    // make a temporary directory 
    if ((td = mkdtemp(ikea_store->tempdir)) == NULL)
        fatal_perror("Error - mkdtemp(): ");

    return ikea_store;
}

// snapshot of temporary files
void ikea_store_snapshot ( ikea_store_t * ikea_store )
{
    TAR *pTar;
    char *tarFilename = "g_snapshot.tgz";
    char *extractTo = ".";
#if 0
    int i;
    elem_t *elem, *next;
    FILE *fp;

//============================= ikea flush all ===============

    ikea_box_t *ikea_box;

    for (i=0; i<64; i++) {
        ikea_box = C->namehash_buckets[i];
        while(ikea_box) {
            ikea_box_flush(ikea_box);
            ikea_box = ikea_box->next;
        }
    }

//============================= old flush ================
    // flush all open files
    for (i=0; i<64; i++) {
        next = C->hash_buckets[i];
        while(next) {
            elem = next;
            next = elem->next;
            if ((fp = ((hash_elem_t*)elem)->out)) {
                if (fflush(fp))
                    fatal_perror("Error - fflush(): ");
            }
        }
//=======================================================
    }
#endif

    if (tar_open(&pTar, tarFilename, &gztype, O_WRONLY | O_CREAT | O_TRUNC, 0600, TAR_GNU) == -1)
        fatal_perror("Error - tar_open(): ");
    if (tar_append_tree(pTar, ikea_store->tempdir, extractTo) == -1)
        fatal_perror("Error - tar_append_tree(): ");
    if (tar_append_eof(pTar) == -1)
        fatal_perror("Error - tar_append_eof(): ");
    if (tar_close(pTar) == -1)
        fatal_perror("Error - tar_close(): ");
}

/**
 * error handler for glob errors
 *
 * @param epath 
 * @param eerrno
 * @return -1 fail
 */
static int glob_err (const char *epath, int eerrno)
{
    fprintf(stderr,"Error - glob(): \"%s\" %s\n", epath, strerror(eerrno));
    return -1;
}

/**
 * restore from snapshot
 * 
 * @param C context
 */
void ikea_store_restore ( ikea_store_t * ikea_store )
{
#if 0
    TAR *pTar;
    glob_t pglob;
    int rc;
    size_t len, pathc;
    char *tarFilename = "g_snapshot.tgz";
    char *glob_pattern;
    unsigned long hash;

    if (tar_open(&pTar, tarFilename, &gztype, O_RDONLY, 0600, TAR_GNU) == -1)
        fatal_perror("Error - tar_open(): ");
    if (tar_extract_all(pTar, C->tempdir) == -1)
        fatal_perror("Error - tar_extract_all(): ");
    if (tar_close(pTar) == -1)
        fatal_perror("Error - tar_close(): ");
    len = strlen(C->tempdir);
    if ((glob_pattern = malloc(len + 2)) == NULL)
        fatal_perror("Error - malloc(): ");
    strcpy(glob_pattern, C->tempdir);
    strcat(glob_pattern, "/*");
    if ((rc = glob(glob_pattern, 0, glob_err, &pglob))) {
        switch (rc) {
        case GLOB_NOSPACE:
            fprintf(stderr,"Error - glob(): no memory available (GLOB_NOSPACE)");
            break;
        case GLOB_ABORTED:
            fprintf(stderr,"Error - glob(): read error (GLOB_ABORTED)");
            break;
        case GLOB_NOMATCH:
            fprintf(stderr,"Error - glob(): no matches found (GLOB_NOMATCH)");
            break;
        }
        exit(EXIT_FAILURE);
    }
    for (pathc=0; pathc < pglob.gl_pathc; pathc++) {
        if (je_base64_to_long(pglob.gl_pathv[pathc]+len+1, &hash) == FAIL) {
            fprintf(stderr, "Error - je_base64_to_long():  invalid base64 name \"%s\"\n",
                    pglob.gl_pathv[pathc]+len+1);
            exit(EXIT_FAILURE);
        }
        je_hash_bucket(C, hash); // reinsert into bucket list.
    }
    free(glob_pattern);
    globfree(&pglob);
#endif
}

/**
 * close snapshot storage - cleanup of temporary files
 *
 * @param C context
 */
void ikea_store_close ( ikea_store_t * ikea_store )
{
#if 0
    FILE *fp;
    int i;
    elem_t *elem, *next;
    char hashname[12], *filename;

//========================== ikea close all ==========================
    ikea_box_t * ikea_box, **ikea_box_p;

    for (i=0; i<64; i++) {
        ikea_box_p = &(C->namehash_buckets[i]);
        while (*ikea_box_p) {
            ikea_box = *ikea_box_p;
            *ikea_box_p = ikea_box->next;  // get next before freeing this one
            ikea_box_close(ikea_box);  // frees ikea_box
        }
    }
//========================== old close ===========================
    for (i=0; i<64; i++) {
        next = C->hash_buckets[i];
        while(next) {
            elem = next;
            next = elem->next;
            if ((fp = ((hash_elem_t*)elem)->out)) {

                // close all open files
                if (fclose(fp))
                    fatal_perror("Error - fclose(): ");
                // FIXME - perhaps we should keep the base64 filenames around?
 
                // reconsitute the filename and unlink
                je_long_to_base64(hashname, &(((hash_elem_t*)elem)->hash));
                if (! (filename = malloc(sizeof(C->template) + 1 + sizeof(hashname) + 1)))
                    fatal_perror("Error - malloc(): ");
                strcpy(filename, C->tempdir);
                strcat(filename, "/");
                strcat(filename, hashname);
                
                // rm all output files
                if (unlink(filename) == -1)
                    fatal_perror("Error - unlink(): ");
                free(filename);
            }

            // return hash_elem to free_elem_list
            elem->next = C->free_elem_list;
            C->free_elem_list = elem;
        }
    }
    free_list(C, &(C->myname));
//================================================================
#endif

#if 0
// FIXME - remove content of temporary directory

    // rmdir the temporary directory
    if (rmdir(ikea_store->tempdir) == -1)
        fatal_perror("Error - rmdir(): ");
#endif

    free(ikea_store);
}
