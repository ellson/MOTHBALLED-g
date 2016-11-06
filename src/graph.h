/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

struct graph_s {               // GRAPH context

// FIXME - move to CONTAINER
    int containment;           // depth of containment
    long stat_containercount;
//

    // FIXME  - do I need to take verb default from parent graph ?
    state_t verb;              // after parsing, 0 "add", TLD "del", QRY "query"
    char need_mum;           // flag set if MUM is referenced

    // ok as per-graph counts - may also need at THREAD level for total i/o stats
    long stat_inactcount;
    long stat_outactcount;
    long stat_sameas;
    long stat_patternactcount;
    long stat_nonpatternactcount;
    long stat_patternmatches;
};

// functions
success_t graph(CONTAINER_t * CONTAINER, elem_t * root, state_t si, unsigned char prop, int nest, int repc);

#ifdef __cplusplus
}
#endif

#endif
