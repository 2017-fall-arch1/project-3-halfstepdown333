	.file	"switch.c"
	.text
	.p2align 4,,15
	.globl	switchBeep
	.type	switchBeep, @function
switchBeep:
.LFB0:
	.cfi_startproc
	movl	4(%esp), %eax
	cmpl	$1, %eax
	je	.L3
	cmpl	$2, %eax
	jne	.L7
	movl	$2000, 4(%esp)
	jmp	buzzer_set_period
	.p2align 4,,10
	.p2align 3
.L7:
	rep ret
	.p2align 4,,10
	.p2align 3
.L3:
	movl	$1000, 4(%esp)
	jmp	buzzer_set_period
	.cfi_endproc
.LFE0:
	.size	switchBeep, .-switchBeep
	.globl	x
	.bss
	.align 4
	.type	x, @object
	.size	x, 4
x:
	.zero	4
	.ident	"GCC: (GNU) 7.1.1 20170622 (Red Hat 7.1.1-3)"
	.section	.note.GNU-stack,"",@progbits
