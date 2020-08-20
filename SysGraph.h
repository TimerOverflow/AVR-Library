/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysGraph.h
*/
/*********************************************************************************/
#ifndef __SYS_GRAPH_H__
#define	__SYS_GRAPH_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define SYS_GRAPH_REVISION_DATE				20200323
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 03. 23.					- 데이터 자료형 4바이트로 변경.
	Jeong Hyun Gu

	2019. 11. 12.					- 초기버전. (7000T egl library만 지원)
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define null		0
#define	true		1
#define	false		0

/*********************************************************************************/
/**Enum**/

/*********************************************************************************/
/**Struct**/

typedef struct
{
	tU16 x, y, w, h;
}tag_Rect;

typedef	struct
{
	struct
	{
		tU8	Init					:		1;
	}Ready;
	
	tS32 MinData, MaxData;
	tU16 Range_X, Range_Y;
	tU16 PosTop, PosBottom, PosLeft, PosRight;
	tU16 DataBufQty;
}tag_SysGraphGrid;

typedef struct
{
	struct
	{
		tU8	Init				:		1;
		tU8	Grid				:		1;
	}Ready;

	tag_SysGraphGrid *Grid;
	tS32 *DataBuf;
	tS32 *BufHead;
	tS32 *DrawHead;
}tag_SysGraphData;

/*********************************************************************************/
/**Function**/

void InitGraphGrid(tag_SysGraphGrid *Grid, tS32 Min, tS32 Max, tag_Rect *Border, tU16 DataBufQty);
void LinkGraphAndGrid(tag_SysGraphData *Graph, tag_SysGraphGrid *Grid);
tU8 InitGraphData(tag_SysGraphData *Graph);

void DrawGraphData(tag_SysGraphData *Graph, tU32 Color);
void SetGraphData(tag_SysGraphData *Graph, tS32 Data);
void RefreshGraphGrid(tag_SysGraphGrid *Grid);

/*********************************************************************************/
#endif //__SYS_GRAPH_H__
