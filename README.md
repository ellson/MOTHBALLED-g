"DOT" is an amazingly concise language for describing graphs.  "g" is my attempt at doing at least as
well whule adding some extra capabilities.

The goals of "g" are to:

0. represent any graph that "DOT" can.

1. incremental graph creation, modifications through a stream of "acts"
  - no beginning, no end,  the "graph" is what you see as a result.
  - Allow for and encourage a multi-threaded implementation.

2. rethink of Subgraphs:
  - Subgraphs used to to describe sets of objects to attatch attributes to
  - Subgraphs used to describe clusters
   
    2a) use of string partterns to specify meta-object and class-like objects

    2b) provide general containment to subsume clusters (i.e.a cluster becomes a first-class node in 'g')
      - Any node can contain a graph
      - Any edge can contain a graph

3. netlists.  (i.e. edges with more than two ends)

4. "multi-nodes" analogous to "multi-edges"

6. minimize special characters:  '-' is not a special character, so now number strings can be used 
   as simple strings.  

7. simplify:  insist that ids use only simple strings with no escapes or quotes.


