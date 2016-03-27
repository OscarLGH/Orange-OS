#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"					//?a??òa°üo??úproto.h???°·??òprotoà??3D???ò?2?ê?±e
#include "proto.h"
#include "global.h"
#include "font_8x16.h"
//#include "math.h"

/******************************************************/
/*				Basic graphics driver				  */
/*					Ring 0							  */
/*										Oscar 2013 	  */
/******************************************************/

GraphicsLayers GraLayerInfo; 
int VRAMBUF = 0x04000000;


void InitGraphic()
{
	int i;
	int *p;
	int count = BootParam.ScreenX*BootParam.ScreenY*NumOfLayer;
	int *addr = BootParam.video_linear_addr;
	//第0层：背景层
	GraLayerInfo.GraLayer[0].LayerStartAddr = VRAMBUF;
	GraLayerInfo.GraLayer[0].Pid = 0;
	GraLayerInfo.GraLayer[0].PosX = 0;
	GraLayerInfo.GraLayer[0].PosY = 0;
	GraLayerInfo.GraLayer[0].Length = BootParam.ScreenX;
	GraLayerInfo.GraLayer[0].Width = BootParam.ScreenY;
	GraLayerInfo.TopLayer = 0;
	p = GraLayerInfo.GraLayer[0].LayerStartAddr;

	for(i=1; i<NumOfLayer; i++)
	{
		//每个图层分配固定显存
		GraLayerInfo.GraLayer[i].LayerStartAddr = VRAMBUF + i*BootParam.ScreenX*BootParam.ScreenY*4;
		GraLayerInfo.GraLayer[i].Pid = -1;
	}
	count = BootParam.ScreenX*BootParam.ScreenY;
	for(i=0; i<count; i++)
	{
		*addr++ = 0x00000000;
	}
}

int AllocateLayer(int PosX,int PosY,int Length,int Width)
{
	int i;
	for(i=0; i<NumOfLayer; i++)
	{
		if(GraLayerInfo.GraLayer[i].Pid == -1)
			break;
	}
	if(i<NumOfLayer)
	{
		GraLayerInfo.GraLayer[i].PosX = PosX;
		GraLayerInfo.GraLayer[i].PosY = PosY;
		GraLayerInfo.GraLayer[i].Length = Length;
		GraLayerInfo.GraLayer[i].Width = Width;
		GraLayerInfo.GraLayer[i].Pid = 0;//getpid();
		GraLayerInfo.TopLayer = i;
		GraLayerInfo.LayerToVRAM = i;
		ZeroLayer(i);
		return i;		//返回申请到的图层
	}
	else
		return -1;	
}

void SlideLayer(int PosX,int PosY,int LayerNum)
{
	int x = GraLayerInfo.GraLayer[LayerNum].PosX;
	int y = GraLayerInfo.GraLayer[LayerNum].PosY;
	int Len = GraLayerInfo.GraLayer[LayerNum].Length;
	int Wid = GraLayerInfo.GraLayer[LayerNum].Width;
	int m,n;
	int *saddr;
	int *daddr;
	int StartAddr = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr;
	int limit = BootParam.ScreenX*BootParam.ScreenY*4;
	if(PosX<BootParam.ScreenX&&PosY<BootParam.ScreenY&&PosX>=0&&PosY>=0)
	{
		if(PosY<y)
		{
			for(m=0; m<Wid; m++)
			{
				saddr = StartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
				daddr = StartAddr+(PosY+m)*BootParam.ScreenX*4 + PosX*4;
				for(n=0; n<Len; n++)
				{
					*daddr++=*saddr++;
				}
			}
		}
		if(PosY>y)
		{
			for(m=Wid; m>=0; m--)
			{
				saddr = StartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
				daddr = StartAddr+(PosY+m)*BootParam.ScreenX*4 + PosX*4;
				for(n=0; n<Len; n++)
				{
					*daddr++=*saddr++;
				}
			}
		}
		if(PosY == y)
		{
			if(PosX<x)
			{
				for(m=0; m<Wid; m++)
				{
					saddr = StartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
					daddr = StartAddr+(PosY+m)*BootParam.ScreenX*4 + PosX*4;
					for(n=0; n<Len; n++)
					{
						*daddr++=*saddr++;
					}
				}
			}
			if(PosX>x)
			{
				for(m=0; m<Wid; m++)
				{
					saddr = StartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
					daddr = StartAddr+(PosY+m)*BootParam.ScreenX*4 + PosX*4;
					for(n=Len; n<=0; n--)
					{
						*daddr--=*saddr--;
					}
				}
			}
		}
		
		
		RefreshArea(PosX,PosY,Len,Wid,LayerNum);
		GraLayerInfo.GraLayer[LayerNum].PosX = PosX;
		GraLayerInfo.GraLayer[LayerNum].PosY = PosY;
		if(PosX - x >= Len || PosY - y >= Wid || x - PosX >= Len || y - PosY >= Wid)
		{
			RefreshArea(x,y,Len,Wid,0);
		}
		else
		{
			if(PosX >= x && PosY >= y)
			{
				RefreshArea(x,y,Len,PosY-y,0);
				RefreshArea(x,y,PosX-x,Wid,0);
			}
			else if(PosX >= x && PosY <= y)
			{
				RefreshArea(x,y,PosX-x,Wid,0);
				RefreshArea(x,PosY+Wid,Len,y-PosY,0);
			}
			else if(PosX <= x && PosY >= y)
			{
				RefreshArea(x,y,Len,PosY-y,0);
				RefreshArea(PosX+Len,PosY,x-PosX,Wid,0);
			}
			else if(PosX <= x && PosY <= y)
			{
				RefreshArea(x,PosY+Wid,Len,y-PosY,0);
				RefreshArea(PosX+Len,y,x-PosX,Wid,0);
			}
			
		}
	}
	else
		return;
}

void GetBackGround(int LayerNum)
{
	int x = GraLayerInfo.GraLayer[LayerNum].PosX;
	int y = GraLayerInfo.GraLayer[LayerNum].PosY;
	int m,n;
	int Len = GraLayerInfo.GraLayer[LayerNum].Length;
	int Wid = GraLayerInfo.GraLayer[LayerNum].Width;
	int * addr = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr;
	int * baddr;
	int j;
	for(m=0; m<Wid; m++)
	{
		addr  = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
		baddr = BootParam.video_linear_addr+(y+m)*BootParam.ScreenX*4 + x*4;
		for(n=0;n<Len;n++)
		{
			for(j=LayerNum-1; j>=0; j--)
			{
				if(x+n>=GraLayerInfo.GraLayer[j].PosX && 
					y+m>=GraLayerInfo.GraLayer[j].PosY && 
					x+n<GraLayerInfo.GraLayer[j].PosX+GraLayerInfo.GraLayer[j].Length && 
					y+m<GraLayerInfo.GraLayer[j].PosY+GraLayerInfo.GraLayer[j].Width)
				{
					break;
				}
			}
			baddr = GraLayerInfo.GraLayer[j].LayerStartAddr + (y+m)*BootParam.ScreenX*4 + x*4;
			*addr=*baddr;
			baddr++;
			addr++;
		}
	}
}
void FreeLayer(int LayerNum)
{
	int i;
	if(LayerNum > GraLayerInfo.TopLayer || LayerNum < 0)
		return;
	for(i=LayerNum; i<=GraLayerInfo.TopLayer; i++)
		GraLayerInfo.GraLayer[i]=GraLayerInfo.GraLayer[i+1];
	GraLayerInfo.GraLayer[i-1].Pid = -1;
	GraLayerInfo.TopLayer--;
	RefreshArea(0,0,BootParam.ScreenX,BootParam.ScreenY);
}

void SetTop(int LayerNum)
{
	int i;
	int x = GraLayerInfo.GraLayer[LayerNum].PosX;
	int y = GraLayerInfo.GraLayer[LayerNum].PosY;
	int Len = GraLayerInfo.GraLayer[LayerNum].Length;
	int Wid = GraLayerInfo.GraLayer[LayerNum].Width;
	int *addr,*vaddr;
	int m,n;

	for(m=0; m<Wid; m++)
	{
		addr  = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr+(y+m)*BootParam.ScreenX*4 + x*4;
		vaddr = BootParam.video_linear_addr+(y+m)*BootParam.ScreenX*4 + x*4;
		for(n=0;n<Len;n++)
		{
			*vaddr++=*addr++;
		}
	}
	GraLayerInfo.LayerToVRAM = LayerNum;
	
}


void ZeroLayer(int LayerNum)
{
	int * addr = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr;
	int count = BootParam.ScreenX*BootParam.ScreenY;
	int i = 0;
	for(i=0; i<count; i++)
	{
		*addr++ = 0x00000000;			//透明黑色
	}
}

void RefreshArea(int PosX,int PosY,int Len,int Wid,int LayerNum)
{
	int i,j;
	int m,n;
	int *addr;
	int *vaddr;
	int FrameOffset = BootParam.ScreenX*BootParam.ScreenY;
	PosX+=GraLayerInfo.GraLayer[LayerNum].PosX;
	PosY+=GraLayerInfo.GraLayer[LayerNum].PosY;
	
	for(m=0; m<Wid; m++)
	{
		addr  = GraLayerInfo.GraLayer[0].LayerStartAddr+(m+PosY)*BootParam.ScreenX*4 + PosX*4;
		vaddr = BootParam.video_linear_addr+(m+PosY)*BootParam.ScreenX*4 + PosX*4;
		if(PosY+m>=GraLayerInfo.GraLayer[0].PosY && PosY+m<GraLayerInfo.GraLayer[0].PosY+GraLayerInfo.GraLayer[0].Width)
			for(n=0;n<Len+1;n++)
			{
				for(j=GraLayerInfo.TopLayer; j>=0; j--)
				{
					if((*(addr+j*FrameOffset)>>24) != 0)	//自上而下寻找第一个不透明的点
						break;
				}
				if(j != -1)							//找到，则将第一个遇到的点写入显存显示
					*vaddr++ = *(addr+++j*FrameOffset);
				else								//未找到，则将该层该点写入显存显示
				{
					*vaddr++ = addr++;
				}
			}
	}
}

void DrawPoint(int PosX,int PosY,int Color,int LayerNum)
{
	/*																		//对于大多数画点操作，均不会写在图层范围外，对于正常的点判断是否在图层内属于浪费
	if(PosX>=GraLayerInfo.GraLayer[LayerNum].Length ||
	   PosY>=GraLayerInfo.GraLayer[LayerNum].Width  ||
	   GraLayerInfo.GraLayer[LayerNum].Pid == -1)
		return;
	*/
	PosX+=GraLayerInfo.GraLayer[LayerNum].PosX;
	PosY+=GraLayerInfo.GraLayer[LayerNum].PosY;
	int * LayerAddr = GraLayerInfo.GraLayer[LayerNum].LayerStartAddr;
	int Offset = PosY*BootParam.ScreenX + PosX;
	
	LayerAddr += Offset;								// 写入帧缓存
	*LayerAddr = Color;
}

void DrawPointVRAM(int PosX,int PosY,int Color)
{
	if(PosX>=BootParam.ScreenX ||
	   PosY>=BootParam.ScreenY)
		return;
	int * PhysicalAddr = BootParam.video_linear_addr;
	int Offset = PosY*BootParam.ScreenX + PosX;

	PhysicalAddr += Offset;
	*PhysicalAddr = Color;
}


void DrawChar(char ch,int PosX,int PosY,int Color,int LayerNum)
{
	int i,j,k;
	char font;
	unsigned char temp = 0x80;
	int x = PosX+GraLayerInfo.GraLayer[LayerNum].PosX;
	int y = PosY+GraLayerInfo.GraLayer[LayerNum].PosY;
	int color;
	int *p;

	for(i=0; i<16; i++)
	{
		font = fontdata_8x16[ch*16 + i];
		for(j=0; j<8; j++)
		{
			if(font&(temp>>j))
			{
				DrawPoint(PosX+j,PosY+i,Color,LayerNum);
			}
		}
	}
}

void DrawString(char * str,int PosX,int PosY,int Color,int LayerNum)
{
	int i;
	int x=PosX;
	int y=PosY;
	int Len = GraLayerInfo.GraLayer[LayerNum].Length;
	int Wid = GraLayerInfo.GraLayer[LayerNum].Width;
	
	for(i=0; (*str)!=0; i++)
	{
		if((*str) == '\n')
		{
			x = 0;
			y+= 16;
		}
		else
		{
			if(Len-x<8)
			//处理到达显示边界的情况
			{
				x = 0;
				y+=16;
			}
			else if(Len-x==8)
			{
				DrawChar((*str),x,y,Color,LayerNum);
				x = 0;
				y+=16;
			}
			else
			{
				DrawChar((*str),x,y,Color,LayerNum);
				x+=8;
			}
		}
		if((*str) == '\t')
			x += 4*8;
			
		str ++;
	}
}

#define abs(x) x>0?x:-x;
//Bresenham画线算法
void DrawLine(int pSX,int pSY,int pEX,int pEY,int Color, int LayerNum)
{
	int dx,dy,dx2,dy2,dxabs,dyabs,p0x,px,p0y,py;
	int i,j;
	float k;
	int sk;
	int sx,sy,ex,ey;
	int ymin,ymax;
	
	//Exchange the x so that sx<ex
	if(pSX<=pEX)
	{
		sx = pSX;
		ex = pEX;
		sy = pSY;
		ey = pEY;
	}
	else
	{
		sx = pEX;
		ex = pSX;
		sy = pEY;
		ey = pSY;
	}
	ymin = sy>ey?ey:sy;
	ymax = sy>ey?sy:ey;
	
	dx = ex - sx;
	dy = ey - sy;
	
	dxabs = dx>0?dx:-dx;
	dyabs = dy>0?dy:-dy;
	
	dx2 = 2*dxabs;
	dy2 = 2*dyabs;
	
	p0x = 2*dyabs - dxabs;
	px  = 2*dyabs - 2*dxabs;
	p0y = 2*dxabs - dyabs;
	py  = 2*dxabs - 2*dyabs;
	
	//Draw the first point of the line
	DrawPoint(sx,sy,Color,LayerNum);
		
	if(dy==0)
	{
		for(i=0;i<ex-sx;i++)
			DrawPoint(sx+i,sy,Color,LayerNum);
		return;
	}
	if(dx==0)
	{
		for(j=0;j<ymax-ymin;j++)
			DrawPoint(sx,sy+j,Color,LayerNum);
		return;
	}

	//除法必须在判断dx不为0时再继续，否则导致除0异常
	
	sk = dx*dy;
	
	if(sk>0)
	{
		if(dyabs<dxabs)
		{
			for(i = sx,j = sy; i <= ex; i++)
			{
				if(p0x < 0)
				{
					p0x += dy2;
				}
				else
				{
					j++;
					p0x += px;
				}
				DrawPoint(i,j,Color,LayerNum);
			}
		}
		else if(dyabs>dxabs)
		{
			for(i = sx,j = sy; j <= ey; j++)
			{
				if(p0y < 0)
				{
					p0y += dx2;
				}
				else
				{
					i++;
					p0y += py;
				}
				DrawPoint(i,j,Color,LayerNum);
			}
		}
		else
		{
			for(i = 0; i < ex-sx; i++)
				DrawPoint(sx+i,sy+i,Color,LayerNum);
		}
	}
	else    //利用对称性画负斜率直线
	{
		if(dyabs<dxabs)
		{			
			for(i = sx,j = ey; i <= ex; i++)
			{
				if(p0x < 0)
				{
					p0x += dy2;
				}
				else
				{
					j++;
					p0x += px;
				}
				DrawPoint(sx+ex-i,j,Color,LayerNum);
			}
		}
		else if(dyabs>dxabs)
		{
			for(i = sx,j = ey; j <= sy; j++)
			{
				if(p0y < 0)
				{
					p0y += dx2;
				}
				else
				{
					i++;
					p0y += py;
				}
				DrawPoint(sx+ex-i,j,Color,LayerNum);
			}
		}
		else
		{
			for(i = 0; i < ex-sx; i++)
				DrawPoint(ex-i,ey+i,Color,LayerNum);
		}
	}
}

//中点画圆算法
void DrawCircle(int PosX,int PosY,int Radius,int Fill,int Color,int LayerNum)
{
	int i,j;
	int Length = GraLayerInfo.GraLayer[LayerNum].Length;
	int Width  = GraLayerInfo.GraLayer[LayerNum].Width;
	int X = PosX;
	int Y = PosY;
	int p;
	int x,y;
	
	if(Fill == 1)
	{
		for(i=X-Radius; i<X+Radius; i++)
			for(j=Y-Radius; j<Y+Radius; j++)
			{
				if(i<0||i>=Length||j<0||j>=Width)
					continue;
				if((i-X)*(i-X)+(j-Y)*(j-Y) <= Radius*Radius)
					DrawPoint(i,j,Color,LayerNum);
			}
	}
	else
	{
		DrawPoint(X,Y+Radius,Color,LayerNum);
		DrawPoint(X+Radius,Y,Color,LayerNum);
		DrawPoint(X,Y-Radius,Color,LayerNum);
		DrawPoint(X-Radius,Y,Color,LayerNum);
		
		p = 1 - Radius;
		x = 0;
		y = Radius;
		while(x<=y)
		{
			if(p<0)
			{
				x++;
				p += (2*x+1);
			}
			else
			{
				x++;
				p += (2*(x-y)+1);
				y--;
			}
			DrawPoint(x+X,y+Y,Color,LayerNum);
			DrawPoint(X-x,y+Y,Color,LayerNum);
			DrawPoint(x+X,Y-y,Color,LayerNum);
			DrawPoint(X-x,Y-y,Color,LayerNum);
			DrawPoint(y+Y,x+X,Color,LayerNum);
			DrawPoint(Y-y,x+X,Color,LayerNum);
			DrawPoint(y+Y,X-x,Color,LayerNum);
			DrawPoint(Y-y,X-x,Color,LayerNum);
		}
	}
}
void DrawRect(int PosX,int PosY,int Length,int Width,int Fill,int Color, int LayerNum)
{
	int i,j;
	int X = PosX;
	int Y = PosY;
	if(Fill == 1)
	{
		for(i=Y; i<Y+Width; i++)
		{
			for(j=X; j<X+Length; j++)
			DrawPoint(j,i,Color,LayerNum);
		}
	}
	else
	{
		for(i=Y; i<Y+Width; i++)
		{
			for(j=X; j<X+Length; j++)
			DrawPoint(j,i,Color,LayerNum);
		}
	}
}


void task_Display()
{
	MESSAGE msg;
	while(1)
	{
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		//msg.u.m3.m3i1,msg.u.m3.m3i2:Position of graphics
		//msg.u.m3.m3i3,msg.u.m3.m3i4:Color,LayerNumber of graphics
		//other parameters
		switch (msg.type)
		{
			case INITG:
				InitGraphic();
				send_recv(SEND, src, &msg);
				break;
			case ALLOCATE:
				msg.RETVAL = AllocateLayer(msg.u.m3.m3i1,msg.u.m3.m3i2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case REFRESH:
				RefreshArea(msg.u.m3.m3i1,msg.u.m3.m3i2,(short int)msg.u.m3.m3l1,(short int)(msg.u.m3.m3l1>>16),msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case FREE:
				FreeLayer(msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case TOP:
				SetTop(msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case ZERO:
				ZeroLayer(msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case REDRAW:
				GetBackGround(msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case SLIDE:
				SlideLayer(msg.u.m3.m3i1,msg.u.m3.m3i2,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case POINT:
				DrawPoint(msg.u.m3.m3i1,msg.u.m3.m3i2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case CHAR:
				DrawChar((char)msg.u.m3.m3l1,msg.u.m3.m3i1,msg.u.m3.m3i2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case STRING:
				DrawString((char *)msg.u.m3.m3p1,msg.u.m3.m3i1,msg.u.m3.m3i2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case LINE:
				//DrawLine(int SX,int SY,int EX,int EY,Pixel Color, int LayerNum);
				send_recv(SEND, src, &msg);
				break;
			case RECTANGLE:
				DrawRect(msg.u.m3.m3i1,msg.u.m3.m3i2,(short int)msg.u.m3.m3l1,(short int)(msg.u.m3.m3l1>>16),(int)msg.u.m3.m3l2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			case CIRCLE:
				DrawCircle(msg.u.m3.m3i1,msg.u.m3.m3i2,(int)msg.u.m3.m3l1,(int)msg.u.m3.m3l2,msg.u.m3.m3i3,msg.u.m3.m3i4);
				send_recv(SEND, src, &msg);
				break;
			
		default:
			panic("unknown msg type");
			break;
		}
	}
}

