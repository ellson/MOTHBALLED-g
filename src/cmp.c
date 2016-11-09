#define MAXNEST 5

typedef struct {
    elem_t *next[MAXNEST];
    int nest;
    char *cp,
    int len;
} iter_t;

static void init(iter_t *iter, elem_t *elem)
{
    assert(iter->nest < MAXNEST);
    switch (elem->type) {
        case FRAGELEM:
            iter->cp = elem->u.f.frag;
            iter->len = elem->len;
            iter->next[iter->nest] = elem->u.f.next;
            break;
        case SHORTSTRELEM:
            iter->cp = &(elem->u.s.str);
            iter->len = elem->len;
            iter->next[iter->nest] = NULL;
            break;
        case LISTELEM:
            iter->next[iter->nest++] = elem->u.l.next;
            init(iter, elem->u.l.first);
            break;
        default:
            assert(0);
            break;
    }
}

int compare (elem_t *A, elem_t *B)
{
    char a, b, rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    init(&ai, A);
    init(&bi, B);
    do {
        do { 
            a = *ai.cp++; ai.len--;
            b = *bi.cp++; bi.len--;
            rc = a - b;
        } while (ai.len && bi.len && !rc);
        if (! ai.len) {
            if (ai.next[ai.nest]) { 
                ai.cp = ai.next[ai.nest]->u.f.next;
                ai.len = ai.next[ai.nest]->len;
                ai.next[ai.nest] = ai.next[ai.nest]->u.f.next;
                a = *ai.cp;
            } else {
                while (--a.nest && ! ai.next[ai.nest]) {}
                if (ai.nest) {
                    a = ' ';
		    init(&ai, ai.next[ai.nest]->u.l.first);
		} else {
                    b = '\0';
		}
            }
        }
        if (! bi.len) {
            if (bi.next[bi.nest]) { 
                bi.cp = bi.next[bi.nest]->u.f.next;
                bi.len = bi.next[bi.nest]->len;
                bi.next[bi.nest] = bi.next[bi.nest]->u.f.next;
                b = *bi.cp;
            } else {
                while (--b.nest && ! bi.next[bi.nest]) {}
                if (bi.nest) {
                    b = ' ';
		    init(&bi, bi.next[bi.nest]->u.l.first);
		} else {
                    b = '\0';
                }
            }
        }
        rc = a - b;
    } while (ai.nest && bi.nest && !rc);
    return rc;
}
