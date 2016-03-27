global PCI_I
global PCI_O

PCI_I:
	push ebp
	push edi
	mov ebp, esp
	
	mov eax, [ebp+12]
	or  eax, 80000000h
	mov dx,  0cf8h
	out dx,	 eax
	mov dx,  0cfch
	in  eax, dx
	
	pop edi
	pop ebp

	ret
	
	
PCI_O:
	push ebp
	push edi
	mov ebp, esp
	
	mov eax, [ebp+12]
	or  eax, 80000000h
	mov dx,  0cf8h
	out dx,	 eax
	mov dx,  0cfch
	out dx,  eax
	
	pop edi
	pop ebp

	ret
	