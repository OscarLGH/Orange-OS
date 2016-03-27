;******************************************************************************
; Loader.asm
; Oscar
; 2013.5
; 此Loader目前只能加载小于1M的内核
;******************************************************************************
org  0100h

	jmp	LABEL_START		; Start

; 下面是 FAT12 磁盘的头, 之所以包含它是因为下面用到了磁盘的一些信息
%include	"Fat32BPB.inc"
%include	"load.inc"
%include	"pm.inc"

; GDT
;                            段基址     段界限, 属性
LABEL_GDT:			Descriptor 0,            0, 0              ; 空描述符
LABEL_DESC_FLAT_C:  Descriptor 0,      0fffffh, DA_CR|DA_32|DA_LIMIT_4K ;0-4G
LABEL_DESC_FLAT_RW: Descriptor 0,      0fffffh, DA_DRW|DA_32|DA_LIMIT_4K;0-4G
LABEL_DESC_VIDEO:   Descriptor 0, 	   0fffffh, DA_DRW|DA_DPL3|DA_LIMIT_4K ; 显存首地址

GdtLen		equ	$ - LABEL_GDT
GdtPtr		dw	GdtLen - 1				; 段界限
GdtAdd		dd	BaseOfLoaderPhyAddr + LABEL_GDT		; 基地址

; GDT 选择子
SelectorFlatC		equ	LABEL_DESC_FLAT_C	- LABEL_GDT
SelectorFlatRW		equ	LABEL_DESC_FLAT_RW	- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT + SA_RPL3

BaseOfStack	equ	0100h


LABEL_START:					; <--- 从这里开始 *************

	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack

	mov	dh, 0					; "Loading  "
	call	DispStrRealMode		; 显示字符串

	; 得到内存数
	mov	ebx, 0					; ebx = 后续值, 开始时需为 0
	mov	di, _MemChkBuf			; es:di 指向一个地址范围描述符结构(ARDS)
.MemChkLoop:
	mov	eax, 0E820h				; eax = 0000E820h
	mov	ecx, 20					; ecx = 地址范围描述符结构的大小
	mov	edx, 0534D4150h			; edx = 'SMAP'
	int	15h						; int 15h
	jc	.MemChkFail
	add	di, 20
	inc	dword [_dwMCRNumber]	; dwMCRNumber = ARDS 的个数
	cmp	ebx, 0
	jne	.MemChkLoop
	jmp	.MemChkOK
.MemChkFail:
	mov	dword [_dwMCRNumber], 0
.MemChkOK:

	; 下面在 A 盘的根目录寻找 KERNEL.BIN
	xor	ah, ah				; ┓
	mov	dl, [BIOS_drive]	; ┣ 硬盘复位
	int	13h					; ┛

	; 下面在 C 盘的根目录寻找 KERNEL.BIN
	mov	word [wSectorNo], SectorNoOfRootDirectory
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	; ┓
	jz	LABEL_NO_LOADERBIN				; ┣ 判断根目录区是不是已经读完
	dec	word [wRootDirSizeForLoop]		; ┛ 如果读完表示没有找到 LOADER.BIN
	mov	ax, BaseOfKernelFile
	mov	es, ax					; es <- BaseOfKernelFile
	mov	bx, OffsetOfKernelFile	; bx <- OffsetOfKernelFile
	mov	ax, [wSectorNo]			; ax <- Root Directory 中的某 Sector 号
	mov [BlockNum_L32],	ax
	mov [BufAddr_H16],es
	mov [BufAddr_L16],bx
	mov word[BlockCount],	1

	call	ReadSector

	mov	si, KernelFileName		; ds:si -> "KERNEL  BIN"
	mov	di, OffsetOfKernelFile	; es:di -> BaseOfLoader:0100 = BaseOfLoader*10h+100
	cld
	mov	dx, 80h								; 原来这里是针对一个扇区的，本程序应该是一个簇
LABEL_SEARCH_FOR_LOADERBIN:
	cmp	dx, 0								; ┓循环次数控制
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR	; ┣如果已经读完了一个 Sector,
	dec	dx									; ┛就跳到下一个 Sector
	mov	cx, 11
LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND		; 如果比较了 11 个字符都相等, 表示找到
	dec	cx
	lodsb							; ds:si -> al
	cmp	al, byte [es:di]
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT				; 只要发现不一样的字符就表明本 DirectoryEntry 不是
; 我们要找的 KERNEL.BIN
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME			;	继续循环

LABEL_DIFFERENT:
	and	di, 0FFE0h					; else ┓	di &= E0 为了让它指向本条目开头
	add	di, 20h						;      ┃
	mov	si, KernelFileName			;      ┣ di += 20h  下一个目录条目
	jmp	LABEL_SEARCH_FOR_LOADERBIN	;      ┛

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov	dh, 2				; "No KERNEL."
	call	DispStrRealMode	; 显示字符串
%ifdef	_BOOT_DEBUG_
	mov	ax, 4c00h			; ┓
	int	21h					; ┛没有找到 LOADER.BIN, 回到 DOS
%else
	jmp	$					; 没有找到 LOADER.BIN, 死循环在这里
%endif

LABEL_FILENAME_FOUND:				; 找到 LOADER.BIN (rootentry) 后便来到这里继续
	mov	ax, RootDirSectors
	and	di, 0FFE0h					; di -> 当前条目的开始/指向内存开始

	push	eax
	mov	eax, [es : di + 01Ch]		; ┓
	mov	dword [dwKernelSize], eax	; ┛保存 KERNEL.BIN 文件大小
	pop	eax

	add	di, 01Ah					; di -> 首簇号
	xor ecx, ecx
	mov	cx, word [es:di]
	push	ecx						; 保存此簇在 FAT 中的序号(只保存了低16位，做文件系统的时候注意保存高16位)
	mov eax,	ecx								
	mul byte[Sectors_per_cluster]	; 文件第一簇所在扇区 = DeltaSectorNo + 序号*8
	add	eax, DeltaSectorNo			; 这句完成时 cx 里面变成 LOADER.BIN 的起始扇区号 (从 0 开始数的序号)
	mov ecx, eax
	mov	ax, BaseOfKernelFile
	mov	es, ax						; es <- BaseOfLoader
	mov	bx, OffsetOfKernelFile			; bx <- OffsetOfLoader	于是, es:bx = BaseOfLoader:OffsetOfLoader = BaseOfLoader * 10h + OffsetOfLoader			
	mov eax, ecx						; ax <- Sector 号
									; 以上代码只在第一次读取目录时调用，读到目录项后便一直在下面的模块中循环直到结束

LABEL_GOON_LOADING_FILE:
	push	eax			; ┓
	push	ebx			; ┃
	mov	ah, 0Eh			; ┃ 每读一个簇(8个扇区)区就在 "Loading  " 后面打一个点, 形成这样的效果:
	mov	al, '.'			; ┃
	mov	bl, 0Fh			; ┃ Loading ......
	int	10h				; ┃
	pop	ebx				; ┃
	pop	eax				; ┛


	mov	[BlockNum_L32], eax
	mov [BufAddr_H16],	es
	mov	[BufAddr_L16],	bx
	mov word[BlockCount],	32	; 取一个簇

	call	ReadSector			; 根据计算出的FAT项取数据
	
	
	pop	eax						; 取出此 Sector 在 FAT 中的序号<--|
																 ;|
	call	GetFATEntry											 ;|
																 ;|
	cmp	eax, 0FFFFFFFh											 ;|
	jz	LABEL_FILE_LOADED										 ;|
	push	eax					;   保存 Sector 在 FAT 中的序号---|
	
	mul byte[Sectors_per_cluster]	;   一个簇8个扇区
	
	add	eax, DeltaSectorNo
	add	bx, BytesPerCluster		; 内存增量，存放下一个簇 最大值65535，内核大于64K时es要+1000
	jc	.1						; 如果 bx 重新变成 0，说明内核大于 64K
	jmp	.2
.1:
	push	eax			; es += 0x1000  ← es 指向下一个段
	mov	ax, es
	add	ax, 1000h		; 要修改Loader加载的位置不然此处会覆盖Loader的内存区域
	mov	es, ax
	pop	eax
.2:
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:

	mov	dh, 1				; "Ready."
	call	DispStrRealMode	; 显示字符串
	
	; 显示读到扇区的内容
	;mov	ax, BaseOfKernelFile
	;mov es,ax			
	;mov bp,	8192
	;mov	cx, 256		; CX = 串长度
	;mov	ax, 01301h		; AH = 13,  AL = 01h
	;mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	;mov	dx, 1400h
	;int	10h				; int 10h
	;jmp $
	
	; 清屏：
	mov ax,0600h
	mov cx,0000h
	mov dx,184fh
	mov bh,07h
	int 10h
	
	; 打印分辨率选择信息
	mov ax,ds
	mov es,ax
	mov bp,ResolutionMessage
	mov ax,01301h
	mov bx, 0007h
	mov cx,49
	mov dh,0
	mov dl,0
	int 10h
	; 显示分辨率选项
	mov ax,ds
	mov es,ax
	mov bp,Resolution1
	mov ax,01301h
	mov bx, 0007h
	mov cx,25
	mov dh,1
	mov dl,0
.rel:
	int 10h
	inc dh
	add bp,25
	dec byte[NumOfResolutions]
	cmp byte[NumOfResolutions],0
	jnz .rel
	
	
	; 分辨率选择
	mov ah,0
	int 16h					; 获取键盘输入，存放ascii码于AL中
	
	cmp al,'1'
	jnz re.1
	mov bx,0x141
	jmp re.f
re.1:	
	cmp al,'2'
	jnz re.2
	mov bx,0x118
	jmp re.f
re.2:	
	cmp al,'3'
	jnz re.3
	mov bx,0x14c
	jmp re.f
re.3:	
	cmp al,'4'
	jnz re.4
	mov bx,0x11B
	jmp re.f
re.4:	
	cmp al,'5'
	jnz re.5
	mov bx,0x167
	jmp re.f
re.5:	
	cmp al,'6'
	jnz re.d
	mov bx,0x14d
	jmp re.f
re.d:	
	mov bx,0x117
re.f:
	; 设置显示模式,BX中为vesa模式号
	mov ax,04f02h
	
	or  bx,0x4000
	int 10h
	
	; 获取此模式的显存线性地址
	mov ax, cs
	mov es,	ax
	mov di, _VESA_OFF
	
	and bx,0xbfff
	mov ax, 0x4f01
	mov cx, bx
	int 10h

	mov eax, [es:di + 40]

	cmp eax,0
	je  NoLinearAdd
	jmp goon
NoLinearAdd:
	mov eax, 0A0000h	
	; 存入视频信息
goon:	
	mov [_VideoLinearAdd], eax
	
	mov ax, [es:di + 18]
	mov [_ScreenX], ax
	mov ax, [es:di + 20]
	mov [_ScreenY], ax
	

	; 修改显存段描述符
	mov word[LABEL_DESC_VIDEO + 2], ax
	shr	eax,16
	mov byte[LABEL_DESC_VIDEO + 4], al
	mov byte[LABEL_DESC_VIDEO + 7], ah
	
;CPU初始化代码

;以下是所有CPU公共运行代码
Startup_Begin:
;此处CS应该被IPI机制自动设置为正确的地址，无需重复设置
;但DS的地址由于全局变量的存在应该设置为全局地址
;之前没有设置DS导致AP写入到错误的地址
	mov	ax, 9000H
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
;为每个CPU设置不同栈地址
	mov ax, word[_NoOfProcessors]
	mov	sp, BaseOfStack
	mov bx, 1000h
	mul bx
	add sp, ax
;上锁
Processor_Lock:
	lock bts DWORD[_SpinLock],0
	jc Processor_Lock;
	lock inc byte[_NoOfProcessors]
	
	; 加载 GDTR
	lgdt	[GdtPtr]
	
	; 关中断
	cli

	; 打开地址线A20
	in	al, 92h
	or	al, 00000010b
	out	92h, al

	; 准备切换到保护模式
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax
	
	; 进入保护模式
	jmp	dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)
	
	hlt
Startup_End:

;============================================================================
;变量
;----------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors	; Root Directory 占用的扇区数
wSectorNo		dw	0		; 要读取的扇区号

dwKernelSize		dd	0		; KERNEL.BIN 文件大小

;============================================================================
;磁盘数据包DAP	调用：AH=42H DL=驱动器号 DS:SI=数据包地址 INT 13H
;----------------------------------------------------------------------------
DiskAddressPacket:
PacketSize  	db	10h						; 大小：一般为10H，也有其他格式
Reserved		db	0						; 保留位，0
BlockCount		dw	32						; 要读取的扇区数(本程序中应该为一个簇)		根据不同介质修改！
BufAddr_L16		dw	0						; 要读入的内存地址低16位(段内偏移地址)
BufAddr_H16		dw  0						; 要读入的内存地址高16位(段基地址)
BlockNum_L32	dd	0						; 要读取的绝对扇区号低32位
BlockNum_H32	dd  0						; 要读取的绝对扇区号高32位

;============================================================================
;字符串
;----------------------------------------------------------------------------
KernelFileName		db	"KERNEL  BIN", 0	; KERNEL.BIN 之文件名
; 为简化代码, 下面每个字符串的长度均为 MessageLength
MessageLength		equ	9
LoadMessage:		db	"Loading  "
Message1			db	"Ready.   "
Message2			db	"No KERNEL"

ResolutionMessage 	db "Choose a resolution that mostly fits your screen:"
Resolution1			db "1. 1024x768  AMD         "
Resolution2			db "2. 1024x768  Intel/Nvidia"
Resolution3			db "3. 1366x768  AMD         "
Resolution4			db "4. 1366x768  Intel/Nvidia"
Resolution5			db "5. 1920x1080 AMD         "
Resolution6			db "6. 1920x1080 Intel/Nvidia"
NumOfResolutions    db 6
;============================================================================

;----------------------------------------------------------------------------
; 函数名: DispStrRealMode
;----------------------------------------------------------------------------
; 运行环境:
;	实模式（保护模式下显示字符串由函数 DispStr 完成）
; 作用:
;	显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)
DispStrRealMode:
	mov	ax, MessageLength
	mul	dh
	add	ax, LoadMessage
	mov	bp, ax				; ┓
	mov	ax, ds				; ┣ ES:BP = 串地址
	mov	es, ax				; ┛
	mov	cx, MessageLength	; CX = 串长度
	mov	ax, 01301h			; AH = 13,  AL = 01h
	mov	bx, 0007h			; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	add	dh, 3				; 从第 3 行往下显示
	int	10h					; int 10h
	ret

;----------------------------------------------------------------------------
; 函数名: ReadSector
;----------------------------------------------------------------------------
; 作用:
;	从第 ax 个 Sector 开始, 将 cl 个 Sector 读入 es:bx 中
ReadSector:
	mov si,	DiskAddressPacket
	mov ah, 42h
	mov dl, 80h
.GoOnReading:
	int	13h
	jc	.GoOnReading		; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止
	ret

;----------------------------------------------------------------------------
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;	根据ax的值（在FAT中的偏移量）找到FAT项，取出FAT项的值放在eax中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
	push	es
	push	bx
	push	eax
	mov	ax, BaseOfKernelFile		; ┓
	sub	ax, 0100h					; ┣ 在 BaseOfKernelFile 后面留出 4K 空间用于存放 FAT
	mov	es, ax						; ┛
	pop	eax

	mov dl,	4						; 一个FAT项占四个字节
	mul dl							; 求出FAT项的字节偏移量（相对FAT1目录）

	mov edx,eax						; 将eax高位给dx用于下面的除法
	shr	edx,16						; 现在 ax 中是 FATEntry 在 FAT 中的偏移量. 下面来计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	div	word[Bytes_per_sector]		; dx:ax / Bytes_per_sector  ==>	ax <- 商   (FATEntry 所在的扇区相对于 FAT 来说的扇区号)
									; dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0						; bx <- 0	于是, es:bx = (BaseOfLoader - 100):00 = (BaseOfLoader - 100) * 10h					
	add	eax, SectorNoOfFAT1			; 此句执行之后的 ax 就是 FATEntry 所在的扇区号

	mov	[BlockNum_L32], eax
	mov [BufAddr_H16],	es
	mov	[BufAddr_L16],	bx
	mov word[BlockCount], 1
	call	ReadSector				; 取出FAT内容（文件所在的下一簇）
	
	pop	dx
	mov ax,	dx
	add	bx, ax
	mov	eax, [es:bx]
	
LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret
;----------------------------------------------------------------------------
; 从此以后的代码在保护模式下执行 ----------------------------------------------------
; 32 位代码段. 由实模式跳入 ---------------------------------------------------------
[SECTION .s32]

ALIGN	32

[BITS	32]

LABEL_PM_START:
	
	mov	ax, SelectorVideo
	mov	gs, ax

	mov	ax, SelectorFlatRW
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov eax, Dword[NoOfProcessors]
	mov ebx, 1024
	mul ebx
	mov	esp, TopOfStack
	add esp, eax
	
	mov ecx,1Bh			;IA32_APIC_BASE
	rdmsr
	bt eax,8
	jnc AP_Processor
	
;BSP代码：
	;复制初始化代码到20000h处供AP读取
	mov esi, Startup_Begin + BaseOfLoaderPhyAddr
	mov edi, 20000H
	mov ecx, Startup_End - Startup_Begin;
	rep movsb
	
	;BSP发送IPI-SIPI-SIPI序列
	mov DWORD [0FEE00000H + 300H],000c4500H
	mov ecx,0FFFFFFFFH
	rep nop
	mov ecx,0FFFFFFFFH
	rep nop
	mov ecx,0FFFFFFFFH
	rep nop
	mov ecx,0FFFFFFFFH
	rep nop
	mov DWORD [0FEE00000H + 300H],000c4620H
	mov ecx,0FFFFFFFFH
	rep nop
	mov ecx,0FFFFFFFFH
	rep nop
	mov DWORD [0FEE00000H + 300H],000c4620H
	mov ecx,0FFFFFFFFH
	rep nop
	mov ecx,0FFFFFFFFH
	rep nop
	jmp BSP_Processor
	
AP_Processor:
	lock btr DWORD[SpinLock],0
	hlt
BSP_Processor:
	lock btr DWORD[SpinLock],0

	;push	szMemChkTitle
	;call	DispStr
	;add	esp, 4

	call	DispMemInfo

	call	SetupPaging

	call	InitKernel
	
	;***************************************************************
	; 记录内核大小及内存地址
	;***************************************************************
	mov dword 	[BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC
	mov eax,	[dwMemSize]
	mov [BOOT_PARAM_ADDR + 4],eax
	mov eax,	BaseOfKernelFile
	shl	eax,	4
	add eax,	OffsetOfKernelFile
	mov [BOOT_PARAM_ADDR + 8], eax	; phy-addr of kernel
	mov eax,[VideoLinearAdd]
	mov [BOOT_PARAM_ADDR + 12],eax
	xor eax,eax
	mov ax,[ScreenX]
	mov [BOOT_PARAM_ADDR + 16],ax
	mov ax,[ScreenY]
	mov [BOOT_PARAM_ADDR + 20],ax
	mov ax,[NoOfProcessors]
	mov [BOOT_PARAM_ADDR + 24],ax
	
	;***************************************************************
	jmp	SelectorFlatC:KernelEntryPointPhyAddr	; 正式进入内核 *
	;***************************************************************
	jmp $
	; 内存看上去是这样的：
	;              ┃                                    ┃
	;              ┃                 .                  ┃
	;              ┃                 .                  ┃
	;              ┃                 .                  ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■Page  Tables■■■■■■┃
	;              ┃■■■■■(大小由LOADER决定)■■■■┃
	;    00101000h ┃■■■■■■■■■■■■■■■■■■┃ PageTblBase
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;    00100000h ┃■■■■Page Directory Table■■■■┃ PageDirBase  <- 1M
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       F0000h ┃□□□□□□□System ROM□□□□□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       E0000h ┃□□□□Expansion of system ROM □□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       C0000h ┃□□□Reserved for ROM expansion□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃ B8000h ← gs
	;       A0000h ┃□□□Display adapter reserved□□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       9FC00h ┃□□extended BIOS data area (EBDA)□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;       80000h ┃■■■■■■■KERNEL.BIN■■■■■■┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;       30000h ┃■■■■■■■■KERNEL■■■■■■■┃ 30400h ← KERNEL 入口 (KernelEntryPointPhyAddr)
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃                                    ┃
	;        7E00h ┃              F  R  E  E            ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■					 ┃
	;        7C00h ┃■■■■■■BOOT  SECTOR(Overwrite)■■■■■■ ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃                                    ┃
	;         500h ┃              LOADER.bin            ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□					 ┃	
	;         400h ┃□□□□ROM BIOS parameter area □□		 ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇┃
	;           0h ┃◇◇◇◇◇◇Int  Vectors◇◇◇◇◇◇┃
	;              ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
	;
	;
	;		┏━━━┓		┏━━━┓
	;		┃■■■┃ 我们使用 	┃□□□┃ 不能使用的内存
	;		┗━━━┛		┗━━━┛
	;		┏━━━┓		┏━━━┓
	;		┃      ┃ 未使用空间	┃◇◇◇┃ 可以覆盖的内存
	;		┗━━━┛		┗━━━┛
	;
	; 注：KERNEL 的位置实际上是很灵活的，可以通过同时改变 LOAD.INC 中的
	;     KernelEntryPointPhyAddr 和 MAKEFILE 中参数 -Ttext 的值来改变。
	;     比如把 KernelEntryPointPhyAddr 和 -Ttext 的值都改为 0x400400，
	;     则 KERNEL 就会被加载到内存 0x400000(4M) 处，入口在 0x400400。
	;

%include	"lib.inc"


; 显示内存信息 --------------------------------------------------------------
DispMemInfo:
	push	esi
	push	edi
	push	ecx

	mov	esi, MemChkBuf
	mov	ecx, [dwMCRNumber] 	;for(int i=0;i<[MCRNumber];i++)//每次得到一个ARDS
.loop:					    ;{
	mov	edx, 5				;  for(int j=0;j<5;j++)//每次得到一个ARDS中的成员
	mov	edi, ARDStruct		;  {//依次显示:BaseAddrLow,BaseAddrHigh,LengthLow
.1:							;               LengthHigh,Type
	push	dword [esi]		;
	call	DispInt			;    DispInt(MemChkBuf[j*4]); // 显示一个成员
	pop	eax					;
	stosd					;    ARDStruct[j*4] = MemChkBuf[j*4];
	add	esi, 4				;
	dec	edx					;
	cmp	edx, 0				;
	jnz	.1					;  }
	call	DispReturn		;  printf("\n");
	cmp	dword [dwType], 1	;  if(Type == AddressRangeMemory)
	jne	.2					;  {
	mov	eax, [dwBaseAddrLow];
	add	eax, [dwLengthLow]	;
	cmp	eax, [dwMemSize]	;    if(BaseAddrLow + LengthLow > MemSize)
	jb	.2					;
	mov	[dwMemSize], eax	;    MemSize = BaseAddrLow + LengthLow;
.2:							;  }
	loop	.loop			;}
				  ;
	call	DispReturn		;printf("\n");
	push	szRAMSize		;
	call	DispStr			;printf("RAM size:");
	add	esp, 4				;
							;
	push	dword [dwMemSize] ;
	call	DispInt			;DispInt(MemSize);
	add	esp, 4				 ;

	pop	ecx
	pop	edi
	pop	esi
	ret
; ---------------------------------------------------------------------------

; 启动分页机制 --------------------------------------------------------------
SetupPaging:
	; 根据内存大小计算应初始化多少PDE以及多少页表
	xor	edx, edx
	mov	eax, 0FFFFFFFFh ;[dwMemSize]由于显卡往往将地址映射到高地址处，所以页表需要映射整个地址空间
	mov	ebx, 400000h	; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
	div	ebx
	mov	ecx, eax		; 此时 ecx 为页表的个数，也即 PDE 应该的个数
	test	edx, edx
	jz	.no_remainder
	inc	ecx				; 如果余数不为 0 就需增加一个页表
.no_remainder:
	push	ecx			; 暂存页表个数

	; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞.

	; 首先初始化页目录
	mov	ax, SelectorFlatRW
	mov	es, ax
	mov	edi, PageDirBase	; 此段首地址为 PageDirBase
	xor	eax, eax
	mov	eax, PageTblBase | PG_P  | PG_USU | PG_RWW
.1:
	stosd
	add	eax, 4096		; 为了简化, 所有页表在内存中是连续的.
	loop	.1

	; 再初始化所有页表
	pop	eax					; 页表个数
	mov	ebx, 1024			; 每个页表 1024 个 PTE
	mul	ebx
	mov	ecx, eax			; PTE个数 = 页表个数 * 1024
	mov	edi, PageTblBase	; 此段首地址为 PageTblBase
	xor	eax, eax
	mov	eax, PG_P  | PG_USU | PG_RWW
.2:
	stosd
	add	eax, 4096			; 每一页指向 4K 的空间
	loop	.2

	mov	eax, PageDirBase
	mov	cr3, eax
	mov	eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	
	nop
	ret
; 分页机制启动完毕 ----------------------------------------------------------

; InitKernel ---------------------------------------------------------------------------------
; 将 KERNEL.BIN 的内容经过整理对齐后放到新的位置
; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
; --------------------------------------------------------------------------------------------
InitKernel:
        xor   esi, esi
        mov   cx, word [BaseOfKernelFilePhyAddr+2Ch]	;`. ecx <- pELFHdr->e_phnum
        movzx ecx, cx									; 将源操作数的内容拷贝到目的操作数，并将该值0扩展至16位或者32位
        mov   esi, [BaseOfKernelFilePhyAddr + 1Ch]		; esi <- pELFHdr->e_phoff
        add   esi, BaseOfKernelFilePhyAddr				;esi<-OffsetOfKernel+pELFHdr->e_phoff
.Begin:
        mov   eax, [esi + 0]
        cmp   eax, 0									; PT_NULL
        jz    .NoAction
        push  dword [esi + 010h]						;size ;`.
        mov   eax, [esi + 04h]							; |
        add   eax, BaseOfKernelFilePhyAddr				; | memcpy((void*)(pPHdr->p_vaddr),
        push  eax								   ;src ; |      uchCode + pPHdr->p_offset,
        push  dword [esi + 08h]					   ;dst ; |      pPHdr->p_filesz;
        call  MemCpy									; |
        add   esp, 12									;/
.NoAction:
        add   esi, 020h									; esi += pELFHdr->e_phentsize
        dec   ecx
        jnz   .Begin

        ret
; InitKernel ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 [SECTION .data1]	 ; 数据段
ALIGN	32
[BITS	32]
LABEL_DATA:				  
String3 db "Memory paging started !!!",0
Str3Len    equ $ - String3					   ;字符串长度					  
OffsetStr3 equ BaseOfLoaderPhyAddr + String3	
_szRAMSize			db	"RAM size:", 0
_szReturn			db	0Ah, 0
; 变量
_dwMCRNumber:			dd	0	; Memory Check Result
_dwDispPos:			dd	(80 * 0 + 0) * 2	; 屏幕第 0 行, 第 0 列。
_dwMemSize:			dd	0
_ARDStruct:			; Address Range Descriptor Structure
	_dwBaseAddrLow:		dd	0
	_dwBaseAddrHigh:	dd	0
	_dwLengthLow:		dd	0
	_dwLengthHigh:		dd	0
	_dwType:			dd	0
_PageTableNumber		dd	0
_MemChkBuf:	times	256	db	0
_VESA_OFF	times 256	db 0
_VideoLinearAdd  	dd	0
VESAMODE			dw  0
_ScreenX			dw	0
_ScreenY			dw	0
_NoOfProcessors		dw	0
_SpinLock			dw	0


;; 保护模式下使用这些符号
szRAMSize			equ	BaseOfLoaderPhyAddr + _szRAMSize
szReturn			equ	BaseOfLoaderPhyAddr + _szReturn
dwDispPos			equ	BaseOfLoaderPhyAddr + _dwDispPos
dwMemSize			equ	BaseOfLoaderPhyAddr + _dwMemSize
dwMCRNumber			equ	BaseOfLoaderPhyAddr + _dwMCRNumber
ARDStruct			equ	BaseOfLoaderPhyAddr + _ARDStruct
	dwBaseAddrLow	equ	BaseOfLoaderPhyAddr + _dwBaseAddrLow
	dwBaseAddrHigh	equ	BaseOfLoaderPhyAddr + _dwBaseAddrHigh
	dwLengthLow		equ	BaseOfLoaderPhyAddr + _dwLengthLow
	dwLengthHigh	equ	BaseOfLoaderPhyAddr + _dwLengthHigh
	dwType			equ	BaseOfLoaderPhyAddr + _dwType
MemChkBuf			equ	BaseOfLoaderPhyAddr + _MemChkBuf
VESA_OFF			equ	BaseOfLoaderPhyAddr + _VESA_OFF
VideoLinearAdd		equ	BaseOfLoaderPhyAddr + _VideoLinearAdd
ScreenX				equ	BaseOfLoaderPhyAddr + _ScreenX
ScreenY				equ	BaseOfLoaderPhyAddr + _ScreenY
NoOfProcessors		equ BaseOfLoaderPhyAddr + _NoOfProcessors
SpinLock			equ BaseOfLoaderPhyAddr + _SpinLock

; 堆栈就在数据段的末尾
StackSpace:	times	1024*16	db	0
TopOfStack	equ	BaseOfLoaderPhyAddr + $	; 栈顶
; SECTION .data1 之结束 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

