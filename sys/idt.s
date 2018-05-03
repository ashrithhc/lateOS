.global timer
.global keyboard

.global int0
.global int14
.global ISR128

.extern intTimer
.extern intWrite
.extern int0
.extern int14

.macro popRegs
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

.macro pushRegs
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
	pushRegs
	callq intTimer
	popRegs
	iretq

keyboard:
	pushRegs
	callq intWrite
	popRegs
	iretq

int0:
	pushRegs
	callq int0
	popRegs
	iretq

int14:
	pushRegs
	callq int14
	popRegs
	add $8, %rsp
	iretq

ISR128:
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
