/*************************************************************************//**
 *****************************************************************************
 * @file   include/sys/fs.h
 * @brief  Header file for FAT32 File System.
 * @author Oscar
 * @date   2014.4.25
 *****************************************************************************
 *****************************************************************************/



struct dev_drv_map {
	int driver_nr; /**< The proc nr.\ of the device driver. */
};


typedef struct PartitionTableEntry {
	
	u8  ActivePartition;
	u32 StartHead;
	u32 StartSector;
	u32 StartCylinder;
	
	u8 PartitionType;
	
  	u32 EndHead;
	u32 EndSector;
	u32 EndCylinder;
			
	u32 SectorsPrePar;		//分区之前空余扇区
 	u32 PartitionSectors;	//分区大小
	/*
	 * the following item(s) are only present in memory
	 */
	int	sb_dev; 	/**< the super block's home device */
}PartitionTableEntry;

#define PARTABSIZE 64

typedef struct FAT32Section{
	u32 SectorsPerFAT;
	u32 FirstRootDirSector;
	u16 FSINFOSector;
	u16 BackupBootSector;
	u8  BIOSDrive;
	u8  ExtBootSignature;
	u32 VolumeSerialNumber;
}FAT32Section;

#define FAT32SEC 512

typedef struct BootSectorFAT32{
	u16 BytesPerSector;
	u8  SectorsPerCluster;
	u16 ReservedSectors;
	u8  NumberOfFATs;
	
	u16 NumberOfSectorsSmall;
	u8  MediaDescriptor;
	u16 SectorsPerFATSmall;
	u16 SectorsPerTrack;
	u16 Heads;
	u32 HiddenSectors;
	u32 Sectors;
	
	FAT32Section fat32sec;
	
}BootSectorFAT32;

#define BOOTSECSIZE 512



typedef struct FSINFO{
	u32 FreeClusters;
	u32 NextAvailableClusters;
}FSINFO;

#define FSINFOSIZE 512

typedef struct FAT{
	u32 FATEntry[128];
}FAT;

typedef struct FileDirEntry{
	u8  FileName[255];
	u8  FileProperty;
	
	u8	CreatedTime_100ms;
	u8	CreatedTime_Hour;
	u8	CreatedTime_Minite;
	u8	CreatedTime_Second;
	
	u16 CreatedDate_Year;
	u8	CreatedDate_Month;
	u8 	CreatedDate_Day;
	
	u16 VisitedDate_Year;
	u8  VisitedDate_Month;
	u8  VisitedDate_Day;
	
	int StartCluster;
		
	u8 	ModifiedTime_Hour;
	u8 	ModifiedTime_Minite;
	u8 	ModifiedTime_Second;
	
	u16 ModifiedDate_Year;
	u8  ModifiedDate_Month;
	u8  ModifiedDate_Day;
	
	int SizeOfFile;
}FileDirEntry;

#define	SUPER_BLOCK_SIZE	56


typedef struct FileDescriptor
{
	u32 IndexOfFDE;
	u32 FilePointer;
}FileDescriptor;
/**
 * @struct inode
 * @brief  i-node
 *
 * The file, currently, have tree valid attributes only:
 *   - size
 *   - start_sect
 *   - nr_sects
 *
 * The \c start_sect and\c nr_sects locate the file in the device,
 * and the size show how many bytes is used.
 * If <tt> size < (nr_sects * SECTOR_SIZE) </tt>, the rest bytes
 * are wasted and reserved for later writing.
 *
 * \b NOTE: Remember to change INODE_SIZE if the members are changed
 */
struct inode {
	u32	i_mode;		/**< Accsess mode. Unused currently */
	u32	i_size;		/**< File size */
	u32	i_start_sect;	/**< The first sector of the data */
	u32	i_nr_sects;	/**< How many sectors the file occupies */
	u8	_unused[16];	/**< Stuff for alignment */

	/* the following items are only present in memory */
	int	i_dev;
	int	i_cnt;		/**< How many procs share this inode  */
	int	i_num;		/**< inode nr.  */
};

/**
 * @def   INODE_SIZE
 * @brief The size of i-node stored \b in \b the \b device.
 *
 * Note that this is the size of the struct in the device, \b NOT in memory.
 * The size in memory is larger because of some more members.
 */
#define	INODE_SIZE	32

/**
 * @struct file_desc
 * @brief  File Descriptor
 */
struct file_desc {
	int		fd_mode;	/**< R or W */
	int		fd_pos;		/**< Current position for R/W. */
	struct inode*	fd_inode;	/**< Ptr to the i-node */
};



#define RD_SECT(dev,sect_nr) rw_sector(DEV_READ, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* read one sector */ \
				       TASK_FS,				\
				       fsbuf);
#define WR_SECT(dev,sect_nr) rw_sector(DEV_WRITE, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* write one sector */ \
				       TASK_FS,				\
				       fsbuf);
