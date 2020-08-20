/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : DataSampling.h
*/
/*********************************************************************************/
#ifndef __DATA_SAMPLING_H__
#define	__DATA_SAMPLING_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define DATA_SAMPLING_REVISION_DATE		20200724
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 07. 24.					- 이제 샘플링 데이터의 자료형을 tS8, tS16, tS32 중 선택할 수 있음.
	Jeong Hyun Gu

	2020. 04. 16.					- DataSamplingResetData()에서 FillBuffer()호출 부분 삭제.
	Jeong Hyun Gu

	2019. 07. 23.					- SysTypedef.h 적용.
	Jeong Hyun Gu

	2018. 04. 03.					- DataSamplingResetData() 함수 추가.
	Jeong Hyun Gu

	2017. 10. 17.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define	false				0
#define	true				1
#define null				0

/*********************************************************************************/
/**Enum**/


/*********************************************************************************/
/**Struct**/

typedef struct
{
	struct
	{
		tU8 InitGeneral					:				1;				//필수초기화
		tU8 InitFillBuffer			:				1;				//버퍼초기화
	}Bit;

	void *Buf;
	tS16 Index, BufSize, Level;
	tS32 Sum;
	tS8 DataSize;
}tag_DataSampling;

/*********************************************************************************/
/**Function**/

tU8 DataSamplingInitGeneral(tag_DataSampling *Smp, tS16 BufSize, tS8 DataSzie);
void DataSamplingChangeLevel(tag_DataSampling *Smp, tS16 Level);
tS32 DataSamplingGetData(tag_DataSampling *Smp, tS32 Data);
void DataSamplingResetData(tag_DataSampling *Smp);

/*********************************************************************************/
#endif //__DATA_SAMPLING_H__
