/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef PARSE_H
#define PARSE_H

#ifdef __cplusplus
extern "C" {
#endif

struct parse_s {               // PARSE context

//    FIXME - its starting to look like we don't need this struct

    // FIXME  - do I need to take verb default from parent graph ?
    state_t verb;              // after parsing, 0 "add", TLD "del", QRY "query"
};

// functions
success_t parse(PARSE_t * PARSE, elem_t * root, state_t si, unsigned char prop, int nest, int repc, state_t bi);

#ifdef __cplusplus
}
#endif

#endif
