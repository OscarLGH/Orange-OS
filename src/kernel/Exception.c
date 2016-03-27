
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Exception.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
exception_handler
*----------------------------------------------------------------------*
Òì³£´¦Àí
*======================================================================*/
PUBLIC void ExceptionHandler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	unsigned int i = 0xffffffff;
	int Blue = 0xff0000ff;
	int White = 0xffffffff;
	char number[10];
	int BlueLayer = AllocateLayer(0,0,BootParam.ScreenX,BootParam.ScreenY);
	DrawRect(0,0,BootParam.ScreenX,BootParam.ScreenY,1,Blue,BlueLayer);
	char * err_msg[] = {"#DE Divide Error",
		"#DB RESERVED",
		"¡ª  NMI Interrupt",
		"#BP Breakpoint",
		"#OF Overflow",
		"#BR BOUND Range Exceeded",
		"#UD Invalid Opcode (Undefined Opcode)",
		"#NM Device Not Available (No Math Coprocessor)",
		"#DF Double Fault",
		"    Coprocessor Segment Overrun (reserved)",
		"#TS Invalid TSS",
		"#NP Segment Not Present",
		"#SS Stack-Segment Fault",
		"#GP General Protection",
		"#PF Page Fault",
		"¡ª  (Intel reserved. Do not use.)",
		"#MF x87 FPU Floating-Point Error (Math Fault)",
		"#AC Alignment Check",
		"#MC Machine Check",
		"#XF SIMD Floating-Point Exception"
	};
	
	disp_pos = 0;
	DrawString("A problem has been detected and os has been shut down to prevent damage to your computer.",0,0,White,BlueLayer);
	DrawString("An exception has occurred!", 0,32,White,BlueLayer);
	DrawString("If this is the first time you've seen this stop error screen ,restart your computer. If this screen appears again ,follow these steps:",0,64,White,BlueLayer);
	DrawString("Check to make sure any new hardware or software is properly installed.",0,96,White,BlueLayer);
	DrawString("If this is a new installation, ask your hardware or software or software manufacturer for any windows updates you might need.",0,128,White,BlueLayer);
	DrawString("If problems continue , disable or remove any newly istalled hardware of software .Disable Bios memory options such as caching or shadowing .",0,160,White,BlueLayer);
	DrawString("If you need to use safe mode to remove or disable components, restart your computer , press FS to select Advanced startup options ,and then select safe mode.",0,192,White,BlueLayer);
	DrawString("Technical information:",0,224,White,BlueLayer);
	DrawString(err_msg[vec_no], 0,240,White,BlueLayer);
	DrawString("EFLAGS: ", 0,272,White,BlueLayer);
	sprintf(number,"%x",eflags);
	DrawString(number,72,272,White,BlueLayer);
	DrawString("    CS: ", 152,272,White,BlueLayer);

	sprintf(number,"%x",cs);
	DrawString(number,224,272,White,BlueLayer);
	DrawString("   EIP: ", 304,272,White,BlueLayer);
	sprintf(number,"%x",eip);
	DrawString(number,376,272,White,BlueLayer);

	if(err_code != 0xFFFFFFFF){
		DrawString("   Error code: ", 0,336,White,BlueLayer);
		sprintf(number,"%x",eflags);
		DrawString(number,90,320,White,BlueLayer);
	}
	RefreshArea(0,0,BootParam.ScreenX,BootParam.ScreenY,BlueLayer);
	while(i--);
}
