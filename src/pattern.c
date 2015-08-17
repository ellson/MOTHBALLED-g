// A pattern is a SUBJECT in which one of more STRINGs contain an AST ('*')
// The AST is a wild-card that matches any substring of zero or more 
// characters at that position.
//
// An indivual STRING may have no more that one AST, but multiple
// STRING in the SUBJECT may have AST
//
// The following are valid pattern STRING:
// 	*
// 	ab*ef
//      *cdef
//      abcd*
//
// and all of these patterns will match
//      abcdef

// A SUBJECT is matched if all its pattern and non-pattern STRINGS
// match, after pattern substitution.

// ENDPOINTSETs are not expanded in patterns, or in SUBJECTss
// before pattern matching.
