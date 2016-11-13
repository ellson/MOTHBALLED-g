Goals:

"g" is a language for describing graphs of nodes and edges.

Its predecessor/mentor language, "DOT" is an amazingly concise language for
describing graphs.  "g" is my attempt at adding some extra capabilities,
while maintaing an equivalent conciseness.

The goals of the "g" language are to:

0. Provide an alternative graph representation to DOT.

   DOT isn't going away, because it has too many users,
   and it is used as a backend to "g", so it is not necessary for g
   to 100% replace DOT.

1. Incremental graph creation, and modification.
   The graphviz tools (dot, neato, fdp, ...) are file oriented
   batch processed.
   - g allows for parallel ssource streams, multi-threaded processing,
       and distrubuted graphs.

2. Rethink of Subgraphs.

   Subgraphs in DOT are used for mutiple purposes:

   1. to describe sets of objects to attach attributes to.
	- g retains this capability through sets of objects and through patterns

   2. To describe a set of node endpoint to an edge.
	- g retains this capabiliy with sets of objects

   3. To describe clusters
	- g hopes to replace the concept of clusters with the use of containment.
        -- any NODE in g can have a contained graph
	    -- any EDGE in g can have a contained graph
              (DOT has no similar concept).

3. Rethink of Meta objects:  "node" and "edge" in DOT.
  - g offers sets to attrubutes that are common to the set members.
  - g offers patterns to specify meta-object and class-like objects

4. Support "netlists"  - DOT supports edges only with two ends.
  - g offers EDGEs with: one, two, three or more ends (LEGs).

5. Support "multi-nodes", analogous to "multi-edges" in DOT.
     (e.g. a node representing a deck of playing cards.)
  - g offers "disambiguation" of "non-strict" NODEs and EDGEs

6. Minimize the use of special characters that need quoting or escaping
in strings, and particularly in strings representing VALUES..
  - g has relaxed quoting in value strings, permitting without quotes:
      -- signed/unsigned int/float numbers
      -- URL strings
      -- Unix and DOS file paths
      -- user@host email addresses
  - g has special quoting modes for:
      -- JSON  (balanced '{' '}' outside of quoting or escapes))
      -- XML/HTML  (balanced '<' '>' outside of quoting or escapes)
      -- Binary    ( length + content )
   
7. Preservation of ordering on ends in edges. DOT preserves tail--head
   - g preserves the ordering of LEGs is preseverved

8. Preservation of input order.  g fails on this goal.
   - g does *not* preserve input order
       -- The arrival order in a g multi-input environment
       is not meaningful, since transmission delays will lose
       any relationship to global event ordering.
       -- In order to recognize common contents in nested containers
       they are stored in a canonical form in which NODES, EDGES, ATTRIBUTES
       are all sorted (C-locale, i.e ASCII order).
   Consequently:
      -- g graphs rendered in text form will look quite different
      from the input stream or file
      -- The semantics of DOT's ordering=in, or out, is *not* maintained
      through g processing.
   Workaround:
      -- The user may be able to control ordering through the uses
      of names chosen to sort in the desired order,
      (As in DOT, labels can be provided that are independent of name.)

9. Persistency outside of a single process.
  - g can save and restore all state from a file, and continue
    incremental evolution of the contents after restoral.

   

Whats in a name:

I like "g" as the name for this minimalist graph description language.s
If "g" collides with something else, then perhaps we can call it "je"
instead ;-)


Application goals:

- To be able to use an inter-connected set of "g" deamons to support a dynamic distributed graph:
	- accepting updates from mutiple sources in different locations.
	- interactively viewable from multiple locations.


Documentation:

- "g -?" provides comman-line documentation.

- "man g" provides a man page

- HTML documentation is generated during the make process.
	<a href="index.html">index.html</a>

- The souce is documented by Doxygen.

John Ellson   November 13, 2016
