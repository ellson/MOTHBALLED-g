Goals:

"g" is a language for describing graphs of nodes and edges.

Its predecessor/mentor language, "DOT" is an amazingly concise language for
describing graphs.  "g" is my attempt at adding some extra capabilities, while
maintaing an equivalent conciseness.

The goals of the "g" language are to:

0. Represent any graph that "DOT" can.

1. Incremental graph creation, and modification, through a stream of ACTs
  - Allow for and encourage a multi-threaded and distrubuted implementation.

2. Rethink of Subgraphs:

  - Subgraphs used to to describe sets of objects to attach attributes to.
	- g retains this capability through LISTS of objects
  - Subgraphs used to describe a set of endpoint to edges.
	- g retains this capabiliy
  - Subgraphs used to describe clusters
	- g replaces the concept of clusters with the use of CONTAINER
	any NODE SUBJECT in g can have a contained graph (an ACTIVITY in g's terms)
        so DOT's clusters are just NODES in g. Additionally, in g, an EDGE SUBJECT
        may also have a CONTAINER.  There is no correspondence to this in DOT.

3. Rethink of Meta objects:  "node" and "edge" in DOT.

  - g uses string patterns to specify meta-object and class-like objects

4. Support "netlists:.  i.e. edges with more than two ends.  Also dangling edges with only one end.

5. Support "multi-nodes", analogous to "multi-edges" in DOT.  (e.g. a node representing a deck of playing cards.)

6. Minimize the use of special characters that need quoting or escaping in strings.

  - in particular, g allows signed decimal numbers to be used without quoting since: '+', '-', '.' and ',' are not special
   

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
