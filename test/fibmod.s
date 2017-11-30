mov R5,#1
LDR R7, =0x5094
STR R4,[R7,#0]
STR R5,[R7,#4]
swi 0x6c
MOV R3,R0
MOV R0,#0
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
	BGE exit

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
    SUB R3,R3,#1
    MOV R0,#1
	LDR R1,[R7,R3,LSL#2]
    swi 0x6b
swi 0x11
