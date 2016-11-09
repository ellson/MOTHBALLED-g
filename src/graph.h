/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

struct graph_s {               // GRAPH context

//    FIXME - its starting to look like we don't need this struct

    // FIXME  - do I need to take verb default from parent graph ?
    state_t verb;              // after parsing, 0 "add", TLD "del", QRY "query"
    char need_mum;           // flag set if MUM is referenced
};

// functions
success_t graph(GRAPH_t * GRAPH, elem_t * root, state_t si, unsigned char prop, int nest, int repc, state_t bi);

#ifdef __cplusplus
}
#endif

#endif
