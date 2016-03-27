#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//这个要包含在proto.h之前否则proto里某些定义不识别
#include "proto.h"
#include "global.h"

PUBLIC void Init8253()
{
	/* 初始化 8253 PIT */
    Out_Byte(TIMER_MODE, RATE_GENERATOR);
    Out_Byte(TIMER0, (u8) (TIMER_FREQ/Hz) );
    Out_Byte(TIMER0, (u8) ((TIMER_FREQ/Hz) >> 8));
}