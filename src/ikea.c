/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <openssl/evp.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <zlib.h>
#include <libtar.h>
#include <fcntl.h>
#include <glob.h>
#include <assert.h>

#include "libje_private.h"

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
 *        A container holds a single graph in a tree of graph containers.
 *
 *        This code maintains each container in a separate file.
 *
 *        During file write (before the contenthash has been determined),  it is kept in namehash.temp.
 *
 *        At close (or flush) the file is moved (or copied) to contenthash.g
 *        where contenthash is the hash of the contents.
 *        Then hardlinked to namehash.name
 *        Possibly multiple, namehash.name will be hardlinked to the same contenthash.g
 *        name {node, edge, pattern) are hardlinked to the master..
 *
 *        A container store is a file collection of all the containers from a tree of graph containers
 *        described by a 'g' file
 *
 *        When g is stopped, the container store is archived to a compressed tar file.
 *        When g is running, the tree is unpacked into a private tree under $HOME/.g/`pid`/              
 */

typedef enum {IKEA_READ, IKEA_WRITE} ikea_mode_t;

// private struct
struct ikea_s {
    ikea_t *next;
    FILE *fh;
    EVP_MD_CTX ctx;   // context for content hash accumulation
    ikea_mode_t mode;
    char namehash[(((EVP_MAX_MD_SIZE+1)*8/6)+1)];
        // big enough for base64 encode largest possible digest, plus \0
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
 * Recursive hash accumulation of a fraglist, or list of fraglists
 *
 * @param ctx  - hashing context
 * @param list - fraglist or list of fraglist to be hashed
 */
static void hash_list_r(EVP_MD_CTX *ctx, elem_t *list)
{
    elem_t *elem; 
    frag_elem_t *fragelem; 

    if ((elem = list->first)) {
        switch ((elemtype_t) elem->type) {
        case FRAGELEM:
            while (elem) {
                fragelem = (frag_elem_t*) elem;
                if (1 != EVP_DigestUpdate(ctx, fragelem->frag, fragelem->len))
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
 * compute namehash
 *
 * if it already exists in namehash_bucketlist
 *    use it from bucketlist with its current state
 * else it does not exist
 *    add to bucket list, and use
 *
 * fopen is defered until first ikea_append()
 *   (or ikea_read() ??? )
 */
ikea_t *ikea_open( context_t * C, elem_t * name )  
{
    ikea_t *ikea, **ikea_open_p;

    EVP_MD_CTX ctx = {0}; // context for namehash 
                          // - don't need to keep around so use stack
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


    // return handle
    return ikea;
}

void ikea_append(ikea_t* ikea, unsigned char *data, size_t data_len)
{
    if (ikea->fh && ikea->mode == IKEA_READ ) { // already open, but for read
        if (fclose(ikea->fh))
            fatal_perror("Error - fclose() ");
        ikea->fh = NULL;
    }
    if (! ikea->fh ) { // if not already open

// FIXME
// compose full pathname: 
//      <ndir> - un_b64[<namehash[0]>]
//      <basedir>/<ndir>/namehash.temp
//      mkdir -p <basedir>/<ndir>

 
        ikea->mode = IKEA_WRITE;  // we're opening for write
        if ((ikea->fh = fopen(ikea->namehash,"a+r")) == NULL)
            fatal_perror("Error - fopen() ");
        // init content hash accumulation
        if (1 != EVP_DigestInit_ex(&(ikea->ctx), EVP_sha1(), NULL))
            fatal_perror("Error - EVP_DigestInit_ex() ");
        //    FIXME  - log file open:   namehash path name
    }
    if (fwrite(data, data_len, 1, ikea->fh) != data_len)
        fatal_perror("Error - fwrite() ");
    if (1 != EVP_DigestUpdate(&(ikea->ctx), data, data_len))
        fatal_perror("Error - EVP_DigestUpdate() ");
}

void ikea_flush(ikea_t* ikea) 
{
    if (ikea->fh && ikea->mode == IKEA_WRITE) {
        if (fflush(ikea->fh))
            fatal_perror("Error - fflush() ");
    }
}

void ikea_close(ikea_t* ikea) 
{
    unsigned char digest[EVP_MAX_MD_SIZE];  // contenthash digest 
    unsigned int digest_len = sizeof(digest); 
    char contenthash[(((EVP_MAX_MD_SIZE+1)*8/6)+1)];
        // big enough for base64 encode largest possible digest, plus \0

    if (ikea->fh) { // if file was ever opened
        // close file
        if (fclose(ikea->fh))
            fatal_perror("Error - fclose() ");
        ikea->fh = NULL;
        if (ikea->mode == IKEA_WRITE) { // if it was opened for write
            // finalize namehash
            if (1 != EVP_DigestFinal_ex(&(ikea->ctx), digest, &digest_len))
                fatal_perror("Error - EVP_DigestFinal_ex() ");
            // convert to string suitable for filename
            base64(digest, digest_len, contenthash, sizeof(contenthash));
#if 1        
            fprintf(stdout, "%s\n", contenthash);
#endif

// FIXME

// compose full pathnames:
//        <basedir>/<ndir>/namehash.temp
//        <basedir>/<ndir>/namehash.name
//        (<basedir>/ndir>/ already exists)
//
//        <basedir>/<cdir>/contenthash.g
//        mkdir -p <basedir>/<cdir>


//  if contenthash_file exists
//       rm temp/namehash -- i.e. the one we just closed
//  else
//       mv temp/namehash cont/contenthash
//       -- unlink any old namehash
//       -- if last link to oldcontenthash - remove contenthash (by finding inode)
//  fi
//  hardlink /cont/contenthash name/namehash
//  log contenthas -> namehash mapping

        }
    }
    free(ikea);
}


//  FIXME
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
elem_t * ikea_persist_open(context_t *C)
{
    int i;
    char *template_init = "/tmp/g_XXXXXX";

    // The top name is simply '^' - like "Dad" (or "Mum", or better: "Parent")
    // Note that adoption is possible as we never use an actual name.

    // 66 bytes for a 1-byte name - oh well 
    static frag_elem_t mumfrag = {
        .next = NULL,
        .inbuf = NULL,
        .frag = (unsigned char *)"^",
        .len = 1,
        .type = FRAGELEM,
        .state = ABC
    };
    static elem_t Mum = {
        .next = NULL,
        .first = (elem_t*)(&mumfrag),
        .last = (elem_t*)(&mumfrag),
        .refs = 0,
        .type = LISTELEM,
        .state = STRING
    };

    // copy template including trailing NULL
    i = 0;
    while ((C->template[i] = template_init[i])) {
        i++;
    }

    // make a temporary directory (probably based on pid - no semantic significance here
    C->tempdir = mkdtemp(C->template);
    if (!C->tempdir)
        fatal_perror("Error - mkdtemp(): ");

    return &Mum;
}

// snapshot of temporary files
void ikea_persist_snapshot (context_t *C)
{
    int i;
    elem_t *elem, *next;
    FILE *fp;
    TAR *pTar;
    char *tarFilename = "g_snapshot.tgz";
    char *extractTo = ".";

//============================= ikea flush all ===============

    ikea_t *ikea;

    for (i=0; i<64; i++) {
        ikea=C->namehash_buckets[i];
        while(ikea) {
            ikea_flush(ikea);
            ikea = ikea->next;
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

    if (tar_open(&pTar, tarFilename, &gztype, O_WRONLY | O_CREAT | O_TRUNC, 0600, TAR_GNU) == -1)
        fatal_perror("Error - tar_open(): ");
    if (tar_append_tree(pTar, C->tempdir, extractTo) == -1)
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
void ikea_persist_restore (context_t *C)
{
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
}

/**
 * close snapshot storage - cleanup of temporary files
 *
 * @param C context
 */
void ikea_persist_close (context_t *C)
{
    FILE *fp;
    int i;
    elem_t *elem, *next;
    char hashname[12], *filename;

//========================== ikea close all ==========================
    ikea_t * ikea, **ikea_p;

    for (i=0; i<64; i++) {
        ikea_p = &(C->namehash_buckets[i]);
        while (*ikea_p) {
            ikea = *ikea_p;
            *ikea_p = ikea->next;  // get next before freeing this one
            ikea_close(ikea);  // frees ikea
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
//================================================================

    // rmdir the temporary directory
    if (rmdir(C->tempdir) == -1)
        fatal_perror("Error - rmdir(): ");

    free_list(C, &(C->myname));
}
