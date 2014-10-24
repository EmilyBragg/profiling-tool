	.file	"lll3.c"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"Done! %f\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB24:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	xorpd	%xmm2, %xmm2
	movl	$100000, %esi
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$80016, %rsp
	movq	%rsp, %rcx
	subq	$80016, %rsp
	movq	%rsp, %rdx
	.p2align 4,,10
	.p2align 3
.L2:
	movapd	%xmm2, %xmm0
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L5:
	movsd	(%rcx,%rax), %xmm1
	mulsd	(%rdx,%rax), %xmm1
	addq	$8, %rax
	cmpq	$80000, %rax
	addsd	%xmm1, %xmm0
	jne	.L5
	subl	$1, %esi
	jne	.L2
	movl	$.LC1, %esi
	movl	$1, %edi
	movl	$1, %eax
	call	__printf_chk
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE24:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
