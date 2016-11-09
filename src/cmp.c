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
            iter->next[iter->nest] = elem->u.f.next
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

static char next(iter_t *iter)
{
    if (! iter->len) {
        while (! iter->next[iter->nest]) {
            if (! --(iter->nest)) return '\0';
            return ' ';
        }
        iter->next[iter->nest++] = elem->u.l.next;
        init(iter, iter->next[iter->nest]->u.l.first);
    }
    iter->len--;
    return *(iter->cp)++;
}

char compare (elem_t *A, elem_t *B)
{
    char a, b, rc;
    iter_t ai = { 0 };
    iter_t bi = { 0 };

    init(&ai, A);
    init(&bi, B);
    do { 
        a = next(&ai);
        b = next(&bi);
        rc = a - b;
    } while (a && b && !rc);
    return rc;
}
