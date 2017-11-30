mov r0, #0
swi 0x6c
mov r6, r0	@Contains the value of n
mov r4, #0 	@Sum=0
mov r5, #0	@Counter i.e i
loop:
mov r0, #0
swi 0x6c	@Input ith number
add r4, r4, r0	@Updating Sum
add r5, r5, #1	@Incrementing counter
cmp r5, r6	@Compare
ble loop	@branch if r5 <= r6
mov r0, #1
mov r1, r4	@Moving Sum to output register
swi 0x6b
swi 0x11
