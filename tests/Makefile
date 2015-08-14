GOODTESTS = pattern.g nodes.g edges.g comments.g escape.g quote.g

BADTESTS = edge_in_node_subj.badg node_in_edge_subj.badg

all: good bad



.PHONY: good
good: ${GOODTESTS} 
	for i in ${GOODTESTS} ; do ../src/g $$i >/dev/null ; done

.PHONY: bad
bad: ${BADTESTS} 
	for i in ${BADTESTS} ; do ../src/g $$i >/dev/null 2>/dev/null ; if test $$? == 0; then fail; break; fi ; done
