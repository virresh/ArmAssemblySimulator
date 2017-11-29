mov R3,#5
mov R2,#0
LDR R7,=0x5094
loop:
	mov R0,#0
	swi 0x6c
	STR R0,[R7,R2,LSL#2]
	ADD R2,R2,#1
	CMP R2, R3
	BNE loop

mov R0,#0
mov R2,#0
loop2:
	LDR R6,[R7,R2,LSL#2]
	ADD R0,R0,R6
	ADD R2,R2,#1
	CMP R2, R3
	BNE loop2

MOV R1,R0
mov R0,#1
swi 0x6b
