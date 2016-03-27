
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            type.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_ORANGES_TYPE_H_
#define	_ORANGES_TYPE_H_

#include "const.h"

typedef	unsigned long long	u64;
typedef	unsigned int		u32;
typedef	unsigned short		u16;
typedef	unsigned char		u8;

typedef	void	(*int_handler)	();
typedef void	(*task_f)		();
typedef void	(*irq_handler)  (int irq);
typedef void     * system_call;

typedef char *	 va_list;

/**
 * MESSAGE mechanism is borrowed from MINIX
 */
struct mess1 {
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};
struct mess2 {
	void* m2p1;
	void* m2p2;
	void* m2p3;
	void* m2p4;
};
struct mess3 {
	int	m3i1;
	int	m3i2;
	int	m3i3;
	int	m3i4;
	u64	m3l1;
	u64	m3l2;
	void*	m3p1;
	void*	m3p2;
};
typedef struct {
	int source;
	int type;
	union {
		struct mess1 m1;
		struct mess2 m2; 
		struct mess3 m3;
	} u;
} MESSAGE;



typedef struct
{
	int year;
	char month;
	char day;
	char week;
	char hour;
	char minite;
	char second;
}Time;

typedef struct boot_params 
{
	int		mem_size;	/* memory size */
	unsigned char *	kernel_file;	/* addr of kernel file */
	unsigned char * video_linear_addr;
	int 		ScreenX;
	int		ScreenY;
	int		NoOfProcessors;
	int		Dirty;
}BootParams;

typedef struct
{
	char Red;
	char Green;
	char Blue;
	char Alpha;
}Pixel;

typedef struct
{
	int PosX,PosY;
	int Length,Width;
	char * LayerStartAddr;
	int Pid;
}GraphicsLayer;


typedef struct
{
	GraphicsLayer GraLayer[NumOfLayer];
	int TopLayer;
	int LayerToVRAM;
}GraphicsLayers;

#endif /* _ORANGES_TYPE_H_ */
