mov R4,#0
mov R5,#1
LDR R7,=0x5094
STR R4,[R7,#0]
STR R5,[R7,#4]
mov R3,#42
mov R2,#1

mov R8,#0x3B
mov R8,R8,LSL#8
add R8,R8,#0x9A
mov R8,R8,LSL#8
add R8,R8,#0xCA
mov R8,R8,LSL#8
add R8,R8,#0x09

loop:

	ADD R2,R2,#1
	CMP R2, R3
	BGT exit

    SUB R9,R2,#2
	LDR R4,[R7,R9,LSL#2]
    SUB R9,R2,#1
	LDR R5,[R7,R9,LSL#2]
	ADD R6,R4,R5

    CMP R6,R8
    BLT afterif
    SUB R6,R6,R8
    afterif:

	STR R6,[R7,R2,LSL#2]
    B loop

exit:
	LDR R0,[R7,R3,LSL#2]
