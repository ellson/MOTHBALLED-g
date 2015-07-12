png = \
	hello_world.png \
	edge_into_container_via_port.png \
	edge_into_container_using_nodepath.png \
	netlist.png \
	implement_netlist_with_edge_container.png \
	multi_node_multi_edge.png


all: ${png}

%.png: %.svg
	inkscape \
		--export-area-drawing \
		--export-background=white \
		--export-png=$@ \
		--file=$<

clean:
	rm -f ${png}
