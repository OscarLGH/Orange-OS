/*************************************************************************//**
 *****************************************************************************
 * @file   FileSystem.c
 * @brief  
 * @author Oscar
 * @date   2014
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
//#include "config.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "global.h"
#include "proto.h"

#include "hd.h"

void get_fat32_params(BootSectorFAT32 * p);
void get_fat32_params(BootSectorFAT32 * p);
void get_FSINFO(FSINFO * p);
u32 get_root_dir(BootSectorFAT32 tem,FileDirEntry * p);
u32 get_next_cluster(BootSectorFAT32 tem, u32 current_cluster);
u32 fread_s(BootSectorFAT32 BSF, FileDirEntry fde [ ], FileDescriptor * fp, u8 * buf, u32 size);
FileDescriptor fopen(BootSectorFAT32 tem, char * filepath);



/*****************************************************************************
 *                                task_fs
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK FS.
 * 
 *****************************************************************************/
PUBLIC void task_fs()
{
	BootSectorFAT32 BSF;
	FileDirEntry FDE[50];
	FSINFO FSO;
	u8 * file_buffer_pointer;
	printl("Task FS begins.\n");

	/* open the device: hard disk */
	MESSAGE driver_msg;
	switch(driver_msg.type)
	{
		case DEV_OPEN:
			send_recv(BOTH, TASK_HD, &driver_msg);

			
			break;
		
			
	}
	spin("FS");
}

void get_fat32_params(BootSectorFAT32 * p)
{
	u8 buf[512];
	
	
	MESSAGE driver_msg;
	driver_msg.type = DEV_READ;
	driver_msg.DEVICE = 0;
	driver_msg.POSITION = 63;
	driver_msg.BUF = buf;
	driver_msg.CNT = 512;
	send_recv(BOTH, TASK_HD, &driver_msg);
	
	p->BytesPerSector = buf[0x0b]+(buf[0x0c]<<8);
	p->SectorsPerCluster = buf[0x0d];
	p->ReservedSectors = buf[0x0e]+(buf[0x0f]<<8);
	p->NumberOfFATs = buf[0x10];
	p->NumberOfSectorsSmall = 0;
	p->MediaDescriptor = buf[0x15];
	p->SectorsPerFATSmall = 0;
	p->SectorsPerTrack = buf[0x18]+(buf[0x19]<<8);
	p->Heads = (u16)buf[0x1a];
	p->HiddenSectors = buf[0x1c]+(buf[0x1d]<<8)+(buf[0x1e]<<16)+(buf[0x1f]<<24);
	p->Sectors = buf[0x20]+(buf[0x21]<<8)+(buf[0x22]<<16)+(buf[0x23]<<24);

	p->fat32sec.SectorsPerFAT = buf[0x24]+(buf[0x25]<<8)+(buf[0x26]<<16)+(buf[0x27]<<24);
	p->fat32sec.FirstRootDirSector = buf[0x2c]+(buf[0x2d]<<8)+(buf[0x2e]<<16)+(buf[0x2f]<<24);
	
	p->fat32sec.FSINFOSector = buf[0x30]+(buf[0x31]<<8);
	p->fat32sec.BackupBootSector = buf[0x32]+(buf[0x33]<<8);
	p->fat32sec.BIOSDrive = buf[0x40];
	p->fat32sec.ExtBootSignature = buf[0x42];
	p->fat32sec.VolumeSerialNumber = buf[0x43]+(buf[0x44]<<8)+(buf[0x45]<<16)+(buf[0x46]<<24);
}

void get_FSINFO(FSINFO * p)
{
	u8 buf[512];
	MESSAGE driver_msg;
	driver_msg.type = DEV_READ;
	driver_msg.DEVICE = 0;
	driver_msg.POSITION = 64;
	driver_msg.BUF = buf;
	driver_msg.CNT = 512;
	send_recv(BOTH, TASK_HD, &driver_msg);

	p->FreeClusters = buf[0x1e8]+(buf[0x1e9]<<8)+(buf[0x1ea]<<16)+(buf[0x1eb]<<24);
	p->NextAvailableClusters = buf[0x1ec]+(buf[0x1ed]<<8)+(buf[0x1ee]<<16)+(buf[0x1ef]<<24);
}

u32 get_root_dir(BootSectorFAT32 tem,FileDirEntry * p)
{
	u8 buf[512];
	u32 NumOfFileDir = 0;
	int i,j;
	u32 FirstClusterOffset = tem.ReservedSectors + tem.HiddenSectors + tem.NumberOfFATs * tem.fat32sec.SectorsPerFAT;
	MESSAGE driver_msg;
	driver_msg.type = DEV_READ;
	driver_msg.DEVICE = 0;
	driver_msg.BUF = buf;
	driver_msg.CNT = 512;
	for(j=0; j<tem.SectorsPerCluster ; j++)
	{
		
		driver_msg.POSITION = FirstClusterOffset + j;
		
		send_recv(BOTH, TASK_HD, &driver_msg);
		for(i=0; buf[i*32+0]!=0 && i<16; i++)
		{
			if(buf[i*32 + 0]!=0xe5&&buf[i*32 + 0x0B]!=0x0F)
			{
				NumOfFileDir++;
				memcpy(p->FileName,&buf[0 + i*32],11);
				p->FileProperty = buf[0x0B + i*32];
			
				p->CreatedTime_100ms = buf[0x0D + i*32];
				p->CreatedTime_Second = (buf[0x0E + i*32]&0x1F)*2 + buf[0x0D + i*32]*10/1000;
				p->CreatedTime_Minite = (buf[0x0E + i*32]>>5)+((buf[0x0F + i*32]&0x07)<<3);
				p->CreatedTime_Hour = buf[0x0F + i*32]>>3;
							
				p->CreatedDate_Day = (buf[0x10 + i*32]&0x1F);
				p->CreatedDate_Month = (buf[0x10 + i*32]>>5)+((buf[0x11 + i*32]&0x01)<<3);
				p->CreatedDate_Year = (buf[0x11 + i*32]>>1) + 1980;


				p->VisitedDate_Day = (buf[0x12 + i*32]&0x1F);
				p->VisitedDate_Month = (buf[0x12 + i*32]>>5)+((buf[0x13 + i*32]&0x01)<<3);
				p->VisitedDate_Year = (buf[0x13 + i*32]>>1) + 1980;

				p->ModifiedTime_Second = (buf[0x16 + i*32]&0x1F)*2;
				p->ModifiedTime_Minite = (buf[0x16 + i*32]>>5)+((buf[0x17 + i*32]&0x07)<<3);
				p->ModifiedTime_Hour = buf[0x17 + i*32]>>3;

				p->ModifiedDate_Day = (buf[0x18 + i*32]&0x1F);
				p->ModifiedDate_Month = (buf[0x18 + i*32]>>5)+((buf[0x19 + i*32]&0x01)<<3);
				p->ModifiedDate_Year = (buf[0x19 + i*32]>>1) + 1980;

				p->StartCluster = buf[0x1A + i*32] + (buf[0x1B + i*32]<<8) + (buf[0x14 + i*32]<<16) + (buf[0x15 + i*32]<<24);

				p->SizeOfFile = buf[0x1C + i*32] + (buf[0x1D + i*32]<<8) + (buf[0x1E + i*32]<<16) + (buf[0x1F + i*32]<<24);

				//加法运算符高于移位运算符，之前因为没有加括号导致移位溢出变为0
				//移位运算记得加括号，因为它优先级低，很容易引起错误

				p++;
			}
		}
	}
	return NumOfFileDir;	
}

u32 get_next_cluster(BootSectorFAT32 tem,u32 current_cluster)
{
	u32 fatsectoroff = current_cluster/128;
	u8 buf[512];
	u32 fatoff = current_cluster%128;
	u32 * p_buf = (u32 *)buf;
	u32 next_num;
	MESSAGE driver_msg;
	driver_msg.type = DEV_READ;
	driver_msg.DEVICE = 0;
	driver_msg.POSITION = tem.HiddenSectors + tem.ReservedSectors + fatsectoroff;
	driver_msg.BUF = buf;
	driver_msg.CNT = 512;
	send_recv(BOTH, TASK_HD, &driver_msg);
	next_num = *(p_buf + fatoff);
	
	return(next_num == 0x0fffffff? 0:next_num);
}

FileDescriptor fopen(BootSectorFAT32 tem,char * filepath)
{
	FileDirEntry fde[50];
	FileDescriptor fd;
	int i,j;
	j = get_root_dir(tem, fde);
	for(i=0; i<=j; i++)
	{
		if(!memcmp(filepath,fde[i].FileName,11))
			break;
	}
	fd.IndexOfFDE = i>j?0:i;
	fd.FilePointer = 0;
	return fd;
}

u32 fread_s(BootSectorFAT32 BSF,FileDirEntry fde[],FileDescriptor *fp,u8 *buf,u32 size)
{
	u8 buf0[512];
	u32 start_cluster = fde[fp->IndexOfFDE].StartCluster;
	u32 logical_cluster_num = (fp->FilePointer)/(BSF.SectorsPerCluster * 512);
	u32 logical_sector_offset = (fp->FilePointer)%(BSF.SectorsPerCluster * 512);
	u32 logical_byte_offset = (fp->FilePointer)%512;
	u32 phy_sector_num;
	u32 i,j;
	
	if(size > BSF.SectorsPerCluster*512 || fp->IndexOfFDE == 0)
		return 0;

	i = fde[fp->IndexOfFDE].StartCluster;
	j = 0;
	while(i != 0x0fffffff&& j<logical_cluster_num)
	{
		i = get_next_cluster(BSF, i);
		j++;
	}
	phy_sector_num =  BSF.HiddenSectors
					+ BSF.ReservedSectors
					+ BSF.NumberOfFATs * BSF.fat32sec.SectorsPerFAT
					+ (i-2) * BSF.SectorsPerCluster
					+ logical_sector_offset;
	
	MESSAGE driver_msg;
	driver_msg.type = DEV_READ;
	driver_msg.DEVICE = 0;
	driver_msg.POSITION = phy_sector_num;
	driver_msg.BUF = buf0;
	driver_msg.CNT = 512;
	send_recv(BOTH, TASK_HD, &driver_msg);

	memcpy(buf,&buf0[logical_byte_offset],size);
	fp->FilePointer += size;
	return size;
	

}
