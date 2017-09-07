top.svg:
L+n3ye.svg L+n3ye.png: L+n3ye.gv QhxNFh.svg QhxNFh.png
QhxNFh.svg QhxNFh.png: QhxNFh.gv
kc8dcM.svg kc8dcM.png: kc8dcM.gv L+n3ye.svg L+n3ye.png
top.svg: kc8dcM.svg
	ln kc8dcM.svg top.svg

.SUFFIXES:
.SUFFIXES: .g .gv .png .svg

.gv.png:
	fdp -Tpng $< >$@

.gv.svg:
	fdp -Tsvg $< >$@

.g.gv:
	g2gv.sh $< >$@ 
