/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysGraph.c
*/
/*********************************************************************************/
#include "SysGraph.h"
#include "adstar.h"
/*********************************************************************************/
#if(SYS_GRAPH_REVISION_DATE != 20200323)
#error wrong include file. (SysGraph.h)
#endif
/*********************************************************************************/
/** Global variable **/

/*********************************************************************************/
static tU8 CheckGraphGridInit(tag_SysGraphGrid *Grid)
{
	if(Grid->Ready.Init == false)
	{
		return false;
	}

	return true;
}
/*********************************************************************************/
static tU8	CheckGraphDataInit(tag_SysGraphData *Graph)
{
	if((Graph->Ready.Init == false) || (Graph->Ready.Grid == false))
	{
		return false;
	}

	if(CheckGraphGridInit(Graph->Grid) == false)
	{
		return false;
	}

	return true;
}
/*********************************************************************************/
static void MoveGraphBufHead(tS32 **pHead, tS32 *StartOfBuf, tS32 *EndOfBuf)
{
	if(++(*pHead) > EndOfBuf)
	{
		*pHead = StartOfBuf;
	}
}
/*********************************************************************************/
void InitGraphGrid(tag_SysGraphGrid *Grid, tS32 Min, tS32 Max, tag_Rect *Border, tU16 DataBufQty)
{
	Grid->MinData = Min;
	Grid->MaxData = Max;

	Grid->Range_X = Border->w;
	/* X축 배경 격자라인 */

	Grid->Range_Y = Border->h;
	/* Y축 배경 격자라인 */

	Grid->PosTop = Border->y;
	Grid->PosBottom = Border->y + Grid->Range_Y;
	Grid->PosLeft = Border->x;
	Grid->PosRight = Border->x + Grid->Range_X;
	
	Grid->DataBufQty = DataBufQty;

	Grid->Ready.Init = true;
}
/*********************************************************************************/
tU8 InitGraphData(tag_SysGraphData *Graph)
{
	tU16 i;

	if(Graph->Ready.Grid == false)
	{
		return false;
	}

	Graph->DataBuf = calloc(Graph->Grid->DataBufQty, sizeof(tS32));
	if(Graph->DataBuf == null)
	{
		return false;
	}

	Graph->DrawHead = Graph->BufHead = Graph->DataBuf;

	for(i = 0; i < Graph->Grid->DataBufQty; i++)
	{
		Graph->DataBuf[i] = Graph->Grid->PosBottom;
	}

	Graph->Ready.Init = true;
	return Graph->Ready.Init;
}
/*********************************************************************************/
void LinkGraphAndGrid(tag_SysGraphData *Graph, tag_SysGraphGrid *Grid)
{
	if(Grid->Ready.Init == false)
	{
		return;
	}

	Graph->Grid = Grid;
	Graph->Ready.Grid = true;
}
/*********************************************************************************/
void DrawGraphData(tag_SysGraphData *Graph, tU32 Color)
{
	tU16 i, Data[2];
	tS32 *BufHead;
	float DrawPitch;

	if(CheckGraphDataInit(Graph) == false) return;

	BufHead = Graph->DrawHead;
	DrawPitch = ((float) Graph->Grid->Range_X / Graph->Grid->DataBufQty);

	for(i = 0; i < (Graph->Grid->DataBufQty - 1); i++)
	{
		Data[0] = *BufHead;
		if((BufHead--) == (&Graph->DataBuf[0]))
		{
			BufHead = &Graph->DataBuf[Graph->Grid->DataBufQty - 1];
			//헤드 버퍼의 역방향으로 읽어야 최신 데이터 순으로 그려진다.
		}
		Data[1] = *BufHead;
		draw_thickline(Graph->Grid->PosLeft + (i * DrawPitch), Data[0], Graph->Grid->PosLeft + ((i + 1) * DrawPitch), Data[1], 2, Color);
	}
}
/*********************************************************************************/
void SetGraphData(tag_SysGraphData *Graph, tS32 Data)
{
	float	Ratio;

	if(CheckGraphDataInit(Graph) == false) return;

	Data -= Graph->Grid->MinData;
	Ratio = (float) (Graph->Grid->MaxData - Graph->Grid->MinData) / Graph->Grid->Range_Y;
	*(Graph->BufHead) = Graph->Grid->PosBottom - (tS16) ((float) Data / Ratio);
	/* DataBuf에 Data 삽입. */

	if(*Graph->BufHead > Graph->Grid->PosBottom) *Graph->BufHead = Graph->Grid->PosBottom;
	if(*Graph->BufHead < Graph->Grid->PosTop) *Graph->BufHead = Graph->Grid->PosTop;
	/* DataBuf의 값은 Pixel의 Y축 위치이므로 Bottom보다 크다는 것은 위치를 초과했다는 의미. */

	Graph->DrawHead = Graph->BufHead;
	MoveGraphBufHead(&(Graph->BufHead), &(Graph->DataBuf[0]), &(Graph->DataBuf[Graph->Grid->DataBufQty - 1]));
	/* 헤드 포인터 롤링. */
}
/*********************************************************************************/
void RefreshGraphGrid(tag_SysGraphGrid *Grid)
{
	EGL_RECT	Area;

	Area.x = Grid->PosLeft - 2;
	Area.y = Grid->PosTop - 2;
	Area.w = Grid->Range_X + 4;
	Area.h = Grid->Range_Y + 4;

	egl_window_redraw_rect(&Area);
	egl_draw();
}
/*********************************************************************************/

