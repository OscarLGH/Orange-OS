#include "pci.h"
#include "type.h"

PCIConfigurationSpace PCI[255];
u32 PCI_Cnt = 0;

void PCI_Scan(PCIConfigurationSpace * buffer)
{
	u32 bus,device = 0,function = 0,pciregister = 0;
	u32 singlefunctionflag = 0;
	u32 * buf = (u32 *)buffer;
	u32 code = 0;
	for(bus = 0; bus<256; bus++)
		for(device = 0; device<64; device++)
		{
			code = 0x0c|(device<<10)|(bus<<16);
			if(1 == PCI_I(code)&0x00C00000)			
				singlefunctionflag = 1;
			for(function =0; function<8; function++)
			{
				/*Check the existance of the device*/
				code = 0x0|(function<<8)|(device<<10)|(bus<<16); 
				if(0xffffffff != PCI_I(code))
				{
					PCI_Cnt++;
					for(pciregister = 0; pciregister<0x3d; pciregister+=4)
					{
						code = pciregister|(function<<8)|(device<<10)|(bus<<16); 
						*buf++ = PCI_I(code);
					}
				}
			}
		}
}