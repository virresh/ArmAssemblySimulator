mov r4, #0	@r4 initialised to 0
mov r5, #1	@r5 initialised to 1
mov r4, r5	@ r4 = r5
mov r0, #1
mov r1, r4
swi 0x6b
swi 0x11