#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "persist.h"
#include "hash.h"

static char template[] = {'g','_','X','X','X','X','X','X','\0'};

elem_t * je_persist_open(context_t *C)
{
    elem_t *elem, *name;
    size_t len = 0, slen = 0;
    char pidbuf[16];

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
    C->tempdir = mkdtemp(template);
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

    // snapshot
    system("grep . g_*/*");
}

// cleanup of temporary files
void je_persist_close (context_t *C)
{
    FILE *fp;
    int i;
    elem_t *elem, *next;
    char outhashname[32];
    char outfilename[sizeof(template) + 1 + sizeof(outhashname)];

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
                je_base64(outhashname, &(elem->u.hash.hash));
                strcpy(outfilename, C->tempdir);
                strcat(outfilename, "/");
                strcat(outfilename, outhashname);
                
                // rm all output files
                if (unlink(outfilename) == -1) {
                    perror("Error - unlink(): ");
                    exit(EXIT_FAILURE);
                }
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
