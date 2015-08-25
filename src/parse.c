#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "info.h"
#include "emit.h"
#include "parse.h"
#include "token.h"
#include "sameas.h"
#include "pattern.h"
#include "hash.h"

// This parser recurses at two levels:
//
//     parse() ----> parse_activity(C) ----> parse_r(CC) -| -|  
//                   ^                       ^            |  |
//                   |                       |            |  |
//                   |                       ------<-------  |
//                   |                                       |
//                   -----------------<-----------------------
//
// The inner recursions is through the grammar state_machine at a single
// level of containment - maintained in container_context (CC)
//
// The outer recursion is through nested containment.
// The top-level context (C) is available to both and maintains the input state.

static success_t more_rep(container_context_t * CC, unsigned char prop)
{
	state_t ei, bi;
    context_t *C = CC->context;

	if (!(prop & (REP | SREP)))
		return FAIL;

	ei = C->ei;
	if (ei == RPN || ei == RAN || ei == RBR || ei == RBE) {
		return FAIL;	// no more repetitions
	}
	bi = C->bi;
	if (bi == RPN || bi == RAN || bi == RBR || bi == RBE
	    || (ei != ABC && ei != AST && ei != DQT)) {
		return SUCCESS;	// more repetitions, but additional WS sep is optional
	}
	if (prop & SREP) {
		emit_sep(CC);	// sep is non-optional, emit the minimal sep
	}
	return SUCCESS;		// more repetitions
}

static success_t
parse_r(container_context_t * CC, elem_t * root,
	state_t si, unsigned char prop, int nest, int repc)
{
	unsigned char nprop;
	char so;		// offset to next state, signed
	state_t ti, ni;
	success_t rc;
	elem_t *elem;
	elem_t branch = { 0 };
	context_t *C = CC->context;
    unsigned long hash;
	static unsigned char nullstring[] = { '\0' };

	rc = SUCCESS;
	emit_start_state(CC, si, prop, nest, repc);
	branch.state = si;

	nest++;
	assert(nest >= 0);	// catch overflows

	if (!C->inbuf) {	// state_machine just started
		C->bi = WS;	    // pretend preceeded by WS to satisfy toplevel SREP or REP
		                // (Note, first REP of a sequence *can*
		                // be preceeded by WS, just not the
		                // rest of the REPs. )
		C->in = nullstring;	// fake it;
		C->insi = NLL;	// pretend last input was the EOF of
		                // a prior file.
	}

	// Entering state
	C->state = si;		// record of last state entered, for error messages.

	// deal with "terminal" states: Whitespace, Tokens, and Contained activity, Strings

	C->ei = C->insi;	// the char class that ended the last token

	// Whitespace
	if ((rc = parse_whitespace(CC)) == FAIL) {
		goto done;	// EOF during whitespace
	}

	// Special character tokens
	if (si == C->insi) {	// single character terminals matching state_machine expectation
		C->bi = C->insi;
		rc = parse_token(CC);
		C->ei = C->insi;
		goto done;
	}
	switch (si) {
	case ACTIVITY:          // Recursion into Contained activity
		if (C->bi == LBE) {	// if not top-level of containment
			C->bi = NLL;
			rc = parse(CC->context, &CC->subject);	// recursively process contained ACTIVITY in to its own root
			C->bi = C->insi;	// The char class that terminates the ACTIVITY
			goto done;
		}
		break;

	case STRING:            // Strings
		rc = parse_string(CC, &branch);
		C->bi = C->insi;	// the char class that terminates the STRING
		goto done;
		break;

	case VSTRING:            // Value Strings
		rc = parse_vstring(CC, &branch);
		C->bi = C->insi;	// the char class that terminates the VSTRING
		goto done;
		break;

		// the remainder of the switch() is just state initialization and emit hooks;
	case ACT:
		// This is a bit ugly.
		//
		// Because the grammar has no mandatory terminal token for ACTs, the 
		// only time we can be sure that the old ACT is finished is when there
		// is enough input stream to determine that a new ACT has sarted.
		//
		// This is not a problem for patterns, since they are not used until they are
		// matched to a later SUBJECT anyway.  Patterns in the last ACT of input just aren't
		// useful.

		if (CC->is_pattern) {   // flag was set by SUBJECT in previous ACT
			                    //  save entire previous ACT in a list of pattern_acts
			C->stat_patterncount++;
			elem = ref_list(C, root);

			if (CC->act_type == NODE) {
				append_list(&(CC->node_pattern_acts), elem);
			} else {
				assert(CC->act_type == EDGE);
				append_list(&(CC->edge_pattern_acts), elem);
			}
		} else {
			C->stat_actcount++;
		}

		free_list(C, root);	// now we're done with the last ACT
		                    // and we can really start on the new ACT
        C->verb = 0;        // initialize verb to default "add"
		break;
	case SUBJECT:
		C->has_ast = 0;     // maintain a flag for an '*' found anywhere in the subject
		break;
	default:
		break;
	}

	// If it wasn't a terminal state, then use the state_machine to
	// iterate through ALTs or sequences, and then recursively process next the state

	rc = FAIL;		// init rc to FAIL in case no ALT is satisfied
	ti = si;
	while ((so = state_machine[ti])) {	// iterate over ALTs or sequences
		nprop = state_props[ti];    	// get the props for the transition
                                        // from the current state (OPT, ALT, REP etc)

		                                // at this point, ni is a signed, non-zero
                                        // offset to the next state
		ni = ti + so;               	// we get to the next state by adding the
                                        // offset from the current state.

		if (nprop & ALT) {              // look for ALT
			if ((rc = parse_r(CC, &branch, ni, nprop, nest, 0)) == SUCCESS) {
				break;                  // ALT satisfied
			}

			                            // we failed an ALT so continue iteration to try next ALT
		} else {                    	// else it is a sequence (or the last ALT, same thing)
			repc = 0;
			if (nprop & OPT) {          // OPTional
				if ((parse_r(CC, &branch, ni, nprop, nest, repc++)) == SUCCESS) {
					while (more_rep(CC, nprop) == SUCCESS) {
						if (parse_r(CC, &branch, ni, nprop, nest, repc++) == FAIL) {
							break;
						}
					}
				}
			} else {                	// else not OPTional
				if ((rc = parse_r(CC, &branch, ni, nprop, nest, repc++)) == FAIL) {
					break;
				}
				while (more_rep(CC, nprop) == SUCCESS) {
					if ((rc = parse_r(CC, &branch, ni, nprop, nest, repc++)) == FAIL) {
						break;
					}
				}
			}
		}
		ti++;		// next ALT (if not yet satisfied), or next sequence item
	}

	// Any subtree rewrites or emits before adding branch to root in the state exit processing
	if (rc == SUCCESS) {
		switch (si) {
		case SUBJECT:
            branch.state = si;

			// Perform EQL "same as in subject of previous ACT" substitutions
			// Also classifies ACT as NODE or EDGE based on SUBJECT
			sameas(CC, &branch);

            hash_list(&hash, &(CC->subject));   // generate name hash
            elem = hash_bucket(C, hash);    // save in bucket list 

			// If this subject is not a pattern, then perform pattern matching and insertion if matched
			if (!(CC->is_pattern = C->has_ast)) {
				pattern(CC, root, &branch);
			}

			emit_subject(CC, &branch);	// emit hook for rewritten subject
			break;
		case ATTRIBUTES:
			emit_attributes(CC, &branch);
			break;
		default:
			break;
		}
	}

 done: // State exit processing
	if (rc == SUCCESS) {
		switch (si) {
		case TLD:
		case HAT:
		case QRY:
            C->verb = si;
            break;
        default:
            break;
        }
		if (branch.u.list.first != NULL || si == EQL) {	// mostly ignore empty lists
            branch.state = si;
			elem = move_list(C, &branch);
			append_list(root, elem);
		}
	}
	nest--;
	assert(nest >= 0);
	emit_end_state(CC, si, rc, nest, repc);

	return rc;
}

success_t parse(context_t * C, elem_t * name)
{
	container_context_t container_context = { 0 };
	elem_t root = { 0 };	// the output parse tree
    elem_t myname = { 0 };
    elem_t *elem, **next;
	success_t rc;
    unsigned long hash;
    char template[] = {'g','_','X','X','X','X','X','X','\0'};
    char outhashname[12];
    char outfilename[sizeof(template) + sizeof(outhashname)];
    int i;
    FILE *fp;

	container_context.context = C;

    if ((C->containment++) == 0) {  // top container
        g_session(&container_context); // gather session info, including starttime for stats

        name = &myname;

        assert (C->inbuf == NULL);
        new_inbuf(C);

        elem = new_frag(C, ABC, strlen(C->username), (unsigned char*)C->username);
        append_list(name, elem);
        elem = new_frag(C, ABC, 1, (unsigned char*)"@");
        append_list(name, elem);
        elem = new_frag(C, ABC, strlen(C->hostname), (unsigned char*)C->hostname);
        append_list(name, elem);

// FIXME - add string versions of pid and starttime to name
//       - how is this node named in a distribution of nodes???

        C->inbuf = NULL;   // hang on to this inbuf privately for myname

        C->tempdir = mkdtemp(template);
        if (!C->tempdir) {
            perror("Error - mkdtemp(): ");
            exit(EXIT_FAILURE);
        }
    }
    hash_list(&hash, name);    // hash name (subject "names" can be very long)
    elem = hash_bucket(C, hash);    // save in bucket list 
    if (! elem->u.hash.out) { // open file, if not already open
        base64(outhashname, &hash);
        strcpy(outfilename, C->tempdir);
        strcat(outfilename, "/");
        strcat(outfilename, outhashname);
        elem->u.hash.out = fopen(outfilename,"wb");
        if (! elem->u.hash.out) {
            perror("Error - fopen(): ");
            exit(EXIT_FAILURE);
        }
    }
    container_context.out = elem->u.hash.out;

	C->stat_containercount++;

	emit_start_activity(&container_context);
	if ((rc = parse_r(&container_context, &root, ACTIVITY, SREP, 0, 0)) != SUCCESS) {
		if (C->insi == NLL) {	// EOF is OK
			rc = SUCCESS;
		} else {
			emit_error(C, C->state, "Parse error. Last good state was:");
		}
	}

	free_list(C, &container_context.subject);
	free_list(C, &container_context.node_pattern_acts);
	free_list(C, &container_context.edge_pattern_acts);

	emit_end_activity(&container_context);

    if (--(C->containment) == 0) {  // top container
        if (C->needstats) {
            // FIXME - need pretty-printer
            fprintf (stderr, "%s\n", g_session(&container_context));
            fprintf (stderr, "%s\n", g_stats(&container_context));
        }
        free_list(C, &myname);

        for (i=0; i<64; i++) {
            next = C->hash_buckets[i];
            while(next) {
                elem=next;
                next = elem->next;
                if ((fp = elem->u.hash.out)) {
                    if (fclose(fp) != 0) {
                        perror("Error - fclose(): ");
                        exit(EXIT_FAILURE);
                    }

#if 0
                    // reconsitute the filename and unlink
                    base64(outhashname, &(elem->u.hash.hash));
                    strcpy(outfilename, C->tempdir);
                    strcat(outfilename, "/");
                    strcat(outfilename, outhashname);
                    if (unlink(outfilename) == -1) {
                        perror("Error - unlink(): ");
                        exit(EXIT_FAILURE);
                    }
#endif
                }

                // return elem to free_elem_list
                elem->next = C->free_elem_list;
                C->free_elem_list = elem;
            }
        }
#if 0
        if (rmdir(C->tempdir) == -1) {
            perror("Error - rmdir(): ");
            exit(EXIT_FAILURE);
        }
#endif
    }
    
	return rc;
}
