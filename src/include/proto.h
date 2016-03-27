
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar	2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


void	Out_Byte(u16 port, char value);
char    In_Byte(u16 port);
void	ReadPort(u16 port, void* buf, int n);
void	WritePort(u16 port, void* buf, int n);

void*	memcpy(void* p_dst, void* p_src, int size);
void	memset(void* p_dst, char ch, int size);
void	init_prot();
void	Init8259A();
void   Delay(int time);
void   ClockHandler();
void   spurious_irq(int irq);
void   Enable_IRQ(int irq);
void   Disable_IRQ(int irq);
void   SetIRQHandler(int irq,irq_handler handler);
void   SetIRQ();
int		sys_get_ticks();
void	sys_call();
int		GetTicks();
void   EnableInterruption();
void   DisableInterruption();
void   InitKeyboard();
void   KeyboardHandler();
void	HDHandler();
void	MouseHandler();
void	InitMouse();
void   SetLEDs();
void	FlashLEDs();
void   SelectConsole(int nr_console);
void   Write(char *buf, int len);
int		printf(const char *fmt,...);
int		vsprintf(char *buf, const char *fmt, va_list args);
int		sprintf(char *buf, const char *fmt, ...);
int		printx(char* str);
void	assertion_failure(char *exp, char *file, char *base_file, int line);
int		sendrec(int function, int src_dest, MESSAGE* p_msg);
void	panic(const char *fmt, ...);
int    sys_sendrec(int function, int src_dest, MESSAGE* m, PROCESS* p);
int    send_recv(int function, int src_dest, MESSAGE* msg);
int		ldt_seg_linear(PROCESS* p, int idx);
void	* va2la(int pid, void* va);
void	reset_msg(MESSAGE* p);
void	dump_proc(PROCESS* p);
void	inform_int(int task_nr);
int		GetTicks();
void	Schedule();
void	Speaker(u16 frequency,u16 duration);
void	GetCurrentTime(Time * time);
void	PrintSysTime(Time systime);
void 	init_descriptor(DESCRIPTOR *p_desc,u32 base,u32 limit,u16 attribute);
void	CPU_ID(int * eax,int * ebx,int * ecx,int * edx);
void 	InitGraph();
int 	AllocateGraph(int x,int y,int len,int wid);
void 	RefreshGraph(int x,int y,int len,int wid,int layer);
void	ReDrawGraph(int layer);
void 	FreeGraph(int layer);
void 	ZeroGraph(int layer);
void	TopLayer(int layer);
void 	SlideGraph(int x,int y,int layer);
void 	Point(int x,int y,int color,int layer);
void 	Rectangle(int x,int y,int len,int wid,int fill,int color,int layer);
void 	Char(char ch,int x,int y,int color,int layer);
void 	String(char * str,int x,int y,int color,int layer);
void 	Line(int x,int y,int color,int layer);
void 	Circle(int x,int y,int radius,int fill,int color,int layer);
void    InitTTY();
void 	SystimeToString(Time time,char * R_time,char * R_date,int methord);
#define	phys_copy	memcpy
#define	phys_set	memset

void restart();

