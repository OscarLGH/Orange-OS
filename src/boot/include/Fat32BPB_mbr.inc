	OEM                   db  'MSDOS5.0'            ;//8Bytes  
    Bytes_per_sector      dw   512  	 
	Sectors_per_cluster   db   8  
	Reserved_sectors      dw   1136
	Number_of_FATs        db   2  
	Root_entries          dw   0  
	Sectors_small         dw   0  
	Media_descriptor      db   0f8h  
	Sectors_per_FAT_small dw   0  
	Sectors_per_track     dw   63  
	Heads                 dw   255  
	Hidden_sectors        dd   0  
	Sectors               dd   7823233
	  
	Sectors_per_FAT       dd   7624
	Extended_flags        dw   0  
	Version               dw   0  
	Root_dir_1st_cluster  dd   2  
	FSInfo_sector         dw   1 
	Backup_boot_sector    dw   6  
	times 12 db 0  
	  
	BIOS_drive            db   80h  
	db 0  
	Ext_boot_signature    db   29h  
	Volume_serial_number  dd   3404242512 
	Volume_label          db   "Oscar's OS "       ;//11个字节  
	File_system           db   'FAT32   '          ;//8个字节 

; 注意在x86体系中按little-endian方式存放数据，所以此处多字节数据不用反转顺序

; -------------------------------------------------------------------------
; 基于 FAT32 头的一些常量定义，如果头信息改变，下面的常量可能也要做相应改变
; -------------------------------------------------------------------------
; BPB_FATSz32
FATSz			equ	7624

; 根目录占用空间:
; RootDirSectors = ((BPB_RootEntCnt*32)+(BPB_BytsPerSec–1))/BPB_BytsPerSec
; 但如果按照此公式代码过长，故定义此宏
RootDirSectors		equ	8

; Root Directory 的第一个扇区号	= BPB_RsvdSecCnt + (BPB_NumFATs * FATSz)
; RootDirSectors = 	Hidden_sectors + Reserved_sectors + Sectors_per_FAT*Number_of_FATs
SectorNoOfRootDirectory	equ	16384

; FAT1 的第一个扇区号	= Reserved_sectors
SectorNoOfFAT1		equ	1136

; 计算第0簇地址（实际上0,1簇都不使用，这里计算好了减少后面代码量）
; 文件目录区在第二簇，后续簇一般为文件簇
; DeltaSectorNo =  SectorNoOfRootDirectory - 2*8
DeltaSectorNo		equ	16368

BytesPerCluster		equ	  4096