
;%define	_BOOT_DEBUG_	; 做 Boot Sector 时一定将此行注释掉!将此行打开后用 nasm Boot.asm -o Boot.com 做成一个.COM文件易于调试

%ifdef	_BOOT_DEBUG_
	org  0100h			; 调试状态, 做成 .COM 文件, 可调试
%else
	org  07c00h			; Boot 状态, Bios 将把 Boot Sector 加载到 0:7C00 处并开始执行
%endif

;================================================================================================
%ifdef	_BOOT_DEBUG_
BaseOfStack		equ	0100h	; 调试状态下堆栈基地址(栈底, 从这个位置向低地址生长)
%else
BaseOfStack		equ	07c00h	; Boot状态下堆栈基地址(栈底, 从这个位置向低地址生长)
%endif

%include	"load.inc"
;================================================================================================

	jmp short LABEL_START		; Start to boot.
	nop							; 这个 nop 不可少

; 下面是 FAT32 磁盘的头, 之所以包含它是因为下面用到了磁盘的一些信息
%include	"Fat32BPB.inc"

LABEL_START:	
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack

	; 清屏
	mov	ax, 0600h			; AH = 6,  AL = 0h
	mov	bx, 0700h			; 黑底白字(BL = 07h)
	mov	cx, 0				; 左上角: (0, 0)
	mov	dx, 0184fh			; 右下角: (80, 50)
	int	10h					; int 10h

	xor	ah, ah				; ┓
	mov	dl, [BIOS_drive]	; ┣ 硬盘复位
	int	13h					; ┛

	mov	dh, 0				; "Booting."
	call	DispStr			; 显示字符串
	; 下面在 A 盘的根目录寻找 LOADER.BIN
	mov	word [wSectorNo], SectorNoOfRootDirectory
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	; ┓
	jz	LABEL_NO_LOADERBIN				; ┣ 判断根目录区是不是已经读完
	dec	word [wRootDirSizeForLoop]		; ┛ 如果读完表示没有找到 LOADER.BIN
	mov	ax, BaseOfLoader
	mov	es, ax					; es <- BaseOfLoader
	mov	bx, OffsetOfLoader		; bx <- OffsetOfLoader	于是, es:bx = BaseOfLoader:OffsetOfLoader
	mov	ax, [wSectorNo]			; ax <- Root Directory 中的某 Sector 号
	mov [BlockNum_L32],	ax
	mov [BufAddr_H16],es
	mov [BufAddr_L16],bx

	call	ReadSector

	mov	si, LoaderFileName		; ds:si -> "LOADER  BIN"
	mov	di, OffsetOfLoader		; es:di -> BaseOfLoader:0100 = BaseOfLoader*10h+100
	cld
	mov	dx, 80h								; 一个簇应该是80h的循环次数
LABEL_SEARCH_FOR_LOADERBIN:
	cmp	dx, 0								; ┓循环次数控制,
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
; 我们要找的 LOADER.BIN
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME			;	继续循环

LABEL_DIFFERENT:
	and	di, 0FFE0h					; else ┓	di &= E0 为了让它指向本条目开头
	add	di, 20h						;      ┃
	mov	si, LoaderFileName			;      ┣ di += 20h  下一个目录条目
	jmp	LABEL_SEARCH_FOR_LOADERBIN	;      ┛

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov	dh, 2				; "No LOADER."
	call	DispStr			; 显示字符串
%ifdef	_BOOT_DEBUG_
	mov	ax, 4c00h			; ┓
	int	21h					; ┛没有找到 LOADER.BIN, 回到 DOS
%else
	jmp	$					; 没有找到 LOADER.BIN, 死循环在这里
%endif

LABEL_FILENAME_FOUND:				; 找到 LOADER.BIN (rootentry) 后便来到这里继续
	mov	ax, RootDirSectors
	and	di, 0FFE0h					; di -> 当前条目的开始/指向内存开始
	add	di, 01Ah					; di -> 首簇
	mov	cx, word [es:di]
	push	cx						; 保存此簇在 FAT 中的序号(只保存了低16位，做文件系统的时候注意保存高16位)
	mov ax,	cx								
	mul byte[Sectors_per_cluster]	; 文件第一簇所在扇区 = DeltaSectorNo + 序号*8
	add	ax, DeltaSectorNo			; 这句完成时 cx 里面变成 LOADER.BIN 的起始扇区号 (从 0 开始数的序号)
	mov cx, ax
	mov	ax, BaseOfLoader
	mov	es, ax						; es <- BaseOfLoader
	mov	bx, OffsetOfLoader			; bx <- OffsetOfLoader	于是, es:bx = BaseOfLoader:OffsetOfLoader = BaseOfLoader * 10h + OffsetOfLoader			
									; cx <- Sector 号
									; 以上代码只在第一次读取目录时调用，读到目录项后便一直在下面的模块中循环直到结束

LABEL_GOON_LOADING_FILE:
	push	ax			; ┓
	push	bx			; ┃
	mov	ah, 0Eh			; ┃ 每读一个簇(8个扇区)区就在 "Booting  " 后面打一个点, 形成这样的效果:
	mov	al, '.'			; ┃
	mov	bl, 0Fh			; ┃ Booting ......
	int	10h				; ┃
	pop	bx				; ┃
	pop	ax				; ┛


	mov	[BlockNum_L32], cx
	mov [BufAddr_H16],	es
	mov	[BufAddr_L16],	bx

	call	ReadSector			; 		根据计算出的FAT项取数据
	pop	ax						;       取出此簇在 FAT 中的序号<--|
																 ;|
	call	GetFATEntry											 ;|
																 ;|
	cmp	eax, 0FFFFFFFh											 ;|
	jz	LABEL_FILE_LOADED										 ;|
	push	ax					;       保存簇在 FAT 中的序号  ---|
	
	mul byte[Sectors_per_cluster]						;   	一个簇8个扇区
	mov	dx, RootDirSectors
	add	ax, dx
	add	ax, DeltaSectorNo
	add	bx, BytesPerCluster		; 内存增量，存放下一个簇 注意这里是定义的常量，不能加方括号，否则数据位置不是预期值
	jmp	LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:

	mov	dh, 1				; "Ready."
	call	DispStr			; 显示字符串

	; 显示读到扇区的内容
	;mov	ax, BaseOfLoader
	;mov es,ax			
	;mov bp,	OffsetOfLoader
	;mov	cx, 1024		; CX = 串长度
	;mov	ax, 01301h		; AH = 13,  AL = 01h
	;mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	;mov	dx, 0300h
	;int	10h				; int 10h

; ****************************************************************************************
	jmp	BaseOfLoader:OffsetOfLoader	; 这一句正式跳转到已加载到内存中的 LOADER.BIN 的开始处
									; 开始执行 LOADER.BIN 的代码
									; Boot Sector 的使命到此结束
; ****************************************************************************************



;============================================================================
;变量
;----------------------------------------------------------------------------
wRootDirSizeForLoop	dw	RootDirSectors	; Root Directory 占用的扇区数, 在循环中会递减至零.
wSectorNo			dw	0				; 要读取的扇区号
;============================================================================
;磁盘地址数据包DAP	调用：AH=42H DL=驱动器号 DS:SI=数据包地址 INT 13H
;----------------------------------------------------------------------------
DiskAddressPacket:
PacketSize  	db	10h						; 大小：一般为10H，也有其他格式
Reserved		db	0						; 保留位，0
BlockCount		dw	8						; 要读取的扇区数(本程序中应该为一个簇)
BufAddr_L16		dw	0						; 要读入的内存地址低16位(段内偏移地址)
BufAddr_H16		dw  0						; 要读入的内存地址高16位(段基地址)
BlockNum_L32	dd	0						; 要读取的绝对扇区号低32位
BlockNum_H32	dd  0						; 要读取的绝对扇区号高32位
;============================================================================
;字符串
;----------------------------------------------------------------------------
LoaderFileName	db	"LOADER  BIN", 0	; LOADER.BIN 之文件名
; 为简化代码, 下面每个字符串的长度均为 MessageLength
MessageLength	equ	9
BootMessage:	db	"Booting  "			; 9字节, 不够则用空格补齐. 序号 0
Message1		db	"Ready.   "			; 9字节, 不够则用空格补齐. 序号 1
Message2		db	"No LOADER"			; 9字节, 不够则用空格补齐. 序号 2
;============================================================================


;----------------------------------------------------------------------------
; 函数名: DispStr
;----------------------------------------------------------------------------
; 作用:
;	显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)
DispStr:
	mov	ax, MessageLength
	mul	dh
	add	ax, BootMessage
	mov	bp, ax			; ┓
	mov	ax, ds			; ┣ ES:BP = 串地址
	mov	es, ax			; ┛
	mov	cx, MessageLength	; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	int	10h				; int 10h
	ret

;----------------------------------------------------------------------------
; 函数名: ReadSector
;----------------------------------------------------------------------------
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
	push	ax
	mov	ax, BaseOfLoader			; ┓
	sub	ax, 0100h					; ┣ 在 BaseOfLoader 后面留出 4K 空间用于存放 FAT
	mov	es, ax						; ┛
	pop	ax
	mov dl,	4						; 一个FAT项占四个字节
	mul dl							; 求出FAT项的字节偏移量（相对FAT1目录）

	xor	dx, dx						; 现在 ax 中是 FATEntry 在 FAT 中的偏移量. 下面来计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	div	word[Bytes_per_sector]		; dx:ax / Bytes_per_sector  ==>	ax <- 商   (FATEntry 所在的扇区相对于 FAT 来说的扇区号)
									; dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0						; bx <- 0	于是, es:bx = (BaseOfLoader - 100):00 = (BaseOfLoader - 100) * 10h					
	add	ax, SectorNoOfFAT1			; 此句执行之后的 ax 就是 FATEntry 所在的扇区号

	mov	[BlockNum_L32], ax
	mov [BufAddr_H16],	es
	mov	[BufAddr_L16],	bx
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

times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55					; 结束标志
