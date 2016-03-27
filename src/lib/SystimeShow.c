
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SystimeToString.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2014
Ring 0
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//这个要包含在proto.h之前否则proto里某些定义不识别
#include "proto.h"
#include "global.h"
/**********************************************************************/
/*
/*
/*
/*
/*
/*
/*
/*********************************************************************/
void SystimeToString(Time time,char * R_time,char * R_date,int methord)
{
	char * week[] =
	{
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thurday",
		"Friday",
		"Saturday",
		"Sunday"
	};
	char * month[] = 
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};
	char * appendix[] = 
	{
		"th",
		"st",
		"nd",
		"rd",
		"th",
		"th",
		"th",
		"th",
		"th",
		"th",
	};
	if(methord == 0)
	{
		sprintf(R_date,"20%d/%d/%d",time.year,time.month,time.day);
		sprintf(R_time,"%d%d:%d%d:%d%d",time.hour/10,time.hour%10,time.minite/10,time.minite%10,time.second/10,time.second%10);
	}
	if(methord == 1)
	{
		sprintf(R_date,"%s,%s %d%s 20%d",week[time.week-1],month[time.month-1],time.day,appendix[time.day%10],time.year);
		sprintf(R_time,"%d%d:%d%d:%d%d",time.hour/10,time.hour%10,time.minite/10,time.minite%10,time.second/10,time.second%10);
	}
}
