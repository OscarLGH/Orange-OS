; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               Interruption.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;															Oscar 2013.2
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; 栈顶
[SECTION .text]

%include "sconst.inc"

extern	p_proc_ready
extern	tss
extern	k_reenter
extern	ClockHandler
extern	irq_table
extern  sys_call_table

; 导出函数
global  hwint00
global  hwint01
global  hwint02
global  hwint03
global  hwint04
global  hwint05
global  hwint06
global  hwint07
global  hwint08
global  hwint09
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15

global  sys_call

; 中断和异常 -- 硬件中断
; ---------------------------------
%macro  hwint_master    1
		call	save
		in		al, 21h 
		or		al, (1 << %1)
		out		21h, al
		mov		al, 20h			; EOI
		out		20h, al
		sti
        push    %1
        call    [irq_table + 4 * %1]
		pop		ecx
		cli
		in		al, 21h 
		and		al, ~(1 << %1)
		out		21h, al
		ret									; 重入时跳转到.restart_reenter,因为.1将.restart_reenter压栈

%endmacro
; ---------------------------------

sys_call:
		call	save
		push	dword [p_proc_ready]		  ; 将当前进程指针传递给sys_write
		sti
		push	edx
		push	ecx
		push	ebx

		call	[sys_call_table + eax * 4]
		add		esp, 4*4
		mov		[esi + EAXREG - P_STACKBASE], eax
		cli
		ret
save:
		pushad
		push	ds
		push	es
		push	fs
		push	gs
							   ; 直到切换内核栈之前不可使用push pop指令，会破坏进程表，所以使用edx传递参数

		mov		esi, edx      ; 保存edx，系统调用将使用edx传递参数

		mov		dx, ss
		mov		ds, dx
		mov		es, dx

		mov		edx, esi		; 恢复edx

		mov		esi, esp			; esi进程表起始地址	 eax要保存系统调用的参数
		inc		dword[k_reenter]			; 判断全局变量k_reenter的值，防止中断重入使堆栈溢出
		cmp		dword[k_reenter], 0
		jne		.1
		mov		esp, StackTop		; 切换到内核栈
		push	.restart
		jmp		[esi + RETADR - P_STACKBASE]
.1:
		push	.restart_reenter
		jmp		[esi + RETADR - P_STACKBASE]
.restart:
		mov		esp, [p_proc_ready]	; 离开内核栈
		lldt	[esp + P_LDT_SEL]				; 进程切换时要重新加载ldt
		lea		esi, [esp + P_STACKTOP]
		mov		dword[tss + TSS3_S_SP0], esi				; 赋值tss.esp0
.restart_reenter:
		dec		dword[k_reenter]				; 之前漏写这句导致中断处理程序只能执行一次
		pop		gs
		pop		fs
		pop		es
		pop		ds
		popad
		add		esp, 4
		iretd

ALIGN   16
hwint00:                ; Interrupt routine for irq 0 (the clock).
		hwint_master    0

ALIGN   16
hwint01:                ; Interrupt routine for irq 1 (keyboard)
		hwint_master    1

ALIGN   16
hwint02:                ; Interrupt routine for irq 2 (cascade!)
        hwint_master    2

ALIGN   16
hwint03:                ; Interrupt routine for irq 3 (second serial)
        hwint_master    3

ALIGN   16
hwint04:                ; Interrupt routine for irq 4 (first serial)
        hwint_master    4

ALIGN   16
hwint05:                ; Interrupt routine for irq 5 (XT winchester)
        hwint_master    5

ALIGN   16
hwint06:                ; Interrupt routine for irq 6 (floppy)
        hwint_master    6

ALIGN   16
hwint07:                ; Interrupt routine for irq 7 (printer)
        hwint_master    7

; ---------------------------------
%macro  hwint_slave     1
        call	save
		in		al, 0A1h 
		or		al, (1 << %1)
		out		0A1h, al
		mov		al, 20h			; EOI
		out		20h,  al		;主片EOI
		out		0A0h, al		;从片EOI
		sti
        push    %1
        call    [irq_table + 4 * %1]
		pop		ecx
		cli
		in		al, 0A1h 
		and		al, ~(1 << %1)
		out		0A1h, al
		ret									; 重入时跳转到.restart_reenter,因为.1将.restart_reenter压栈
%endmacro
; ---------------------------------

ALIGN   16
hwint08:                ; Interrupt routine for irq 8 (realtime clock).
        hwint_slave     8

ALIGN   16
hwint09:                ; Interrupt routine for irq 9 (irq 2 redirected)
        hwint_slave     9

ALIGN   16
hwint10:                ; Interrupt routine for irq 10
        hwint_slave     10

ALIGN   16
hwint11:                ; Interrupt routine for irq 11
        hwint_slave     11

ALIGN   16
hwint12:                ; Interrupt routine for irq 12
        hwint_slave     12

ALIGN   16
hwint13:                ; Interrupt routine for irq 13 (FPU exception)
        hwint_slave     13

ALIGN   16
hwint14:                ; Interrupt routine for irq 14 (AT winchester)
        hwint_slave     14

ALIGN   16
hwint15:                ; Interrupt routine for irq 15
        hwint_slave     15


