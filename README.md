Goals:

"g" is a language for describing graphs of nodes and edges.

Its predecessor/mentor language, "DOT" is an amazingly concise language for
describing graphs.  "g" is my attempt at adding some extra capabilities, while
maintaing an equivalent conciseness.

The goals of the "g" language are to:

0. Represent any graph that "DOT" can, unfortunately  not 100% achievable with the other goals, which are:

1. Incremental graph creation, and modification, through a stream of ACTs
  - Allow for and encourage a multi-threaded and distrubuted implementation.

2. Rethink of Subgraphs:

  - Subgraphs in DOT are used to to describe sets of objects to attach attributes to.
	- g retains this capability through sets of objects
  - Subgraphs in DOT are used to describe a set of endpoinast to edges.
	- g retains this capabiliy with sets of objects
  - Subgraphs used to describe clusters
	- g hopes to replace the concept of clusters with the use of containment.
        -- Any NODE in g can have a contained graph, also 
	    -- any EDGE in g can have a contained graph (DOT has no similar concept).

3. Rethink of Meta objects:  "node" and "edge" in DOT.

  - g offers string patterns to specify meta-object and class-like objects

4. Support "netlists"  i.e. edges with: one, two, three or more ends.

5. Support "multi-nodes", analogous to "multi-edges" in DOT.  (e.g. a node representing a deck of playing cards.)

6. Minimize the use of special characters that need quoting or escaping in strings.

  - in particular, g has relaxed quoting constrains in value strings, which permit most signed/unsigned int/float
    numbers, and most URL strinasg, to be provided without quoting. 
   
7. Preservation of input ordering.   
   I've been struggling with this one ... current plans are to give this up so that graphs can be stored in
   a canonical form independent of the arrival order of nodes and edges. In its place, objects will be stored in
   C-locale sort order of node and edge names (SUBJECT).

   The user may be able to control ordering theough the use of names chosen to sort in that order,
   As in DOT, labels can be provided that are independent on name.
   

Whats in a name:

I like "g" as the name for this minimalist graph description language.    If "g" collides
with something else, then perhaps we can call it "je" instead ;-)


Application goals:

- To be able to use an inter-connected set of "g" deamons to support a dynamic distributed graph:
	- accepting updates from mutiple sources in different locations.
	- interactively viewable from multiple locations.


Documentation:

- After running "make" in the toplevel directory, there should be an
	<a href="index.html">index.html</a>
with additional documentation.

John Ellson   August 16, 2015
