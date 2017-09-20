/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <openssl/evp.h>
#include <errno.h>
#include <zlib.h>
#include <libtar.h>
#include <fcntl.h>
#include <glob.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined ( _MSC_VER) || defined(__WATCOMC__)
#include <io.h>
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "io.h"
#include "ikea.h"

/**
 * ikea.c   (the container store )
 *
 * Objective:  
 *        A container (ikea_box_t)  holds a single flat graph from
 *        a tree of graphs described by a 'g' file.
 *
 *        A store (ikea_store_t) holds the set of containers correspondings
 *        to a g file, in a persistent tar.gz format.
 *
 *        This code maintains each container in a separate file, but
 *        only once in the case that contents of multiple containers match.
 *
 *        The "contents" of each container are hashed, and used as the
 *        file name. This allows duplicates to be stored just once.
 *
 *        When g is stopped, the container store is archived
 *        to a compressed tar file.
 *        When g is running, the tree is unpacked into a private tree.
 */

static char tempdir_template[]="/tmp/g_XXXXXX";
static char tempfile_template[]="/g_XXXXXX";

// private structs
struct ikea_store_s {
    char tempdir[sizeof(tempdir_template)+1];     // place to keep tempdir string for mkdtemp() to compose
};

struct ikea_box_s {
    ikea_box_t *next;
    ikea_store_t *ikea_store;
    FILE *fh;
    char tempfile[sizeof(tempdir_template)+sizeof(tempfile_template)+1]; // place to keep template for mkstemp()
    EVP_MD_CTX *ctx;
};

// forward b64 mapping table (6-bits of data to 7-bit ASCII)
//   - the encoding is suitable for use as filenames.
const static char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

#if 0  //unused
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
#endif

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
    int pad1=0, pad2=0;

    if (!op || !oc || !ip || !ic) return;

    oc--; // leave room for NUL
    while (ic-- && oc--) {
        uint32_t d;
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
 * open a new box in the ikea store in which to write content
 *
 * @param ikea_store - point to store to hold the box
 * @param mode - currently ignored  ( for arglist compat with fopen() )
 * @return - pointer to new open box
 */
static ikea_box_t* ikea_box_open( ikea_store_t * ikea_store, const char *mode )
{
    ikea_box_t *ikea_box;
    int fd;

    assert(ikea_store);

    if ((ikea_box = calloc(1, sizeof(ikea_box_t))) == NULL)
        FATAL("calloc()");

#ifndef HAVE_EVP_MD_CTX_NEW
    if ((ikea_box->ctx = calloc(1, sizeof(EVP_MD_CTX))) == NULL)
        FATAL("calloc()");
#else
    if ((ikea_box->ctx = EVP_MD_CTX_new()) == NULL)
        FATAL("EVP_MD_CTX_new()");
#endif
    ikea_box->ikea_store = ikea_store;

    // construct temporary file path, in the temporary directory
    strcpy(ikea_box->tempfile, ikea_store->tempdir);
    strcat(ikea_box->tempfile, tempfile_template);

    // make a temporary file 
    if ((fd = mkstemp(ikea_box->tempfile)) == -1)
        FATAL("mkstemp(%s)", ikea_box->tempfile);

    // convert to FILE*
    if ((ikea_box->fh = fdopen(fd, "w")) == NULL)
        FATAL("fdopen()");

    // initialize contenthash accumulation
    if ((EVP_DigestInit_ex(ikea_box->ctx, EVP_sha1(), NULL)) != 1)
        FATAL("EVP_DigestInit_ex()");

    // return handle
    return ikea_box;
}

static void ikea_box_append(ikea_box_t* ikea_box, const unsigned char *data, size_t data_len)
{
    if (fwrite(data, 1, data_len, ikea_box->fh) != data_len)
        FATAL("fwrite()");
    if ((EVP_DigestUpdate(ikea_box->ctx, data, data_len)) != 1)
        FATAL("EVP_DigestUpdate()");
}


// FIXME - probably just ikea_box_close needs LOCKs because of the rename operation.

static void ikea_box_close(ikea_box_t* ikea_box, char *contenthash, int contenthashsz) 
{
    assert(SIZEOFHASH >= (((EVP_MAX_MD_SIZE+1)*8/6)+1));   // =87, last time I checked

    unsigned char digest[EVP_MAX_MD_SIZE];  // contenthash digest 
    unsigned int digest_len = sizeof(digest); 
//    fprintf(stderr,"(((EVP_MAX_MD_SIZE+1)*8/6)+1) = %d\n", (((EVP_MAX_MD_SIZE+1)*8/6)+1));
//    fprintf(stderr,"sizeof(sizeof(tempdir_template)) = %ld\n", sizeof(tempdir_template));
//    fprintf(stderr,"SIZEOFHASH = %d\n", SIZEOFHASH);
//    fprintf(stderr,"sizeof(contenthash) = %ld\n", sizeof(contenthash));
//    fprintf(stderr,"contenthashsz = %d\n", contenthashsz);
    char contentpathname[sizeof(tempdir_template) +1 +SIZEOFHASH +2 +1];
    ikea_store_t * ikea_store = ikea_box->ikea_store;

    if (fclose(ikea_box->fh))
        FATAL("fclose()");
    if ((EVP_DigestFinal_ex(ikea_box->ctx, digest, &digest_len)) != 1)
        FATAL("EVP_DigestFinal_ex()");
    // convert to string suitable for filename
    base64(digest, digest_len, contenthash, contenthashsz);

    //compose the new filename
    strcpy(contentpathname, ikea_store->tempdir);
    strcat(contentpathname, "/");
    strcat(contentpathname, contenthash);
    strcat(contentpathname, ".g");

    // rename the file
    if ((rename(ikea_box->tempfile, contentpathname)) == -1)
        FATAL("rename()");

    // free resources
    free(ikea_box);
}

/**
 * open an existing ikea_box for regular file reading
 *
 * NB: use regular fread(), fclose(), on handle after opening
 *
 * @param ikea_store - pointer to store that holds the box
 * @param contenthash - the name of the box
 * @param mode - currently ignored  ( for arglist compat with fopen() )
 * @return - FILE pointer to opened box (or NULL if error)
 */
FILE* ikea_box_fopen( ikea_store_t * ikea_store, const char *contenthash, const char *mode )
{
    char contentpathname[sizeof(tempdir_template) +1 +SIZEOFHASH +2 +1];

    // compose the file path
    strcpy(contentpathname, ikea_store->tempdir);
    strcat(contentpathname, "/");
    strcat(contentpathname, contenthash);
    strcat(contentpathname, ".g");

    return (fopen(contentpathname, "r"));
}

//  FIXME - avoid globals - not while this is only used in the main thread, in process()
static gzFile gzf;

/**
 * open a gz compressed file
 *
 * @param pathname of ikea_store tgz to be opened
 * @param oflags
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
 * @param oldstore
 * @return an opened ikea_store
 */
ikea_store_t * ikea_store_open( const char * oldstore )
{
    ikea_store_t * ikea_store;
    char *td;

    // FIXME - what are we doing with oldstore ?
 
    // allocate a new store
    if ((ikea_store = calloc(1, sizeof(ikea_store_t))) == NULL)
        FATAL("calloc() ");

    strcpy(ikea_store->tempdir, tempdir_template);

    // make a temporary directory 
    if ((td = mkdtemp(ikea_store->tempdir)) == NULL)
        FATAL("mkdtemp()");

    return ikea_store;
}

// snapshot of temporary files
void ikea_store_snapshot ( ikea_store_t * ikea_store )
{
    TAR *pTar;
    char *tarFilename = "g_snapshot.tgz";
    char *extractTo = ".";

    if (tar_open(&pTar, tarFilename, &gztype, O_WRONLY | O_CREAT | O_TRUNC, 0600, TAR_GNU) == -1)
        FATAL("tar_open()");
    if (tar_append_tree(pTar, ikea_store->tempdir, extractTo) == -1)
        FATAL("tar_append_tree()");
    if (tar_append_eof(pTar) == -1)
        FATAL("tar_append_eof()");
    if (tar_close(pTar) == -1)
        FATAL("tar_close()");
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
    fprintf(stderr,"glob(): \"%s\" %s\n", epath, strerror(eerrno));
    return -1;
}

/**
 * restore from snapshot
 * 
 * @param ikea_store context
 */
void ikea_store_restore ( ikea_store_t * ikea_store )
{
    TAR *pTar;
    glob_t pglob;
    int rc;
    size_t len, pathc;
    char *tarFilename = "g_snapshot.tgz";
    char *glob_pattern;
#if 0
    uint64_t hash;
#endif

    if (tar_open(&pTar, tarFilename, &gztype, O_RDONLY, 0600, TAR_GNU) == -1)
        FATAL("tar_open()");
    if (tar_extract_all(pTar, ikea_store->tempdir) == -1)
        FATAL("tar_extract_all()");
    if (tar_close(pTar) == -1)
        FATAL("tar_close()");
    len = strlen(ikea_store->tempdir);
    if ((glob_pattern = malloc(len + 2)) == NULL)
        FATAL("malloc()");
    strcpy(glob_pattern, ikea_store->tempdir);
    strcat(glob_pattern, "/*");
    if ((rc = glob(glob_pattern, 0, glob_err, &pglob))) {
        switch (rc) {
        case GLOB_NOSPACE:
            FATAL("glob(): no memory available (GLOB_NOSPACE)");
            break;
        case GLOB_ABORTED:
            FATAL("glob(): read error (GLOB_ABORTED)");
            break;
        case GLOB_NOMATCH:
            FATAL("glob(): no matches found (GLOB_NOMATCH)");
            break;
        }
        exit(EXIT_FAILURE);
    }
    for (pathc=0; pathc < pglob.gl_pathc; pathc++) {
#if 0
        if (base64_to_long(pglob.gl_pathv[pathc]+len+1, &hash) == FAIL) {
            fprintf(stderr, "Error - base64_to_long():  invalid base64 name \"%s\"\n",
                    pglob.gl_pathv[pathc]+len+1);
            exit(EXIT_FAILURE);
        }
        hash_bucket(PARSE, hash); // reinsert into bucket list.
#endif
    }
    free(glob_pattern);
    globfree(&pglob);
}

/**
 * close snapshot storage - cleanup of temporary files
 *
 * @param ikea_store store to be close
 */
void ikea_store_close ( ikea_store_t * ikea_store )
{
    glob_t pglob;
    size_t pathc;
    char glob_pattern[(sizeof(tempdir_template) +4)];
    int rc;

    // glob for all the files in the temporaray directory
    strcpy(glob_pattern, ikea_store->tempdir);
    strcat(glob_pattern, "/*");
    if ((rc = glob(glob_pattern, 0, glob_err, &pglob))) {
        switch (rc) {
        case GLOB_NOSPACE:
            FATAL("glob(): no memory available (GLOB_NOSPACE)");
            break;
        case GLOB_ABORTED:
            FATAL("glob(): read error (GLOB_ABORTED)");
            break;
        case GLOB_NOMATCH:
            break; // not an error to have an empty graph
        }
    }
    for (pathc=0; pathc < pglob.gl_pathc; pathc++) {
        if ((unlink(pglob.gl_pathv[pathc])) == -1)
            FATAL("unlink()");
    }
    globfree(&pglob);

    // rmdir the temporary directory
    if (rmdir(ikea_store->tempdir) == -1)
        FATAL("rmdir()");

    free(ikea_store);
}

/**
 * Recursive hash accumulation of a fraglist, or list of fraglists
 *
 * @param ctx  - hashing context
 * @param list - fraglist or list of fraglist to be hashed
 */
static void sslhash_list_r(EVP_MD_CTX *ctx, elem_t *list)
{
    elem_t *elem;

// FIXME - this should be using iter()
// FIXME - where does this deal with shortstr?

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
                sslhash_list_r(ctx, elem);    // recurse
                elem = elem->u.l.next;
            }
            break;
        default:
            assert(0);  // should not be here
            break;
        }
    }
}

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

// FIXME - check available space for hash

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
    sslhash_list_r(ctx, list);
    if ((EVP_DigestFinal_ex(ctx, digest, &digest_len)) != 1)
        FATAL("EVP_DigestFinal_ex()");
}

//===================================

static void* ikea_open(void* descriptor, char *mode) {
    return ikea_box_open( (ikea_store_t*)descriptor, mode);
}

static void ikea_flush(IO_t *IO) {
    ikea_box_append((ikea_box_t*)IO->out_chan, IO->buf, IO->pos);
    IO->pos = 0;
}

static size_t ikea_write(IO_t *IO, unsigned char *cp, size_t size)
{
    int len = size;

    while (len--) {
        IO->buf[IO->pos++] = *cp++;
        if (IO->pos >= sizeof(IO->buf)) {
            ikea_flush(IO);
        }
    }
    return size;
}

static void ikea_close(IO_t *IO) {
    ikea_box_close ((ikea_box_t*)IO->out_chan,
            IO->contenthash,
            SIZEOFHASH>SUFFICIENTHASH?SUFFICIENTHASH:SIZEOFHASH
            );
}

// FIXME - require locks when writing a box that  maybe read or modified by other threads.
// FIXME - probably just ikea_box_close needs LOCKs becase of the rename operation.

// discipline for writing to ikea
out_disc_t ikea_disc = {
    &ikea_open,
    &ikea_write,
    &ikea_flush,
    &ikea_close
};
