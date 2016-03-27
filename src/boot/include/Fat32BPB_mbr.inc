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
	Volume_label          db   "Oscar's OS "       ;//11���ֽ�  
	File_system           db   'FAT32   '          ;//8���ֽ� 

; ע����x86��ϵ�а�little-endian��ʽ������ݣ����Դ˴����ֽ����ݲ��÷�ת˳��

; -------------------------------------------------------------------------
; ���� FAT32 ͷ��һЩ�������壬���ͷ��Ϣ�ı䣬����ĳ�������ҲҪ����Ӧ�ı�
; -------------------------------------------------------------------------
; BPB_FATSz32
FATSz			equ	7624

; ��Ŀ¼ռ�ÿռ�:
; RootDirSectors = ((BPB_RootEntCnt*32)+(BPB_BytsPerSec�C1))/BPB_BytsPerSec
; ��������մ˹�ʽ����������ʶ���˺�
RootDirSectors		equ	8

; Root Directory �ĵ�һ��������	= BPB_RsvdSecCnt + (BPB_NumFATs * FATSz)
; RootDirSectors = 	Hidden_sectors + Reserved_sectors + Sectors_per_FAT*Number_of_FATs
SectorNoOfRootDirectory	equ	16384

; FAT1 �ĵ�һ��������	= Reserved_sectors
SectorNoOfFAT1		equ	1136

; �����0�ص�ַ��ʵ����0,1�ض���ʹ�ã����������˼��ٺ����������
; �ļ�Ŀ¼���ڵڶ��أ�������һ��Ϊ�ļ���
; DeltaSectorNo =  SectorNoOfRootDirectory - 2*8
DeltaSectorNo		equ	16368

BytesPerCluster		equ	  4096