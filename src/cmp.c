typedef struct {
    elem_t next;
    char *cp,
    int len;
} iter_t;

static void init(iter_t *iter, elem_t *elem)
{
    switch (elem->type) {
        case FRAGELEM:
            iter->cp = elem->u.f.frag;
            iter->len = elem->len;
            iter->next = elem->u.f.next
            break;
        case SHORTSTRELEM:
            iter->cp = &(elem->u.s.str);
            iter->len = elem->len;
            iter->next = NULL;
            break;
        case LISTELEM:
            // ???
            break;
        default:
            assert(0);
    }
}

static char next(iter_t *iter)
{

    if (! iter->len) {
        if (! iter->next) {
            return '\0';
        }
        iter->cp = next->u.f.frag;
        iter->len = next->len;
        iter->next = next->u.f.next
    }
    assert(len);
    iter->len--;
    return *(iter->cp)++;
}

char compare (elem_t *A, elem_t *B)
{
    char a, b, rc;
    iter_t ai, bi;

    init(&ai, A);
    init(&bi, B);
    do { 
        a = next(&ai);
        b = next(&bi);
        rc = a - b;
    } while (a && b && !rc);
    return rc;
}
