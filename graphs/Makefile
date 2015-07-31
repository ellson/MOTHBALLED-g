out = \
	edge_into_container_via_port.out \
	edge_into_container_using_nodepath.out \
	hello_world.out \
	implement_netlist_with_edge_container_1.out \
	implement_netlist_with_edge_container_2.out \
	implement_netlist_with_edge_container_3.out \
	multi_node_multi_edge_1.out \
	multi_node_multi_edge_2.out \
	multi_node_multi_edge_3.out \
	netlist.out

all: ${out}

%.out: %.g
	../src/g $< >$@

clean:
	rm -f ${out}

