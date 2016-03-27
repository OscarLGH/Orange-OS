
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
RTC.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//这个要包含在proto.h之前否则proto里某些定义不识别
#include "proto.h"
#include "global.h"

/***********************************************************************/
/*Ring 1
/*Request RTC time from CMOS,called when system initialized

/***********************************************************************/
void GetCurrentTime(Time * time)
{
	unsigned int temp;
	Out_Byte(0x70,0x80|0x00);//秒
	temp = In_Byte(0x71);
	time->second = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制	+的优先级比&高！
	
	Out_Byte(0x70,0x80|0x02);//分
	temp = In_Byte(0x71);
	time->minite = (temp&0x0F) + (temp>>4)*10;  //BCD转十进制
	
	Out_Byte(0x70,0x80|0x04);//时
	temp = In_Byte(0x71);
	time->hour = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制
	
	Out_Byte(0x70,0x80|0x07);//日
	temp = In_Byte(0x71);
	time->day = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制
	
	Out_Byte(0x70,0x80|0x08);//月
	temp = In_Byte(0x71);
	time->month = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制
	
	Out_Byte(0x70,0x80|0x09);//年
	temp = In_Byte(0x71);
	time->year = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制
	
	Out_Byte(0x70,0x80|0x06);//星期
	temp = In_Byte(0x71);
	time->week = (temp&0x0F) + (temp>>4)*10;   //BCD转十进制
	
}