
#ifndef _ORANGES_TTY_H_
#define _ORANGES_TTY_H_


#define TTY_IN_BYTES	256	/* tty input queue size */

#define SCR_SIZE		(80 * 25)
#define SCR_WIDTH		80


/* CONSOLE */
typedef struct s_console
{
	int	window_size_limit;		/* 当前控制台占的显存大小 */
	int window_len;
	int window_wid;
	int	cursor;					/* 当前光标位置（像素） */
	
	char 	char_buf[20000];		/* TTY 字符缓冲区 */
	int  	char_current_pos;		/* 当前字符位置   */
	int	char_count;					/* 字符个数 */
	int	char_current_line;			/* 当前显示行 */
	int	char_start_line;			/* 起始行 */
	
	int	gralayer1;			//TTY图层一（背景）
	int	gralayer2;			//TTY图层二（文字）
	
}CONSOLE;


/* TTY */
typedef struct s_tty
{
	int	in_buf[TTY_IN_BYTES];	/* TTY 输入缓冲区 */
	int*	p_inbuf_head;		/* 指向缓冲区中下一个空闲位置 */
	int*	p_inbuf_tail;		/* 指向键盘任务应处理的键值 */
	int	inbuf_count;			/* 缓冲区中已经填充了多少 */
	
	
	
	struct s_console *	p_console;
}TTY;


#endif /* _ORANGES_TTY_H_ */
