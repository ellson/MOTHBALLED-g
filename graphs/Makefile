out = \
	edge_into_container_using_nodepath.g
	edge_into_container_via_port.g
	hello_world.g
	implement_netlist_with_edge_container_1.g
	implement_netlist_with_edge_container_2.g
	implement_netlist_with_edge_container_3.g
	multi_node_multi_edge_1.g
	multi_node_multi_edge_2.g
	multi_node_multi_edge_3.g
	netlist.g

all: ${out}

%.out: %.g
        ../src/g $< >$@

clean:
        rm -f ${out}

