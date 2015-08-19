GOODTESTS = comments.g edges.g escape.g nodes.g pattern.g quote.g sameas.g

BADTESTS = eof_inquote.badg edge_subject_includes_node.badg node_subject_includes_edge.badg

all: good bad


.PHONY: good
good: ${GOODTESTS} 
	for i in ${GOODTESTS} ; do ../src/g $$i >/dev/null ; done

.PHONY: bad
bad: ${BADTESTS} 
	for i in ${BADTESTS} ; do ../src/g $$i >/dev/null 2>/dev/null ; if test $$? == 0; then false; break; fi ; done
