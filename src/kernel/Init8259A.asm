; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               Init8259A.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;															Oscar 2013.2
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
global	Init8259A
global  Enable_IRQ
global  Disable_IRQ

Init8259A:
	mov	al, 00010001b
	out	020h, al	; 主8259, ICW1.
	call	IO_Delay

	out	0A0h, al	; 从8259, ICW1.
	call	IO_Delay

	mov	al, 020h	; IRQ0 对应中断向量 0x20
	out	021h, al	; 主8259, ICW2.
	call	IO_Delay

	mov	al, 028h	; IRQ8 对应中断向量 0x28
	out	0A1h, al	; 从8259, ICW2.
	call	IO_Delay

	mov	al, 004h	; IR2 对应从8259
	out	021h, al	; 主8259, ICW3.
	call	IO_Delay

	mov	al, 002h	; 对应主8259的 IR2
	out	0A1h, al	; 从8259, ICW3.
	call	IO_Delay

	mov	al, 001h
	out	021h, al	; 主8259, ICW4.
	call	IO_Delay

	out	0A1h, al	; 从8259, ICW4.
	call	IO_Delay

	mov	al, 11111111b; 屏蔽所有中断
	out	021h, al	; 主8259, OCW1.
	call	IO_Delay

	mov	al, 11111111b	; 屏蔽从8259所有中断
	out	0A1h, al	; 从8259, OCW1.
	call	IO_Delay

	ret
IO_Delay:
	times 4 nop
	ret

; ========================================================================
;                  void Disable_IRQ(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8)
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	else
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
Disable_IRQ:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, 1
        rol     ah, cl                  ; ah = (1 << (irq % 8))
        cmp     cl, 8
        jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
        in      al, 21h
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     21h, al       ; set bit at master 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
disable_8:
        in      al, 0A1h
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     0A1h, al       ; set bit at slave 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
dis_already:
        popf
        xor     eax, eax                ; already disabled
        ret

; ========================================================================
;                  void Enable_IRQ(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8)
;               out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;       else
;               out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;
Enable_IRQ:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, ~1
        rol     ah, cl                  ; ah = ~(1 << (irq % 8))
        cmp     cl, 8
        jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
        in      al, 21h
        and     al, ah
        out     21h, al       ; clear bit at master 8259
        popf
        ret
enable_8:
        in      al, 0xA1
        and     al, ah
        out     0A1h, al       ; clear bit at slave 8259
        popf
        ret
