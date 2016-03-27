; CPU_ID.asm
; Copyright (c) 2009-2012 mik 
; All rights reserved.
; Test CPUID
global TestCPUID
global CPU_ID

;**********************************************************
;*Function Name: TestCPUID								  *
;*Function Prototype: int TestCPUID();					  *
;*Description:											  *
;*	Test whether current cpu supports cpuid instruction	  *
;*Parameter:											  *
;*	None												  *
;*Return Value:											  *
;*	eax: 1 - Support,0 - Not Support					  *
;**********************************************************
TestCPUID:
	pushfd
	mov eax, dword[esp]
	xor dword [esp], 0x200000
	popfd
	pushfd
	pop ebx
	cmp eax, ebx
	setnz al
	movzx eax, al
	ret

;**********************************************************
;*Function Name: CPU_ID									  *
;*Description:											  *
;*	cpuid instruction									  *
;*Function Prototype: 									  *
;*	void CPU_ID(int *EAX,int *EBX,int *ECX,int *EDX)	  *
;*Parameter:											  *
;*	In_Out: int *EAX									  *
;*	Out:	int *EBX									  *
;*	In_Out:	int	*ECX									  *
;*	Out:	int *EDX									  *
;*	ebx[23:16]:logical processor						  *
;*Notice:												  *
;*	This function can be called only if current CPU 	  *
;*	supports cpuid instruction.							  *
;*Return Value:											  *
;*None													  *
;**********************************************************
CPU_ID:
	push ebp
	push edi
	mov ebp, esp


	mov edi, [ebp + 12]			 ; input index:eax,ecx
	mov eax, [edi]
	mov edi, [ebp + 20]
	mov ecx, [edi]					
	cpuid
	mov edi, [ebp + 12]
	mov [edi], eax
		
	mov edi, [ebp + 16]
	mov [edi], ebx

	mov edi, [ebp + 20]
	mov [edi], ecx

	mov edi, [ebp + 24]
	mov [edi], edx

	pop edi
	pop ebp

	ret
