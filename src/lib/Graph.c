
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Speaker.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Oscar 2013
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//这个要包含在proto.h之前否则proto里某些定义不识别
#include "proto.h"
#include "global.h"

void InitGraph()
{
	//This function should not be called by user application,
	//it can only be called by system application for once
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = INITG;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

int AllocateGraph(int x,int y,int len,int wid)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = ALLOCATE;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = len;
	msg.u.m3.m3i4 = wid;
	send_recv(BOTH, TASK_DISPLAY, &msg);
	return msg.RETVAL;
}

void RefreshGraph(int x,int y,int len,int wid,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = REFRESH;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3l1 = (short int)len + (int)(wid<<16);
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void ReDrawGraph(int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = REDRAW;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void FreeGraph(int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = FREE;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void TopGraph(int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = TOP;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void ZeroGraph(int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = ZERO;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void SlideGraph(int x,int y,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = SLIDE;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void Point(int x,int y,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = Point;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void Rectangle(int x,int y,int len,int wid,int fill,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = RECTANGLE;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	
	msg.u.m3.m3l1 = (short int)len + (int)(wid<<16);
	msg.u.m3.m3l2 = fill;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void Charactor(char ch,int x,int y,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = CHAR;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	
	msg.u.m3.m3l1 = ch;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void String(char * str,int x,int y,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = STRING;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	msg.u.m3.m3p1 = (void *)str;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void Line(int x,int y,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = LINE;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}

void Circle(int x,int y,int radius,int fill,int color,int layer)
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = CIRCLE;
	msg.u.m3.m3i1 = x;
	msg.u.m3.m3i2 = y;
	msg.u.m3.m3i3 = color;
	msg.u.m3.m3i4 = layer;
	
	msg.u.m3.m3l1 = radius;
	msg.u.m3.m3l2 = fill;
	send_recv(BOTH, TASK_DISPLAY, &msg);
}
