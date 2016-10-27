/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "dispatch.h"

static void je_dispatch_r(CONTEXT_t * C, elem_t * plist, elem_t *pattributes, elem_t * pnodes, elem_t * pedges);
static void je_assemble_act(CONTEXT_t *C, elem_t * pelem, elem_t * pattributes, elem_t * plist);

/*
 * Processes an ACT after sameas and pattern substitutions.
 *
 *
 * FIXME - description needs updating
 *
 * - expand OBJECT_LIST
 *    - expand ENDPOINT_SETs
 *       - promote ENDPOINTS to common ancestor
 *           - extract NODES from EDGES for node induction
 *               - dispatch NODES (VERB NODE ATTRIBUTES CONTENT)
 *                   - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
 *           - dispatch EDGE  (VERB EDGE ATTRIBUTES CONTENT)
 *               - if add or delete and CONTENT is from pattern, copy content before modifying (COW)
 *
 * VERBs (add), delete, query
 *
 * Edges are owned (stored, rendered) by the common ancestor
 * of all the endpoints.
 *
 * So an edge to an external node:, e.g
 *                      <sibling ^/uncle/cousin>
 * is dealt with as follows:    // FIXME - not right
 *       1. node "sibling" is induced (created in the local
 *              graph of siblings, if it isn't already there.)
 *       2. the edge is sent though a special channel to
 *              the nameless parent of the the current siblings.
 *              That parent, prepends its name to all noderefs
 *              without a ^/, and removes one ^/ from all the others,
 *              So. in the parent's graph, the edge becomes:
 *                      <mum/sibling uncle/cousin>
 *              If there still ^/ left, then the process
 *              is repeated, sending it though the nameless channel to
 *              their parent, which applies the same rule:
 */

/**
 * @param CC container context
 * @param plist
 */
void je_dispatch(container_CONTEXT_t * CC, elem_t * plist)
{
    CONTEXT_t *C = CC->C;
    LIST_t * LIST = (LIST_t *)C;
    elem_t attributes = { 0 };
    elem_t nodes = { 0 };
    elem_t edges = { 0 };
    elem_t *pelem;
    
    assert(plist);
// P(plist);
    assert(plist->type == (char)LISTELEM);

    // expand OBJECT_LIST and ENDPOINTSETS
    je_dispatch_r(C, plist, &attributes, &nodes, &edges);

    // if NODE ACT ... for each NODE from nodes, generate new ACT: verb node attributes
    // else if EDGE ACT ... for each NODEREF, generate new ACT: verb node
    //                      for each EDGE, generate new ACT: verb edge attributes

    free_list(LIST, plist);  // free old ACT to be replace by these new expanded ACTS
    switch (CC->subject_type) {
    case NODE:
        pelem = nodes.u.l.first;
        while (pelem) {
            je_assemble_act(C,pelem,&attributes,plist);
            pelem = pelem->next;
        }
        break;
    case EDGE:
        if (C->has_cousin) {
            // FIXME - deal with edges that require help from ancestors
            fprintf(stdout,"EDGE has COUSIN\n");
        }
        else {
                pelem = nodes.u.l.first;
                while (pelem) {
                    je_assemble_act(C,pelem,NULL,plist);
                    pelem = pelem->next;
                }

                pelem = edges.u.l.first;
                while (pelem) {
                    je_assemble_act(C,pelem,&attributes,plist);
                    pelem = pelem->next;
                }
        }
        break;
    default:
        assert(0);  // shouldn't happen
        break;
    }

    free_list(LIST, &attributes);
    free_list(LIST, &nodes);
    free_list(LIST, &edges);
}

/**
 * This function expands OBJECT_LIST of NODES or EDGES, and then expands ENPOINTSETS in EDGES
 *
 * @param C context
 * @param plist   -- object-list
 * @param pattributes
 * @param pnodes
 * @param pedges
 */
static void je_dispatch_r(CONTEXT_t * C, elem_t * plist, elem_t * pattributes, elem_t * pnodes, elem_t * pedges)
{
    LIST_t * LIST = (LIST_t *)C;
    elem_t *pelem, *pnew, *pobject;
    state_t si;

    assert(plist->type == (char)LISTELEM);

    pelem = plist->u.l.first;
    while (pelem) {
        si = (state_t) pelem->state;
        switch (si) {
        case ACT:
            je_dispatch_r(C, pelem, pattributes, pnodes, pedges);
            break;
        case ATTRIBUTES:
            pnew = ref_list(LIST, pelem);
            append_list(pattributes, pnew);
            break;
        case SUBJECT:
            pobject = pelem->u.l.first;
            switch ((state_t)pobject->state) {
            case OBJECT:
                je_dispatch_r(C, pobject, pattributes, pnodes, pedges);
                break;
            case OBJECT_LIST:
                pobject = pobject->u.l.first;
                assert((state_t)pobject->state == OBJECT);
                while(pobject) {
                    je_dispatch_r(C, pobject, pattributes, pnodes, pedges);
                    pobject = pobject->next;
                }
                break;
            default:
                assert(0); //should never get here
                break;
            }
            break;
        case NODE:
            pnew = ref_list(LIST, pelem);
            append_list(pnodes, pnew);
            break;
        case EDGE:
            je_expand(C, pelem, pnodes, pedges);
            break;
        default:
            break;
        }
        pelem = pelem->next;
    }
}

/**
 * This function reassembles ACTS with no containment  -- FIXME improve on this
 *
 * @param C context
 * @param pelem   -- node or edge object
 * @param pattributes
 * @param plist - output ACT
 */
static void je_assemble_act(CONTEXT_t *C, elem_t *pelem, elem_t *pattributes, elem_t *plist)
{
    TOKEN_t * IN = (TOKEN_t *)C;
    LIST_t * LIST = (LIST_t *)C;
    elem_t act = { 0 };
    elem_t verb = { 0 };
    elem_t *pnew;

    act.state = ACT;

    // verb
    switch(IN->verb) {
    case QRY:
    case TLD:
        verb.state = IN->verb;
        pnew = move_list(LIST, &verb);
        append_list(&act, pnew);
        break;
    default:
        break;
    }

    // subject
    pnew = ref_list(LIST, pelem);
    append_list(&act, pnew);

    // attributes
    if (pattributes && pattributes->u.l.first) {
        pnew = ref_list(LIST, pattributes->u.l.first);
        append_list(&act, pnew);
    }

    // no container ever because contains are in their own streams

    pnew = move_list(LIST, &act);
    append_list(plist, pnew);
}

