.code64
startup_64:
	xorl	%eax, %eax
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %ss
	movl	%eax, %fs
	movl	%eax, %gs
	leaq	startup_32(%rip) /* - $startup_32 */, %rbp
	movl	BP_kernel_alignment(%rsi), %eax
	decl	%eax
	addq	%rax, %rbp
	notq	%rax
	andq	%rax, %rbp
	cmpq	$LOAD_PHYSICAL_ADDR, %rbp
	jge	1f
	movq	$LOAD_PHYSICAL_ADDR, %rbp
1:
	movl	BP_init_size(%rsi), %ebx
	subl	$_end, %ebx
	addq	%rbp, %rbx
	leaq	boot_stack_end(%rbx), %rsp
	pushq	%rsi
	call	l5_paging_required
	popq	%rsi
	cmpq	$0, %rax
	je	lvl5
	leaq	lvl5_pgtable(%rbx), %rdi
	xorq	%rax, %rax
	movq	$(0x1000/8), %rcx
	rep	stosq
	movq	%cr3, %rdi
	leaq	0x7 (%rdi), %rax
	movq	%rax, lvl5_pgtable(%rbx)
	pushq	$__KERNEL32_CS
	leaq	compatible_mode(%rip), %rax
	pushq	%rax
	lretq
lvl5:
	pushq	$0
	popfq
	pushq	%rsi
	leaq	(_bss-8)(%rip), %rsi
	leaq	(_bss-8)(%rbx), %rdi
	movq	$_bss /* - $startup_32 */, %rcx
	shrq	$3, %rcx
	std
	rep	movsq
	cld
	popq	%rsi
	leaq	relocated(%rbx), %rax
	jmp	*%rax
efi_pe_entry:
	movq	%rcx, efi64_config(%rip)	/* Handle */
	movq	%rdx, efi64_config+8(%rip) /* EFI System table pointer */
	leaq	efi64_config(%rip), %rax
	movq	%rax, efi_config(%rip)
	call	1f
1:	popq	%rbp
	subq	$1b, %rbp
	addq	%rbp, efi64_config+40(%rip)
	movq	%rax, %rdi
	call	make_boot_params
	cmpq	$0,%rax
	je	fail
	mov	%rax, %rsi
	leaq	startup_32(%rip), %rax
	movl	%eax, BP_code32_start(%rsi)
	jmp	2f		/* Skip the relocation */
handover_entry:
	call	1f
1:	popq	%rbp
	subq	$1b, %rbp
	movq	efi_config(%rip), %rax
	addq	%rbp, 40(%rax)
2:
	movq	efi_config(%rip), %rdi
	call	efi_main
	movq	%rax,%rsi
	cmpq	$0,%rax
	jne	2f
fail:
	hlt
	jmp	fail
2:
	movl	BP_code32_start(%esi), %eax
	leaq	startup_64(%rax), %rax
	jmp	*%rax
efi64_stub_entry:
	movq	%rdi, efi64_config(%rip)	/* Handle */
	movq	%rsi, efi64_config+8(%rip) /* EFI System table pointer */
	leaq	efi64_config(%rip), %rax
	movq	%rax, efi_config(%rip)
	movq	%rdx, %rsi
	jmp	handover_entry
relocated:
	xorl	%eax, %eax
	leaq    _bss(%rip), %rdi
	leaq    _ebss(%rip), %rcx
	subq	%rdi, %rcx
	shrq	$3, %rcx
	rep	stosq
	leaq	_got(%rip), %rdx
	leaq	_egot(%rip), %rcx
1:
	cmpq	%rcx, %rdx
	jae	2f
	addq	%rbx, (%rdx)
	addq	$8, %rdx
	jmp	1b
2:
	pushq	%rsi			/* Save the real mode argument */
	movq	%rsi, %rdi		/* real mode address */
	leaq	boot_heap(%rip), %rsi	/* malloc area for uncompression */
	leaq	input_data(%rip), %rdx  /* input_data */
	movl	$z_input_len, %ecx	/* input_len */
	movq	%rbp, %r8		/* output target address */
	movq	$z_output_len, %r9	/* decompressed length, end of relocs */
	call	extract_kernel		/* returns kernel location in %rax */
	popq	%rsi
	jmp	*%rax
