/*
 * setjmp.s
 *
 *  Created on: Aug 5, 2009
 *      Author: misc
 *  $Id$
 */


	.align 2
	.global __aeabi_ldivmod
	.type __aeabi_ldivmod,%function
__aeabi_ldivmod:
	sub		sp, sp, #8	// reserve space for remainder
	push	{sp, lr}	// Store address of 8-byte space and link register on stack
	bl		__ldivmod_helper
	ldr		lr, [sp, #4]	// restore link reg
	add		sp, sp, #8 // skip  lr and sp on the stack
	pop		{r2, r3}	// get remainder from the stack
	bx		lr
