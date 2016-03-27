#include "type.h"

typedef struct
{
	u16 VendorID;							u16 DeviceID;				
	u16 Command;							u16 Status;					
	u8 RevisionID;			u8 ClassCode[3];								
	u8 CacheLineSize;		u8 LatencyTimer; 	u8 HeaderType;		u8 BIST;    	
	u32 BAR0;
	u32 BAR1;
	u32 BAR2;
	u32 BAR3;
	u32 BAR4;
	u32 BAR5;
	u32 CardbusCSPointer;
	u16 SubsystemVendorID;					u16 SubsystemID;						
	u32 ExpansionROMBaseAddress;
	u8 CapabilitiesPointer;	u8  Reversed0[3];											
	u32 Reversed1;
	u8 InterruptLine;		u8 InterruptPin;	u8 Min_Gnt;			u8 Max_Lat;		
}PCIConfigurationSpace;

u32 PCI_I(u32 index);
u32 PCI_O(u32 index);

void PCI_Scan(PCIConfigurationSpace * buffer);