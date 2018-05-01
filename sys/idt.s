.global timer
.global keyboard

.global isr_0
.global isr_14
.global isr_128

.extern timer_handler
.extern write_terminal
.extern isr0
.extern isr14

.macro pop
popq %r15
popq %r14
popq %r13
popq %r12
popq %r11
popq %r10
popq %r9
popq %r8
popq %rbp
popq %rdi
popq %rsi
popq %rdx
popq %rcx 
popq %rbx 
popq %rax 
.endm

.macro push
pushq %rax 
pushq %rbx 
pushq %rcx 
pushq %rdx 
pushq %rsi 
pushq %rdi
pushq %rbp
pushq %r8
pushq %r9
pushq %r10
pushq %r11
pushq %r12
pushq %r13
pushq %r14
pushq %r15
.endm

timer:
	push
	callq timer_handler
	pop
	iretq

keyboard:
	push
	callq write_terminal
	pop
	iretq

isr_0:
	push
	callq isr0
	pop
	iretq

isr_14:
	push
	callq isr14
	pop
	add $8, %rsp
	iretq

isr_128:
	cli
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	movq %rsp, %rdi
	callq isr128
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9 
	popq %r8 
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rdx
	popq %rcx 
	popq %rbx 
	sti	
	iretq
