

/**
 * scan input for a single character of the indicated character set
 *
 * starts scanning at TOKEN->in, 
 * updates TOKEN->in, TOKEN->insi
 *
 * @param TOKEN context
 * @param si character class to be scanned for
 *
 * @return size of token
 */

size_t scan_1 (TOKEN_t * TOKEN, state_t si)
{
    unsigned char *in = TOKEN->in;
    state_t ci;

    if (in != TOKEN->end) {
        ci = char2state[*in++];
        if (ci != si) {
            return 0;
        }
        TOKEN->insi = ci;
        TOKEN->in = in;
        return 1;
    }
    TOKEN->insi = END;
    return 0;
}

/**
 * scan input for multiple characters of the indicated character set
 *
 * starts scanning at TOKEN->in, 
 * updates TOKEN->in, TOKEN->insi
 *
 * @param TOKEN context
 * @param si character class to be scanned for
 *
 * @return size of token
 */

size_t scan_n (TOKEN_t * TOKEN, state_t si)
{
    unsigned char *in = TOKEN->in;
    state_t ci;
    size_t sz = 0;

    while (in != TOKEN->end) {
        ci = char2state[*in++];
        if (ci != si) {
            TOKEN->insi = ci;
            TOKEN->in = in;
            return sz;
        }
        sz++;
    }
    TOKEN->insi = END;
    TOKEN->in = in;
    return sz;
}

