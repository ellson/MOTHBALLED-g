/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <zlib.h>
#include <libtar.h>
#include <fcntl.h>
#include <glob.h>

#include "libje_private.h"

#if defined ( _MSC_VER) || defined(__WATCOMC__)
#include <io.h>
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

static gzFile gzf;

/**
 * open a gz compressed file
 *
 * @param pathname
 * @param oflags
 * @param VARARGS
 * @return filedescriptor, or -1=fail
 */
static int je_gzopen(const char *pathname, int oflags, ...)
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
static int je_gzclose(int fd)
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
static ssize_t je_gzread(int fd, void* buf, size_t count)
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
static ssize_t je_gzwrite(int fd, const void* buf, size_t count)
{
    (void) fd; // NOTUSED
    return gzwrite(gzf, (void*)buf, (unsigned int)count);
}

// discipline
static tartype_t gztype = { 
    je_gzopen,
    je_gzclose,
    je_gzread,
    je_gzwrite
};

/**
 * open a gz compressed tar file for a set of per-container graphs
 *
 * @param C context
 * @return list of containers ??
 */
elem_t * je_persist_open(context_t *C)
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
void je_persist_snapshot (context_t *C)
{
    int i;
    elem_t *elem, *next;
    FILE *fp;
    TAR *pTar;
    char *tarFilename = "g_snapshot.tgz";
    char *extractTo = ".";

    // flush all open files
    for (i=0; i<64; i++) {
        next = C->hash_buckets[i];
        while(next) {
            elem = next;
            next = elem->next;
            if ((fp = ((hash_elem_t*)elem)->out)) {
//============================= ikea flush ???  ===============
                if (fflush(fp) != 0)
                    fatal_perror("Error - fflush(): ");
//=======================================================
            }
        }
        //
        // FIXME - hash the content of the files, create hardlinks to the content hash
        //
    }

    if (tar_open(&pTar, tarFilename, &gztype, O_WRONLY | O_CREAT | O_TRUNC, 0600, TAR_GNU) == -1)
        fatal_perror("Error - tar_open():");
    if (tar_append_tree(pTar, C->tempdir, extractTo) == -1)
        fatal_perror("Error - tar_append_tree():");
    if (tar_append_eof(pTar) == -1)
        fatal_perror("Error - tar_append_eof():");
    if (tar_close(pTar) == -1)
        fatal_perror("Error - tar_close():");
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
void je_persist_restore (context_t *C)
{
    TAR *pTar;
    glob_t pglob;
    int rc;
    size_t len, pathc;
    char *tarFilename = "g_snapshot.tgz";
    char *glob_pattern;
    unsigned long hash;

    if (tar_open(&pTar, tarFilename, &gztype, O_RDONLY, 0600, TAR_GNU) == -1)
        fatal_perror("Error - tar_open():");
    if (tar_extract_all(pTar, C->tempdir) == -1)
        fatal_perror("Error - tar_extract_all():");
    if (tar_close(pTar) == -1)
        fatal_perror("Error - tar_close():");
    len = strlen(C->tempdir);
    if ((glob_pattern = malloc(len + 2)) == NULL)
        fatal_perror("Error - malloc():");
    strcpy(glob_pattern, C->tempdir);
    strcat(glob_pattern, "/*");
    if ((rc = glob(glob_pattern, 0, glob_err, &pglob)) != 0) {
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
void je_persist_close (context_t *C)
{
    FILE *fp;
    int i;
    elem_t *elem, *next;
    char hashname[12], *filename;

    for (i=0; i<64; i++) {
        next = C->hash_buckets[i];
        while(next) {
            elem = next;
            next = elem->next;
            if ((fp = ((hash_elem_t*)elem)->out)) {

//========================== ikea close ==========================
                // close all open files
                if (fclose(fp) != 0)
                    fatal_perror("Error - fclose(): ");
//================================================================
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

    // rmdir the temporary directory
    if (rmdir(C->tempdir) == -1)
        fatal_perror("Error - rmdir(): ");

    free_list(C, &(C->myname));
}
