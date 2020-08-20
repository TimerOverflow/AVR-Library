/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysHumiControl.h
*/
/*********************************************************************************/
#ifndef __SYS_HUMI_CONTROL_H__
#define	__SYS_HUMI_CONTROL_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define SYS_HUMI_CONTROL_REVISION_DATE				20200729
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 07. 29.					- Gaseup_Test_Mode_Control_Proc() 급수와 배수가 동시에 실행되지 않도록 수정.
	Jeong Hyun Gu

	2020. 07. 01.					- tag_SysHumiEtype::In.Set_baesuhoesu가 0일 경우 배수를 실행하지 않도록 수정.
	Jeong Hyun Gu

	2020. 06. 18.					- Gaseup_Test_Mode_Control_Proc() 함수내 지연 변수 다운카운트로 변경하고 외부로 공개.
	Jeong Hyun Gu					- 현재 운전 조건을 지시하는 tag_SysHumiEtype::Out.Bit.Status_Run 비트 추가.

	2020. 03. 04.					- EtypeCurrentTable[] 220V 전류 테이블 수정.
	Jeong Hyun Gu

	2020. 01. 31.					- __SYS_HUMI_CONTROL_TARGET_IAR_AVR__ 선언 추가 build target이 AVR일 경우
	Jeong Hyun Gu						'true'로 설정하여 RAM 용량 적게 소모하도록 지원.

	2020. 01. 02.					- SysTypedef.h 적용.
	Jeong Hyun Gu					- BesuXsegi_LeeSang_Check_Proc() 함수 삭제.
												- 용량별 배수시간을 일체형 가습전용 컨트롤러와 동일하게 수정.
												- 전류가 140%이상이면 배수점검 경보 발생하도록 수정.
												- 수동급수,배수는 동시에 ON할 수 없도록 변경.

	2019. 03. 28.					- Over_Current_Check_Control_Proc() 과전류 관련 카운트 및 배수관련 제어 수정.
	Jeong Hyun Gu

	2019. 03. 27.					- tag_SysHumiEtype::In.Bit.Di_WaterLevelLow 삭제.
	Jeong Hyun Gu					- tag_SysHumiEtype::In.Bit.Flag_Sec 내부 변수로 변경.
												- tag_SysHumiEtype::In.Bit.Flag_Min 내부 변수로 변경.
												- tag_SysHumiEtype::In.Bit.Flag_Hour 내부 변수로 변경.
												- tag_SysHumiEtype::In.Bit.Flag_Twink 내부 변수로 변경.
												- 내부에 TimeFlagControl() 함수 추가.

	2018. 07. 07.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define true		1
#define	false		0
#define	null		0

#define __SYS_HUMI_CONTROL_TARGET_IAR_AVR__			true

/*********************************************************************************/
/**Enum**/



/*********************************************************************************/
/**Struct**/

typedef struct
{
	struct
	{
		struct
		{
			tU8 Flag_msec100											:				1;
			tU8 Flag_TestMode											:				1;
			tU8 Flag_HumiOff											:				1;
			tU8 Di_WaterLevelHigh									:				1;
		}Bit;

		tU8 Set_seoljeongjeonab;
		tU8 Set_seoljeongyongryang;
		tU8 Set_baesuhoesu;
		tS16 etypecurrentvalue;
	}In;

	struct
	{
		struct
		{
			tU8 Do_WaterSupply										:				1;
			tU8 Do_WaterDrain											:				1;
			tU8 Do_Humi														:				1;

			tU8 Alm_WaterNeed											:				1;
			tU8 Alm_WaterDrain										:				1;
			tU8 Alm_CurrentSen										:				1;
			tU8 Alm_OverCurrent										:				1;
			
			tU8 Status_Run												:				1;
		}Bit;
	}Out;

	struct
	{
		tU8 gaseupgistatus;
	}InOut;

	struct
	{
		tU8 mansucheckflag											:				1;
		tU8 besu10sectimeflag										:				1;
		tU8 besu10mintimeflag										:				1;
		tU8 besuendchkflag											:				1;
		tU8 xsegikillflag												:				1;
		tU8 xsegidspflag												:				1;
		tU8 autobesuchkflag											:				1;
		tU8 Flag_Sec														:				1;
		tU8 Flag_Min														:				1;
		tU8 Flag_Hour														:				1;
		tU8 Flag_Twink													:				1;
	}Bit;

	tS16 mansusavecurrent;
	tS16 autosavecurrent;
	tS16 TargetCurrent;
	tS16 backupcurrent;

	tU8 geupsu20mintime;
	tU8 etypestepcount;
	tU8 watercheckerrtime;
	tU8 xsegidelaytime;
	tU8 xsegichecktime;
	tU8 besu10sectime;
	tU16 TargetDrainTime;
	tU16 autobesutime;
	tU8 etype72hour;
	tU8 etypechecktime;
	tU8 backchecktime;
	tU8 besu10time;
	tU8 currentlacktime;
	tU8 besuerrchat;
	tU8 lowcurrentchktime;
	tU8 overctxsegicnt;
	tU8 overcurrentchat;
	tU8 ManualCtrlCnt;
	
	tU8 TimeSecCnt;
	tU8 TimeMinCnt;
	tU8 TimeHourCnt;
	tU8 TimeTwinkCnt;
}tag_SysHumiEtype;

/*********************************************************************************/
/**Function**/

void SysHumiEtypeProc(tag_SysHumiEtype *Humi, tU8 Run);
void SysHumiEtypeAlmClear(tag_SysHumiEtype *Humi);

/*********************************************************************************/
#endif //__SYS_HUMI_CONTROL_H__
