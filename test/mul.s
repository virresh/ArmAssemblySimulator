LDR R7,=mat1
LDR R8,=mat2
LDR R9,=mat3

loop:
	MOV R0,#0
	swi 0x6c
	str R0,[R7,R2,lsl#2]
	add R2,R2,#1
	cmp R2,#4
	BNE loop

MOV R2,#0
loop2:
	MOV R0,#0
	swi 0x6c
	str R0,[R8,R2,lsl#2]
	add R2,R2,#1
	cmp R2,#4
	BNE loop2

LDR R0,[R7,#0]
LDR R1,[R7,#4]

LDR R2,[R8,#0]
LDR R3,[R8,#8]

MUL R4,R0,R2
MUL R5,R1,R3
ADD R6,R4,R5

STR R6,[R9,#0]

LDR R0,[R7,#0]
LDR R1,[R7,#4]

LDR R2,[R8,#4]
LDR R3,[R8,#12]

MUL R4,R0,R2
MUL R5,R1,R3
ADD R6,R4,R5

STR R6,[R9,#4]

LDR R0,[R7,#8]
LDR R1,[R7,#12]

LDR R2,[R8,#0]
LDR R3,[R8,#8]

MUL R4,R0,R2
MUL R5,R1,R3
ADD R6,R4,R5

STR R6,[R9,#8]

LDR R0,[R7,#8]
LDR R1,[R7,#12]

LDR R2,[R8,#4]
LDR R3,[R8,#12]

MUL R4,R0,R2
MUL R5,R1,R3
ADD R6,R4,R5

STR R6,[R9,#12]

mov R0,#1
LDR R1,[R9,#0]
swi 0x6b
LDR R1,[R9,#4]
swi 0x6b
LDR R1,[R9,#8]
swi 0x6b
LDR R1,[R9,#12]
swi 0x6b
swi 0x11
mat1: .skip 20
mat2: .skip 20
mat3: .skip 20
