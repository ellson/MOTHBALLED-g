#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include <libtar.h>
#include <fcntl.h>
#include <glob.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "persist.h"
#include "hash.h"

#if defined ( _MSC_VER) || defined(__WATCOMC__)
#include <io.h>
#ifndef O_ACCMODE
# define O_ACCMODE 0x0003
#endif
#endif

// FIXME - ya global
gzFile gzf;

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

static int je_gzclose(int fd)
{
    (void) fd; // NOTUSED
    return gzclose(gzf);
}

static ssize_t je_gzread(int fd, void* buf, size_t count)
{
    (void) fd; // NOTUSED
    return gzread(gzf, buf, (unsigned int)count);
}

static ssize_t je_gzwrite(int fd, const void* buf, size_t count)
{
    (void) fd; // NOTUSED
    return gzwrite(gzf, (void*)buf, (unsigned int)count);
}

tartype_t gztype = { 
    je_gzopen,
    je_gzclose,
    je_gzread,
    je_gzwrite
};

elem_t * je_persist_open(context_t *C)
{
    elem_t *elem, *name;
    size_t len = 0, slen = 0;
    char pidbuf[16];
    int i;
    char *template_init = "/tmp/g_XXXXXX";

    // copy template including trailing NULL
    i = 0;
    while ((C->template[i] = template_init[i])) {
        i++;
    }

    name = &(C->myname);

    assert (C->inbuf == NULL);
    new_inbuf(C);

    len = strlen(C->username);
    elem = new_frag(C, ABC, len, (unsigned char*)C->username);
    append_list(name, elem);
    slen += len;

    elem = new_frag(C, ABC, 1, (unsigned char*)"@");
    append_list(name, elem);
    slen++;

    len = strlen(C->hostname);
    elem = new_frag(C, ABC, len, (unsigned char*)C->hostname);
    append_list(name, elem);
    slen += len;

    elem = new_frag(C, ABC, 1, (unsigned char*)"_");
    append_list(name, elem);
    slen++;

    sprintf(pidbuf,"%u",C->pid);
    len = strlen(C->hostname);
    assert (len  < sizeof(pidbuf));
    elem = new_frag(C, ABC, len, (unsigned char*)pidbuf);
    append_list(name, elem);
    slen += len;

    assert (slen < INBUFSIZE);

    C->inbuf = NULL;   // hang on to this inbuf privately for myname

    // make a temporary directory
    C->tempdir = mkdtemp(C->template);
    if (!C->tempdir) {
        perror("Error - mkdtemp(): ");
        exit(EXIT_FAILURE);
    }

    return name;
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
            if ((fp = elem->u.hash.out)) {
                if (fflush(fp) != 0) {
                    perror("Error - fclose(): ");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    if (tar_open(&pTar, tarFilename, &gztype, O_WRONLY | O_CREAT | O_TRUNC, 0600, TAR_GNU) == -1) {
        perror("Error - tar_open():");
        exit(EXIT_FAILURE);
    }
    if (tar_append_tree(pTar, C->tempdir, extractTo) == -1) {
        perror("Error - tar_append_tree():");
        exit(EXIT_FAILURE);
    }
    if (tar_append_eof(pTar) == -1) {
        perror("Error - tar_append_eof():");
        exit(EXIT_FAILURE);
    }
    if (tar_close(pTar) == -1) {
        perror("Error - tar_close():");
        exit(EXIT_FAILURE);
    }
}

static int glob_err (const char *epath, int eerrno)
{
    fprintf(stderr,"Error - glob(): \"%s\" %s\n", epath, strerror(eerrno));
    return -1;
}

// restore from snapshot
void je_persist_restore (context_t *C)
{
    TAR *pTar;
    glob_t pglob;
    int rc;
    size_t len, pathc;
    char *tarFilename = "g_snapshot.tgz";
    char *glob_pattern;
    unsigned long hash;

    if (tar_open(&pTar, tarFilename, &gztype, O_RDONLY, 0600, TAR_GNU) == -1) {
        perror("Error - tar_open():");
        exit(EXIT_FAILURE);
    }
    if (tar_extract_all(pTar, C->tempdir) == -1) {
        perror("Error - tar_extract_all():");
        exit(EXIT_FAILURE);
    }
    if (tar_close(pTar) == -1) {
        perror("Error - tar_close():");
        exit(EXIT_FAILURE);
    }
    len = strlen(C->tempdir);
    if ((glob_pattern = malloc(len + 2)) == NULL) {
        perror("Error - malloc():");
        exit(EXIT_FAILURE);
    }
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

// cleanup of temporary files
void je_persist_close (context_t *C)
{
    FILE *fp;
    int i;
    elem_t *elem, *next;
    char *hashname, *filename;

    for (i=0; i<64; i++) {
        next = C->hash_buckets[i];
        while(next) {
            elem = next;
            next = elem->next;
            if ((fp = elem->u.hash.out)) {

                // close all open files
                if (fclose(fp) != 0) {
                    perror("Error - fclose(): ");
                    exit(EXIT_FAILURE);
                }

                // reconsitute the filename and unlink
                hashname = je_long_to_base64(&(elem->u.hash.hash));
                if (! (filename = malloc(sizeof(C->template) + 1 + sizeof(hashname) + 1))) {
                    perror("Error - malloc(): ");
                    exit(EXIT_FAILURE);
                }
                strcpy(filename, C->tempdir);
                strcat(filename, "/");
                strcat(filename, hashname);
                
                // rm all output files
                if (unlink(filename) == -1) {
                    perror("Error - unlink(): ");
                    exit(EXIT_FAILURE);
                }
                free(filename);
            }

            // return elem to free_elem_list
            elem->next = C->free_elem_list;
            C->free_elem_list = elem;
        }
    }

    // rmdir the temporary directory
    if (rmdir(C->tempdir) == -1) {
        perror("Error - rmdir(): ");
        exit(EXIT_FAILURE);
    }

    free_list(C, &(C->myname));
}
