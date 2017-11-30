@Getting input
mov r0, #0
swi 0x6c

mov r6, r0	@Contains the value of n

mov r4, #1 	@Previous = 1
mov r5, #1	@Current = 1
mov r7, #1	@next = 1
mov r8, #0

cmp r6, r8
bne else_stub

	mov r0, #1
	mov r1, #0	@Print 0
	swi 0x6b
	b end_if

else_stub:

	mov r9, #3

	loop:
	add r7, r7, r4	@Updating Next
	mov r4, r5	@Prev=current
	mov r5,r7	@Current=next
	add r9, r9, #1
	cmp r6, r9	@Compare
	bgt loop	@branch if r9 > r6

	@r7 contains the value of the fibonacci number

	mov r0, #1
	mov r1, r7	@Moving Sum to output register

	swi 0x6b

end_if:

swi 0x11
