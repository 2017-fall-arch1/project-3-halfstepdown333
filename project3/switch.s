	.data
count:	.word 0; count=0

	.text

SP:	.word case1
	.word case2

	.global switchBeep

switchBeep:
	mov &count, r12
	add #1,r12
	mov SP(r12),r0

case1:
	mov #1000,r12
	call #buzzer_set_period;
	add #1,&count;
	jmp break

case2:
	mov #2000,r12
	call #buzzer_set_period;
	add #1,&count;
	jmp break
break:
	pop r0
