
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Mouse.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//这个要包含在proto.h之前否则proto里某些定义不识别
#include "proto.h"
#include "global.h"
#include "Mouse.h"

MouseData Mouse_Data;
void InitMouse()
{
	Out_Byte(0x64,0xa8);// 允许 鼠标 接口 
	Out_Byte(0x64,0xd4);// 通知 8042 下个字节的发向 0x60 的数据将发给 鼠标
	Out_Byte(0x60,0xf4);// 允许 鼠标 发数据
	Out_Byte(0x64,0x60);// 通知 8042,下个字节的发向 0x60 的数据应放向 8042 的命令寄存器
	Out_Byte(0x60,0x47);// 许可键盘及 鼠标 接口及中断
}

void MouseHandler()
{
	static int NoMousePackage;	// 此变量用来记录这是第几个数据包了 因为 鼠标 发来的每个数据包 都会引起一个中断

	static int SignX;
	static int SignY;
	static int SignZ;
	
	char PackageData;
	
	PackageData = In_Byte(0x60);
	switch(NoMousePackage ++)
	{
		case 0:
			Mouse_Data.LeftButton = PackageData & 0x01 ? 1:0;
			Mouse_Data.RightButton = PackageData & 0x02 ? 1:0;
			Mouse_Data.MidButton = PackageData & 0x04 ? 1:0;
			SignX = PackageData & 0x10 ? 0xffffff00:0;
			SignY = PackageData & 0x20 ? 0xffffff00:0;
			break;
		case 1:
			Mouse_Data.PositionX += (PackageData | SignX);
			break;
		case 2:
			Mouse_Data.PositionY += (PackageData | SignY);
			NoMousePackage = 0;				// 2D鼠标，3D鼠标请注释掉该语句
			DrawPointVRAM(Mouse_Data.PositionX,Mouse_Data.PositionY,0xffffffff);
			break;
		case 3:
			SignZ = PackageData & 0x08 ? 0xffffff00:0;
			Mouse_Data.MidWheel +=(PackageData | SignZ);
			NoMousePackage = 0;
			break;
	}
	
}



