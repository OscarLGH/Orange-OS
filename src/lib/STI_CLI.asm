
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                              STI_CLI.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                       Oscar, 2013
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
[SECTION .text]
; µ¼³öº¯Êý
global	EnableInterruption
global  DisableInterruption

EnableInterruption:
	sti
	ret
DisableInterruption:
	cli
	ret
