<ACT ACTION>[OPT]<ACT SUBJECT><ACT ATTRIBUTES>[OPT]<ACT CONTAINER>[OPT]<ACT TERM>[OPT]
<ACTION TLD>
<SUBJECT OBJECT>[ALT]<SUBJECT OBJECT_LIST>[ALT]
<ATTRIBUTES LBR><ATTRIBUTES ATTR>[OPT SREP]<ATTRIBUTES RBR>
<CONTAINER LBE><CONTAINER ATTRIBUTES>[OPT]<CONTAINER ACT>[OPT SREP REC]<CONTAINER RBE>
<OBJECT_LIST LPN><OBJECT_LIST OBJECT>[OPT SREP]<OBJECT_LIST RPN>
<OBJECT EDGE>[ALT]<OBJECT NODE>[ALT]
<EDGE LAN><EDGE LEG><EDGE LEG>[SREP]<EDGE RAN><EDGE DISAMBIGUATOR>[OPT]
<LEG ENDPOINT_SET>[ALT]<LEG ENDPOINT>[ALT]
<ENDPOINT_SET LPN><ENDPOINT_SET ENDPOINT>[SREP]<ENDPOINT_SET RPN>
<ENDPOINT COUSIN>[ALT]<ENDPOINT SIBLING>[ALT]<ENDPOINT PORT>[ALT]
<COUSIN PARENT>[REP]<COUSIN CHILD>[REP]<COUSIN PORT>[OPT]
<PARENT HAT><PARENT FSL>
<SIBLING NODE><SIBLING CHILD>[OPT REP]<SIBLING PORT>[OPT]
<CHILD FSL><CHILD NODE>
<PORT CLN><PORT PORTID>
<NODE NODEID><NODE DISAMBIGUATOR>[OPT]
<DISAMBIGUATOR TIC><DISAMBIGUATOR DISAMBID>
<ATTR ATTRID><ATTR VALASSIGN>[OPT]
<VALASSIGN EQL><VALASSIGN VALUE>
<NODEID STRING>
<DISAMBID STRING>
<PORTID STRING>
<ATTRID STRING>
<VALUE STRING>
<STRING FRAG>[REP]
<TERM SCN>
BIN{01 02 03 04 05 06 07 08 0b 0c 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 7f}
ABC{21 24 25 26 2b 2c 2d 2e 30 31 32 33 34 35 36 37 38 39 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5f 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7c}
UTF{80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff}
WS{09 20}
DQT{22}
SQT{27}
LPN{28}
RPN{29}
LAN{3c}
RAN{3e}
LBR{5b}
RBR{5d}
LBE{7b}
RBE{7d}
FSL{2f}
BSL{5c}
OCT{23}
AST{2a}
CLN{3a}
SCN{3b}
EQL{3d}
HAT{5e}
TIC{60}
TLD{7e}
LF{0a}
CR{0d}
NUL{00}
