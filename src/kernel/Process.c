
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Process.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//?a??òa°üo??úproto.h???°·??òprotoà??3D???ò?2?ê?±e
#include "proto.h"
#include "global.h"
#include "musics.h"
#include "fs.h"
#include "pci.h"

extern PCIConfigurationSpace PCI[255];
extern PCI_Cnt;

void	TaskTTY();
void	task_hd();
void	task_fs();
void	TestA();
void	TestB();
void	TestC();
void	PrintTime();
int		sys_printx();
void 	task_sys();
void 	task_mm();
void	task_video();
void	Init();
void 	task_Display();
void 	get_fat32_params(BootSectorFAT32 * p);
void 	get_fat32_params(BootSectorFAT32 * p);
void 	get_FSINFO(FSINFO * p);
u32 	get_root_dir(BootSectorFAT32 tem,FileDirEntry * p);
u32 	get_next_cluster(BootSectorFAT32 tem, u32 current_cluster);
u32 	fread_s(BootSectorFAT32 BSF, FileDirEntry fde [ ], FileDescriptor * fp, u8 * buf, u32 size);
FileDescriptor 	fopen(BootSectorFAT32 tem, char * filepath);




PUBLIC	PROCESS			proc_table[NR_TASKS + NR_PROCS];
PUBLIC	char			task_stack[STACK_SIZE_TOTAL];
PUBLIC  TASK			task_table[NR_TASKS] = {
												{TaskTTY, STACK_SIZE_TTY, "TTY"},
												{task_sys, STACK_SIZE_SYST, "SystemTask"},
												{task_mm,STACK_SIZE_DEFAULT,"MM"},
												{task_Display,STACK_SIZE_DEFAULT,"Display"},
												{task_hd,STACK_SIZE_DEFAULT,"HardDisk"},
												{task_fs,0xB000,"FileSystem"}
												};

PUBLIC	TASK			user_proc_table[NR_PROCS] = {
													{Init, 0xB000, "INIT"},
													{TestA, STACK_SIZE_TESTA, "TestA"},
													{TestB, STACK_SIZE_TESTB, "TestB"},
													{TestC, STACK_SIZE_TESTC, "TestC"},
													};
													
PUBLIC	system_call sys_call_table[NR_SYS_CALL] = {sys_printx,sys_sendrec};   //之前写反导致消息无法传递

PUBLIC	irq_handler		irq_table[NR_IRQ];

BootParams BootParam={0,0,0,0,0};
/*======================================================================*
kernel_main
*======================================================================*/
PUBLIC int kernel_main()
{
	Init8259A();
	Init8253();
	SetIRQ();
	
	k_reenter = -1;
	ticks = 0;
	get_boot_params(&BootParam);
	GetCurrentTime(&systime);			//?áè?RTCê±??

	int i, j, eflags, prio;
        u8  rpl;
        u8  priv; 

	TASK * t;
	PROCESS * p = proc_table;

	char * stk = task_stack + STACK_SIZE_TOTAL + 0x4000;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p++,t++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) 
		{
			p->p_flags = FREE_SLOT;
			p->ldt_sel = SELECTOR_LDT_FIRST + (i << 3);		//空槽也要给LDT赋值否则调度任务会#GP异常
			continue;
		}

	        if (i < NR_TASKS) {     
                        t	= task_table + i;
                        priv	= PRIVILEGE_TASK;
                        rpl     = RPL_TASK;
                        eflags  = 0x1202;
			prio    = 1;
                }
                else {                  
                        t	= user_proc_table + (i - NR_TASKS);
                        priv	= PRIVILEGE_USER;
                        rpl     = RPL_USER;
                        eflags  = 0x202;	
			prio    = 1;
                }

		strcpy(p->name, t->name);	
		p->p_parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0) {
			p->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			
			p->ldts[INDEX_LDT_C].attr1  = DA_C   | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else {		
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_descriptor(&p->ldts[INDEX_LDT_C],
				  0, 
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_descriptor(&p->ldts[INDEX_LDT_RW],
				  0, 
				  (k_base + k_limit) >> LIMIT_4K_SHIFT,
				  DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 |	SA_TIL | rpl;
		p->regs.ds =
			p->regs.es =
			p->regs.fs =
			p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip	= (u32)t->initial_eip;
		p->regs.esp	= (u32)stk;
		p->regs.eflags	= eflags;

		p->ticks = p->priority = prio;

		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;
		p->ldt_sel = SELECTOR_LDT_FIRST + (i << 3);

		p->nr_tty = 0;

		stk -= t->stacksize;
	}
	p_proc_ready	= proc_table;
	PCI_Scan(&PCI);
	InitGraphic();
	InitKeyboard();
	InitTTY();
	
	SetIRQHandler(0,ClockHandler); 			//打开8253中断

	DrawRect(0,0,BootParam.ScreenX,BootParam.ScreenY,1,0xff008f8f,0);
	RefreshArea(0,0,BootParam.ScreenX,BootParam.ScreenY);
	printf("Number Of Processors:%d\n",BootParam.NoOfProcessors);
	
	restart();
	while(1);
}
/*****************************************************************************
*Ring 3
*								Init()
*****************************************************************************/
void Init()
{
	int eax = 0x80000002;
	int ebx = 0;
	int ecx = 0;
	int edx = 0;
	int CPU_char[12];
	int i;
	int numofdir;
	char showbuf[512] = {"Reading Failed!"};
	int *p = BootParam.video_linear_addr;

	
	
	BootSectorFAT32 BSF;
	FileDirEntry FDE[50];
	FSINFO fso;
	memset(FDE,sizeof(FDE),0);
	get_fat32_params(&BSF);
	get_FSINFO(&fso);
	numofdir = get_root_dir(BSF,FDE);
	FileDescriptor fp = fopen(BSF, "TEST    TXT");
	//fp.IndexOfFDE = 2;
	//fp.FilePointer = 0;
	//printf("fp:%d",fp.IndexOfFDE);
	fread_s(BSF, FDE, &fp, showbuf, 512);
	
	//int fd_stdin = open("/dev_tty0",O_RDWR);
	//int fd_stdout = open("/dev_tty0",O_RDWR);
	printf("Init...\n");
	for(i=0;i<3;i++)
	{
		eax = 0x80000002 + i;
		CPU_ID(&eax,&ebx,&ecx,&edx);
		CPU_char[0+i*4] = eax;
		CPU_char[1+i*4] = ebx;
		CPU_char[2+i*4] = ecx;
		CPU_char[3+i*4] = edx;
	}
	printf("%s\n",CPU_char);
	eax = 0x80000008;
	CPU_ID(&eax,&ebx,&ecx,&edx);
	printf("Linear Addr:%dbits,Physics Addr:%d\n",eax>>8,eax&0x000000ff);
	//printf("Bytes Per Sector:%d\n",BSF.BytesPerSector);
	//printf("Sectors Per Cluster:%d\n",BSF.SectorsPerCluster);
	//printf("Hidden Sectors:%d\n",BSF.HiddenSectors);
	//printf("Reserved Sectors:%d\n",BSF.ReservedSectors);
	//printf("Volume Size:%d Sectors.\n",BSF.Sectors);
	//printf("Sectors Per FAT:%d\n",BSF.fat32sec.SectorsPerFAT);
	//printf("1st Root Dir Sector:%d\n",BSF.fat32sec.FirstRootDirSector);
	//printf("Free Clusters:%d\n",fso.FreeClusters);
	//printf("Next Available Cluster:%d\n",fso.NextAvailableClusters);
	//printf("File Dir Entry found:%d\n",numofdir);
	//printf("next cluster no:%d\n",get_next_cluster(BSF, 3));

	printf("Root Directory Contents:\n");

	printf("File Name  File Size  Created Date  Created Time  Last Visited Date  Modified Date  Modified Time\n");
	for(i=0;i<numofdir;i++)
	{
		//printf("%d.%s start cluster:%d size:%d\n",i,FDE[i].FileName,FDE[i].StartCluster,FDE[i].SizeOfFile);
		printf("%s  %d  ",FDE[i].FileName,FDE[i].SizeOfFile);
		printf("%d.%d.%d  %d:%d:%d  ",
								FDE[i].CreatedDate_Year,FDE[i].CreatedDate_Month,FDE[i].CreatedDate_Day,
								FDE[i].CreatedTime_Hour,FDE[i].CreatedTime_Minite,FDE[i].CreatedTime_Second);
		printf("%d.%d.%d  ",
								FDE[i].VisitedDate_Year,FDE[i].VisitedDate_Month,FDE[i].VisitedDate_Day
								);
		printf("%d.%d.%d  %d:%d:%d\n",
								FDE[i].ModifiedDate_Year,FDE[i].ModifiedDate_Month,FDE[i].ModifiedDate_Day,
								FDE[i].ModifiedTime_Hour,FDE[i].ModifiedTime_Minite,FDE[i].ModifiedTime_Second);
		
	}
	printf("File Contents of test.txt:\n");
	printf("%s\n",showbuf);
	
	printf("VideoPhyAddr:%x\n",BootParam.video_linear_addr);
	printf("Number Of Processors:%d\n",BootParam.NoOfProcessors);
	printf("PCI Scanning:Device found:%d\n",PCI_Cnt);
	for(i = 0; i<PCI_Cnt; i++)
	{
		printf("VendorID:%x,DeviceID:%x\n",PCI[i].VendorID,PCI[i].DeviceID);
		//printf("BAR0:%x,BAR1:%x,BAR2:%x,BAR3:%x,BAR4:%x,BAR5:%x\n",PCI[i].BAR0,PCI[i].BAR1,PCI[i].BAR2,PCI[i].BAR3,PCI[i].BAR4,PCI[i].BAR5);
	}
	int pid = fork();
	if(pid != 0)
	{
		printf("parent process running,child pid:%d\n", pid);
		spin("parent");
	}
	else
	{
		printf("child process running,pid:%d\n", pid);
		spin("child");
	}
	while(1);
}

void	TestA()			
{
	int i = 0;
	int j = 0;
	int k = 0;
	int m = 0;
	int n = 0;
	int status = 0;
	Pixel color1={0,0,0,0};
	int color=0xffff00f0;
	char * color_c = &color; 
	int layer1 = AllocateGraph(580,300,200,216);
	Rectangle(0,0,200,16,1,0xff44433f,layer1);
	String("Window1",0,0,0xffffffff,layer1);
	Rectangle(0,16,200,200,1,0xffffffff,layer1);	//之前为0x00ffffff被认为透明而被刷新
	RefreshArea(0,0,200,216,layer1);
	int layer2 = AllocateGraph(580,316,200,200);
	while(1)
	{
		for(k=0; k<51; k++)
		{
			switch(j)
			{
				case 0:
					color1.Red = 255;
					color1.Green+=5;
					break;
				case 1:
					color1.Red-=5;
					break;
				case 2:
					color1.Blue+=5;
					break;
				case 3:
					color1.Green-=5;
					break;
				case 4:
					color1.Red+=5;
					break;
				case 5:
					color1.Blue-=5;
					break;
			}
			*color_c = color1.Blue;
			*(color_c+1) = color1.Green;
			*(color_c+2) = color1.Red;
			//*(color_c+3) = color1.Alpha;
			Circle(100,100,100,0,color,layer2);
			//DrawLine(200,100,0,0,color,layer2);
			
			DrawLine(0,0,200,200,color,layer2);
			DrawLine(100,0,100,200,color,layer2);
			DrawLine(0,100,200,100,color,layer2);
			DrawLine(0,200,200,0,color,layer2);
			
			if(m==0 && n==0)
				status = 0;
			if(m==199 && n<199)
				status = 1;
			if(m==199 && n==199)
				status = 2;
			if(m==0 && n==199)
				status = 3;
			switch(status)
			{
				case 0:
				DrawLine(m++,n,100,100,color,layer2);
				break;
				case 1:
				DrawLine(m,n++,100,100,color,layer2);
				break;
				case 2:
				DrawLine(m--,n,100,100,color,layer2);
				break;
				case 3:
				DrawLine(m,n--,100,100,color,layer2);
				break;
				
			}
			
			RefreshArea(0,0,200,200,layer2);	
			//if(m>=100)
				//m=0;
		}
		if(j++ == 6)
				j = 0;
		
	}
}

void	TestB()
{
	int i = 0;
	int temp;
	int layer_m = AllocateGraph(500,50,200,48);
	int layer_n = AllocateGraph(500,82,200,16);
	Rectangle(0,0,200,16,1,0xff44433f,layer_m);
	String("Music",0,0,0xffffffff,layer_m);
	Rectangle(0,16,200,16,1,0xffff00ff,layer_m);
	String("Now Playing:",0,16,0xffffffff,layer_m);
	Rectangle(0,32,200,16,1,0xff00ffff,layer_m);
	RefreshArea(0,0,200,48,layer_m);
	while(1)
	{
		i = 0;
		ZeroLayer(layer_n);
		String("American Petro",0,0,0xffffffff,layer_n);
		RefreshArea(0,0,200,16,layer_n);
		while(1)
		{
			temp = AmericanPetro.m_tone[i];
			if(AmericanPetro.duration[i]==0)
				break;
			if(temp<0)
				temp = -temp+14-1;
			else
				temp = temp+14-1;
			//Speaker(tone[temp],AmericanPetro.duration[i]*125);
			i++;
		}
		Delay(1000);
		i = 0;
		ZeroLayer(layer_n);
		String("Bad Apple",0,0,0xffffffff,layer_n);
		RefreshArea(0,0,200,16,layer_n);
		while(1)
		{
			temp = BadApple.m_tone[i];
			if(BadApple.duration[i]==0)
				break;
			if(temp<0)
				temp = -temp+14-1;
			else
				temp = temp+14-1;
			//Speaker(tone[temp],BadApple.duration[i]*200);
			i++;
		}
		
		Delay(1000);
	}
}

void	TestC()
{
	char time[15];
	char date[35];
	char cticks[35];
	int layer_t1 = AllocateGraph(500,600,200,64);
	Rectangle(0,0,200,16,1,0xff44433f,layer_t1);
	String("Date&Time",0,0,0xffffffff,layer_t1);
	Rectangle(0,16,200,48,1,0xffffffff,layer_t1);
	RefreshArea(0,0,200,64,layer_t1);
	int layer_t2 = AllocateGraph(500,616,200,48);
	while(1)
	{
		ZeroLayer(layer_t2);
		SystimeToString(systime,time,date,1);
		sprintf(cticks,"8253 Ticks:%d",GetTicks());
		String(date,0,0,0xffff00ff,layer_t2);
		String(time,0,16,0xff00ff00,layer_t2);
		String(cticks,0,32,0xffff0000,layer_t2);
		RefreshArea(0,0,200,48,layer_t2);
	}	
}

PUBLIC int GetTicks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}
/*****************************************************************************
*				  ldt_seg_linear
*****************************************************************************/
/**
* <Ring 0~1> Calculate the linear address of a certain segment of a given
* proc.
* 
* @param p   Whose (the proc ptr).
* @param idx Which (one proc has more than one segments).
* 
* @return  The required linear address.
*****************************************************************************/
PUBLIC int ldt_seg_linear(PROCESS* p, int idx)
{
	DESCRIPTOR * d = &p->ldts[idx];

	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}
/*****************************************************************************
*				  va2la
*****************************************************************************/
/**
* <Ring 0~1> Virtual addr --> Linear addr.
* 
* @param pid  PID of the proc whose address is to be calculated.
* @param va   Virtual address.
* 
* @return The linear address for the given virtual address.
*****************************************************************************/
PUBLIC void* va2la(int pid, void* va)
{
	struct proc* p = &proc_table[pid];

	u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if (pid < NR_TASKS + NR_PROCS) {
		assert(la == (u32)va);
	}

	return (void*)la;
}



/*****************************************************************************
*                                schedule
*****************************************************************************/
/**
* <Ring 0> Choose one proc to run.
* Process scheduling algorithm
* Called by ClockHandler()
*****************************************************************************/
PUBLIC void Schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) 
	{
		for (p = &FIRST_PROC; p <= &LAST_PROC; p++) 
		{
			if(p->p_flags == 0)
			{
				if (p->ticks > greatest_ticks) 			
				{
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}

		if (!greatest_ticks) 				//如果所有进程的时间片都用完，则利用优先级的值重新赋值给ticks
		{
			for (p = &FIRST_PROC; p <= &LAST_PROC; p++) 
			{
				if(p->p_flags == 0)
					p->ticks = p->priority;
			}
		}
	}
}

PUBLIC void task_sys()
{
	MESSAGE msg;
	u8 BeepGate;
	u16 BeepFrequency;
	while(1)
	{
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		switch (msg.type)
		{
		case GET_TICKS:
			msg.RETVAL = ticks;
			send_recv(SEND, src, &msg);
			break;
		case BEEP_ON:
			BeepGate = In_Byte(0x61) | 0x03;   //61h???úD0 D1?a??éù?÷;
			BeepFrequency = 0x123280/msg.u.m1.m1i1;
			Out_Byte(0x43,0xB6);
			Out_Byte(0x42,BeepFrequency);
			Out_Byte(0x42,BeepFrequency>>8);
			Out_Byte(0x61,BeepGate);
			send_recv(SEND, src, &msg);
			break;
		case BEEP_OFF:
			BeepGate = In_Byte(0x61) & 0xFC;	  //1?±???éù?÷
			Out_Byte(0x61,BeepGate);
			send_recv(SEND, src, &msg);
			break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}

/*****************************************************************************
*                                panic
*****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printf("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}
