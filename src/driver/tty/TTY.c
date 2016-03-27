
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TTY.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//?a??òa°üo??úproto.h???°·??òprotoà??3D???ò?2?ê?±e
#include "proto.h"
#include "global.h"
#include "keyboard.h"
#include "tty.h"

#define TTY_FIRST (tty_table)
#define TTY_END	  (tty_table + NR_CONSOLES)

TTY	    tty;
CONSOLE  console;

void InitTTY()
{
	InitScreen(&tty,0,0,640,400);
}

int IsCurrentConsole(CONSOLE * p_con)
{
	return 1;
}

/*======================================================================*
			    SetCursor
 *======================================================================*/
static void SetCursor(int position)
{
	
	
}
/*======================================================================*
			    SetVideoStartAddr
 *======================================================================*/
static void SetVideoStartAddr(CONSOLE * p_con,int line)
{
	int i,j;
	int n_line = 0;
	int pos = 0;
	char * str = 0;
	
	for(i=0;i<p_con->char_count;i++)
	{
		if(p_con->char_buf[i] == 0)
			n_line++;
		if(n_line == line)
			break;
	}
	
	ZeroLayer(p_con-> gralayer2);
	for(j=0;j<24;j++)
	{
		for(;;i++)
		{
			if(p_con->char_buf[i] == 0)
			{
				i++;
				break;
			}
		}
		str = &p_con->char_buf[i]; 
		DrawString(str,0,pos,0xffffffff,p_con-> gralayer2);
		pos+=16;
	}
	RefreshArea(0,0,p_con-> window_len,p_con-> window_wid,p_con-> gralayer2);
}


void InitScreen(TTY * p_tty,int PosX,int PosY,int Len,int Wid)
{
	p_tty -> p_console = &console;								//结构体内的指针变量必须先赋值后对其指针成员变量赋值！之前就因为直接对成员赋值导致错误！
	p_tty-> p_console-> window_len = Len;
	p_tty-> p_console-> window_wid = Wid;
	p_tty-> p_console-> gralayer1 = AllocateLayer(PosX,PosY,Len,Wid+16);
	p_tty-> p_console-> gralayer2 = AllocateLayer(PosX,PosY+16,Len,Wid);		//初始化TTY时task_sys进程还没有运行，此时调用用户级分配函数AllocateGraph发送消息将导致死锁
	int color1 = 0xff44433f;
	int color2 = 0xff301024;
	int color3 = 0xffffffff;
	int WindowSize = Len*Wid;
	p_tty->p_console->window_size_limit   = WindowSize;
	
	/*??è?1a±ê?ú×??eê???*/
	p_tty->p_console->cursor = 0;
	p_tty->p_console->char_current_pos = 0;
	p_tty->p_console->char_count = 0;
	p_tty->p_console->char_current_line = 0;
	p_tty->p_console->char_start_line = 1;
	
	
	p_tty->p_console->char_buf[p_tty->p_console->char_current_pos++] = 0;
	
	DrawRect(0,0,Len,16,1,color1,p_tty-> p_console-> gralayer1);
	DrawString("Terminal",0,0,color3,p_tty-> p_console-> gralayer1);
	DrawRect(0,16,Len,Wid,1,color2,p_tty-> p_console-> gralayer1);
	RefreshArea(0,0,Len,Wid+16,p_tty-> p_console-> gralayer1);
	//SetCursor(p_tty->p_console->cursor);
	p_tty->inbuf_count = 0;										//之前漏写这两句初始化导致输入字符过多时出现异常
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;
}


/*======================================================================*
			   OutChar
 *======================================================================*/
void OutChar(CONSOLE* p_con, char ch)
{
	int Len = p_con-> window_len;
	int Wid = p_con-> window_wid;
	int Color = 0xffffffff;
	int i,j;
	int line;
	while(p_con->cursor >= p_con->window_size_limit)				/* 超出显示边界 */
	{	
		ScrollScreen(p_con, SCR_UP, 1);					//滚屏
		p_con->cursor -= Len*16;
	}
	if(p_con->char_current_line-p_con->char_start_line > Wid/16 - 1)
	{
		line = p_con->char_current_line - Wid/16 - 1 - p_con->char_start_line;
		ScrollScreen(p_con, SCR_UP, line);	
	}
	if(p_con->char_current_pos >= 10000)
				p_con->char_current_pos = 0;
	switch(ch)
	{
	case '\n':
		if(p_con->cursor < p_con->window_size_limit)
		{
			if(p_con->char_current_line>=24)
			{
				p_con->char_current_line++;
				ScrollScreen(p_con, SCR_UP, 1);
				p_con->cursor -= p_con->cursor%Len;
			}
			else
			{
				p_con->cursor += (Len*16 - p_con->cursor%Len);
				p_con->char_current_line++;
			}
			p_con->char_count++;
			p_con->char_buf[p_con->char_current_pos++] = 0;
			SetCursor(p_con->cursor);
		}
		break;
	case '\b':
		if(p_con->cursor > 0)
		{
			if(p_con->cursor%Len<=7)
				p_con->cursor -= (Len*15 + 8);
			else
				p_con->cursor -= 8;
			SetCursor(p_con->cursor);
			DrawRect(p_con->cursor%Len,p_con->cursor/Len,8,16,1,0xff301024,p_con->gralayer2);
			RefreshArea(p_con->cursor%Len,p_con->cursor/Len,8,16,p_con->gralayer2);
		}
		break;
	default:
		if(p_con->cursor < p_con->window_size_limit - 1)
		{
			
			if((p_con->cursor%Len+8)>Len)
			//处理到达显示边界的情况
			{
				p_con->cursor += (Len*16 - p_con->cursor%Len);
				p_con->char_buf[p_con->char_current_pos++] = 0;
				p_con->char_current_line++;
				p_con->char_count++;
				SetCursor(p_con->cursor);
			}
			else if((p_con->cursor%Len+8)==Len)
			{
				DrawChar(ch,p_con->cursor%Len,p_con->cursor/Len,Color,p_con-> gralayer2);
				RefreshArea(p_con->cursor%Len,p_con->cursor/Len,8,16,p_con->gralayer2);
				p_con->cursor += (Len*16 - p_con->cursor%Len);
				p_con->char_buf[p_con->char_current_pos++] = ch;
				p_con->char_buf[p_con->char_current_pos++] = 0;
				p_con->char_current_line++;
				p_con->char_count++;
				SetCursor(p_con->cursor);
			}
			else
			{
				DrawChar(ch,p_con->cursor%Len,p_con->cursor/Len,Color,p_con-> gralayer2);
				RefreshArea(p_con->cursor%Len,p_con->cursor/Len,8,16,p_con->gralayer2);
				p_con->cursor += 8;
				p_con->char_buf[p_con->char_current_pos++] = ch;
				p_con->char_count++;
				SetCursor(p_con->cursor);
			}
		
		}
		break;
	}
}




static TTY_Read(TTY * p_tty)
{
	if(IsCurrentConsole(p_tty->p_console))
	{
		ReadKeyboard(p_tty);
	}
}

static TTY_Write(TTY * p_tty)
{
	if (p_tty->inbuf_count) 
	{
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
		{
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		OutChar(p_tty->p_console, ch);
	}
}

void TaskTTY()
{
	SelectConsole(0);   //??è?μú0??????ì?
	InitKeyboard();
	while(1)
	{
		TTY_Read(&tty);
		TTY_Write(&tty);
	}
}

static void PutKey(TTY* p_tty, u32 key)
{
	switch(key)
	{
		case '1':
			Speaker(262,10);
			break;
		case '2':
			Speaker(294,10);
			break;
		case '3':
			Speaker(330,10);
			break;
		case '4':
			Speaker(349,10);
			break;
		case '5':
			Speaker(392,10);
			break;
		case '6':
			Speaker(440,10);
			break;
		case '7':
			Speaker(494,10);
			break;
		default:
			Speaker(1000,5);
			break;
	}
	if(p_tty->inbuf_count < TTY_IN_BYTES ) //???1?òμúò???????ì?ê?è?×?·?
		{
			*(p_tty->p_inbuf_head) = key;
			p_tty -> p_inbuf_head++;
			if(p_tty->p_inbuf_head == p_tty -> in_buf + TTY_IN_BYTES)
				p_tty -> p_inbuf_head = p_tty -> in_buf;

			p_tty -> inbuf_count++;
		}
}
void ProcessKey(TTY * p_tty, u32 key)
{
	if(!(key & FLAG_EXT))
	{
		PutKey(p_tty, key);
	}
	else {
		int raw_code = key & MASK_RAW;
		switch(raw_code) {
		case UP:
				ScrollScreen(p_tty->p_console, SCR_DN, 1);
			break;
		case DOWN:
				ScrollScreen(p_tty->p_console, SCR_UP, 1);
			break;
		case LEFT:
				p_tty->p_console->cursor-=8;
				SetCursor(p_tty->p_console->cursor);
			break;
		case RIGHT:
				p_tty->p_console->cursor+=8;
				SetCursor(p_tty->p_console->cursor);
			break;

		case F1:
				DisableInterruption();
				if(proc_table[6].p_flags==0)
					proc_table[6].p_flags=1;
				EnableInterruption();
				Speaker(1000,5);
				break;
		case F2:
				DisableInterruption();
				if(proc_table[6].p_flags==1)
					proc_table[6].p_flags=0;
				EnableInterruption();
				Speaker(1000,5);
				break;
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				SelectConsole(raw_code - F1);
			}
			break;
		case ENTER:
			PutKey(p_tty, '\n');
			break;
		case BACKSPACE:
			PutKey(p_tty, '\b');
			break;
		case ESC:
			__asm__ __volatile__("ud2");
			break;
		default:
			break;
		}
	}
}


void SelectConsole(int nr_console)
{
	if(nr_console < 0 || (nr_console >= NR_CONSOLES))
		return;
	nr_current_console = nr_console;
	//SetCursor(console_table[nr_console].cursor);
	//SetVideoStartAddr(console_table[nr_console].current_start_addr);
}

void ScrollScreen(CONSOLE * p_con, int direction, int line)
{
	if(direction == SCR_UP)
	{
		if(p_con->char_start_line<=p_con->char_current_line-24&&p_con->char_current_line>=25)
		{
			p_con->char_start_line +=line;
			SetVideoStartAddr(p_con,p_con->char_start_line);
		}
	}
	else
	{
		if(p_con->char_start_line>1)
		{
			p_con->char_start_line -=line;
			SetVideoStartAddr(p_con,p_con->char_start_line);
		}
	}
	SetCursor(p_con->cursor);
}

 void tty_write(TTY * p_tty, char *buf, int len)
{
	char *p = buf;
	int i = len;
	while(i)
	{
		OutChar(p_tty->p_console, *p++);
		i--;
	}
}
 int sys_write(char * buf, int len, PROCESS *p_proc)
{
	tty_write(&tty, buf, len);
	return 0;
}


/*======================================================================*
                              sys_printx
*======================================================================*/
int sys_printx(int _unused1, int _unused2, char* s, PROCESS*  p_proc)
{
	const char * p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	/**
	 * @note Code in both Ring 0 and Ring 1~3 may invoke printx().
	 * If this happens in Ring 0, no linear-physical address mapping
	 * is needed.
	 *
	 * @attention The value of `k_reenter' is tricky here. When
	 *   -# printx() is called in Ring 0
	 *      - k_reenter > 0. When code in Ring 0 calls printx(),
	 *        an `interrupt re-enter' will occur (printx() generates
	 *        a software interrupt). Thus `k_reenter' will be increased
	 *        by `kernel.asm::save' and be greater than 0.
	 *   -# printx() is called in Ring 1~3
	 *      - k_reenter == 0.
	 */
	if (k_reenter == 0)  /* printx() called in Ring<1~3> */
		p = va2la(proc2pid(p_proc), s);
	else if (k_reenter > 0) /* printx() called in Ring<0> */
		p = s;
	else	/* this should NOT happen */
		p = reenter_err;

	/**
	 * @note if assertion fails in any TASK, the system will be halted;
	 * if it fails in a USER PROC, it'll return like any normal syscall
	 * does.
	 */

	if ((*p == MAG_CH_PANIC) ||
	    (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
		DisableInterruption();
		char * v = (char*)V_MEM_BASE;
		const char * q = p + 1; /* +1: skip the magic char */

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
			//*v++ = *q++;
			//*v++ = RED_CHAR;
			if (!*q) {
				while (((int)v - V_MEM_BASE) % (640 * 16)) {
					/* *v++ = ' '; */
					//v++;
					//*v++ = GRAY_CHAR;
				}
				q = p + 1;
			}
		}

		__asm__ __volatile__("hlt");
	}
	while ((ch = *p++) != 0) {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue; /* skip the magic char */
		OutChar(tty.p_console, ch);
		
	}

	return 0;
}

