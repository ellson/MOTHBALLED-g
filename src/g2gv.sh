echo "graph {"
while read op t h x x; do
    if test "$op" = "<"; then
	echo "  $t -> $h"
    fi
done
echo "}"
