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
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "thread.h"
#include "expand.h"
#include "dispatch.h"

static void
dispatch_r(CONTAINER_t * CONTAINER, elem_t * list, elem_t *disambig,
        elem_t *attributes, elem_t * nodes, elem_t * edges);

static elem_t *
assemble_act(THREAD_t * THREAD, state_t verb, elem_t *elem, elem_t *disambig,
        elem_t *attributes);


/*
 * Processes an ACT after sameas  substitution
 *
 * FIXME - description needs updating
 *
 * - expand SETs of NODES or EDGES
 *    - expand ENDPOINTSETs
 *       - promote ENDPOINTS to common ancestor
 *           - extract NODES from EDGES for node induction
 *               - dispatch NODES (VERB NODE ATTRIBUTES CONTAINER)
 *           - dispatch EDGE  (VERB EDGE ATTRIBUTES CONTAINER)
 *
 * VERBs (add), delete, query
 *
 * Edges are owned (stored, rendered) by the common ancestor
 * of all the endpoints.
 *
 * So an edge to an external node:, e.g
 *                      <sis ^aunt/cousin>
 * is dealt with as follows:    // FIXME - not right
 *       1. node "sibling" is induced (created in the local
 *              graph of siblings, if it isn't already there.)
 *       2. the edge is sent though a special channel to
 *              the nameless parent (mum) of the the current siblings.
 *              That parent, prepends its name to all noderefs
 *              without a ^, and removes one ^ from all the others,
 *              So. in the parent's graph, the edge becomes:
 *                      <mum/sis aunt/cousin>
 *              If there still ^ left, then the process
 *              is repeated, sending it though the nameless channel to
 *              their parent, which applies the same rule:
 */

/**
 * @param CONTAINER container context
 * @param act - input act, after whatever rewrites have happened so far
 * @param verb - add, del, qry 
 * @param mum - will be needed
 * @return new list of ACTS
 */
elem_t *
dispatch(CONTAINER_t * CONTAINER, elem_t * act, state_t verb, state_t mum)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *new, *newacts, *elem, *nodes, *edges, *disambig, *attributes;
    
    assert(act);
    assert(act->type == (char)LISTELEM);
    assert((state_t) act->state == ACT);
//E();
//P(act);

    newacts = new_list(LIST(), ACTIVITY); // return list

    nodes = new_list(LIST(), NODES);
    edges = new_list(LIST(), EDGES);
    attributes = new_list(LIST(), ATTRIBUTES);
    disambig = new_list(LIST(), DISAMBIG);

    // expand SET and ENDPOINTSET
    dispatch_r(CONTAINER, act, disambig, attributes, nodes, edges);

    if (CONTAINER->has_node) {
        elem = nodes->u.l.first;
        while (elem) {
            if (disambig->u.l.first && !verb) {
                new = assemble_act(THREAD, verb, elem, NULL, NULL);  // induce base node
                append_transfer(newacts, new);
            }
            new = assemble_act(THREAD, verb, elem, disambig, attributes);
            append_transfer(newacts, new);
            elem = elem->u.l.next;
        }
    }
    if (CONTAINER->has_edge) {
        if (mum) {
            // FIXME - deal with edges that require help from ancestors
            fprintf(stdout,"Need Mum's help\n");
        }
        if (!verb) { // don't query or delete induced nodes
            elem = nodes->u.l.first;
            while (elem) {
                // inducing base nodes from NODEREFS - no attributes from these
                new = assemble_act(THREAD, verb, elem, NULL, NULL);
                append_transfer(newacts, new);
                elem = elem->u.l.next;
            }
        }
        elem = edges->u.l.first;
        while (elem) {
            if (disambig->u.l.first && !verb) {
                new = assemble_act(THREAD, verb, elem, NULL, NULL);  // induce base edge
                append_transfer(newacts, new);
            }
            new = assemble_act(THREAD, verb, elem, disambig, attributes);
            append_transfer(newacts, new);
            elem = elem->u.l.next;
        }
    }

    free_list(LIST(), nodes);
    free_list(LIST(), edges);
    free_list(LIST(), attributes);
    free_list(LIST(), disambig);

//P(newacts);
    return(newacts);
}

/**
 * This function expands SETs of NODES or EDGES,
 *     then expands ENPOINTSETS in EDGES,
 *     and returns the expansion as list of simple NODEs and EDGEs
 *
 * @param CONTAINER context
 * @param list of object -- not modified
 * @param disambig -- set if one found
 * @param attributes -- appended
 * @param nodes -- appended
 * @param edges -- appended
 */
static void
dispatch_r(CONTAINER_t * CONTAINER, elem_t * list, elem_t *disambig,
        elem_t * attributes, elem_t * nodes, elem_t * edges)
{
    THREAD_t *THREAD = CONTAINER->THREAD;
    elem_t *elem, *new, *object;

    assert(list->type == (char)LISTELEM);
//E();
//P(list);

    elem = list->u.l.first;
    while (elem) {
        switch ((state_t) elem->state) {
            case ACT:
                dispatch_r(CONTAINER, elem, disambig, attributes, nodes, edges);
                break;
            case SUBJECT:
                object = elem->u.l.first;
                switch ((state_t)object->state) {
                    case SET:
                        dispatch_r(CONTAINER, object, disambig, attributes, nodes, edges);
                        break;
                    case NODE:
                    case PORT:
                        new = ref_list(LIST(), object);
                        append_transfer(nodes, new);
                        break;
                    case EDGE:
                        expand(CONTAINER, object, nodes, edges);
                        break;
                    default:
                        S((state_t)object->state);
                        assert(0); //should never get here
                        break;
                }
                break;
            case DISAMBIG:
                append_addref(disambig, elem->u.l.first);
                break;
            case ATTRIBUTES:
                new = ref_list(LIST(), elem);
                append_transfer(attributes, new);
                break;
            case NODE:
            case PORT:
                new = ref_list(LIST(), elem);
                append_transfer(nodes, new);
                break;
            case EDGE:
                expand(CONTAINER, elem, nodes, edges);
                break;
            default:
                S((state_t) elem->state);
                assert(0);  // should never get here
                break;
        }
        elem = elem->u.l.next;
    }
//E();
}

/**
 * This function assembles ACTs with simplified SUBJECT and no CONTAINER
 * The act VERB is copied from the current input ACT verb
 *
 * @param THREAD context
 * @param verb --  add, delete, query
 * @param elem -- node or edge object
 * @param disambig -- subject disambiguator
 * @param attributes 
 * @return a reassembled node or edge act
 */
static elem_t *
assemble_act(THREAD_t * THREAD, state_t verb, elem_t *elem, elem_t *disambig, elem_t *attributes)
{
    elem_t *new, *act, *subject, *noun;

    act = new_list(LIST(), ACT);  // return list

    // verb
    switch(verb) {
    case QRY:
    case TLD:
        new = new_list(LIST(), verb);
        append_transfer(act, new);
        break;
    case 0:        // default is add
        break;
    default:
        assert(0); //should never get here
        break;
    }

    // subject
    switch ((state_t)elem->state) {
        case NODE:
            noun = new_list(LIST(), NODE);
            break;
        case PORT:
            noun = new_list(LIST(), PORT);
            break;
        case EDGE:
            noun = new_list(LIST(), EDGE);
            break;
        default:
            S((state_t)elem->state);
            assert(0);
    }
    if (elem->u.l.first) {
        append_addref(noun, elem->u.l.first);
        subject = new_list(LIST(), SUBJECT);
        append_transfer(subject, noun);
        append_transfer(act, subject);
    
        // disambig - make part of subject  
        if (disambig && disambig->u.l.first) {
            new = new_list(LIST(), DISAMBIG);
            append_addref(new, disambig->u.l.first);
            append_transfer(act, new);
        }
    
        // attributes
        if (attributes && attributes->u.l.first) {
            new = ref_list(LIST(), attributes->u.l.first);
            append_transfer(act, new);
        }
    
        // no container ever because contents are handled in a parse_nest_r() recursion.
    }
    return act;
}
