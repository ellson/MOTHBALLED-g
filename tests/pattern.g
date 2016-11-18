*[x=y]         # add pattern
b              # match pattern
*              # delete pattern
b              # no more pattern to match

a*[x=y]        # add pattern
a              # match pattern
abc            # match pattern
b              # don't match pattern
aa             # match pattern
ba             # don't match pattern
a*             # delete pattern
a              # no more pattern to match

(*)[x=y]       # add pattern
a              # don't match pattern
(a)            # match pattern
(*)            # delete pattern
(a)            # no more pattern to match

(* b *)[x=y]   # add pattern
b              # don't match pattern
(a b c)        # match pattern
(a x c)        # don't match pattern
(* b *)        # delete pattern
(a b c)        # no more pattern to match

#a*[b=c] ab ab*[e=f] abc[g=h]

#<a b*> [default=true] {some content}
#<a c*> [default=false] {different stuff}
#<a aa> 
#<a bb>
#<a cc>
