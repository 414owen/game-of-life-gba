	.text;
	.arm;
	.align 2;
	.global halt;
	.type halt STT_FUNC;
halt:
	swi		0x02
	bx		lr
