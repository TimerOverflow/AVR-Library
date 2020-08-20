/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysSdkLite.h
*/
/*********************************************************************************/
#ifndef __SYS_SDK_LITE_H__
#define	__SYS_SDK_LITE_H__
/*********************************************************************************/
#include <stdlib.h>
#include "SysTypedef.h"
#include "SysSdkLiteConfig.h"
/*********************************************************************************/
#define SYS_SDK_LITE_REVISION_DATE		20200720
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 07. 20.					- AdTempCalc_20k_TypeB() 함수 추가. 슈타인하트 공식은 기준저항과 상관없이
	Jeong Hyun Gu						결정된다. 오롯이 센서 저항값에 의해 결정되므로 센서 사양만 변경되지 않으면
													기준저항이나 ADC 해상도와 상관없이 적용 가능하다. 지금까지 기준저항에 따라
													세분화한 함수들 그렇게 할 필요가 없었다. 정확도를 요하는 구간을 선택하는 옵션이
													있는데 그에 따라 공식이 약간 변경될 수는 있다. F/W 개바 및 유지보수 전략적인 면에서 보았을 때 
													통합이 필요해 보인다.

	2020. 07. 03.					- SetRunningTime() 함수 추가.
	Jeong Hyun Gu

	2020. 06. 10.					- CalcPercentage() 함수의 Resolution 인자 tS16 -> tU16으로 변경.
	Jeong Hyun Gu

	2020. 05. 22.					- CalcPercentage() 함수에서 실수형 절대값 얻는 부분 abs() -> fabs()로 변경.
	Jeong Hyun Gu

	2020. 05. 20.					- abs() 함수 삭제. 대신 stdlib.h included.
	Jeong Hyun Gu					- AdTempCalc_20k_TypeA() 함수 추가.

	2020. 03. 11.					- CheckScheduleTimeStop() 함수 추가. 시스트로닉스 표준 스케줄 함수.
	Jeong Hyun Gu					- RecordInput() 함수의 Min, Max 인수 타입 tS32로 변경.

	2019. 12. 12.					- GetDateAfterDay() 함수 추가. 특정날짜에서 n일 더한 후 날짜 계산.
	Jeong Hyun Gu					- GetDiffDaysEarlierToAfterDate() 함수 추가. 특정 날짜 사이의 일수 계산.

	2019. 11. 26.					- 헤더파일에 함수 설명 주석 추가.
	Jeong Hyun Gu					- CnvTempC_To_F() 함수 추가.

	2019. 10. 31.					- Setbit(), Clrbit(), Chkbit() 매크로 함수 삭제.
	Jeong Hyun Gu					- STM32등 32bit CPU와 호환 위해 AdTempCalc_10(), AdTempCalc_27(), AdTempCalc_4_7()의 
													타입 float -> double로 변경. 

	2019. 10. 17.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define	true		1
#define	false		0
#define	null		0

/*********************************************************************************/
/**Macro Function**/

#define GetArrayQty(Arry)			(sizeof(Arry) / sizeof(Arry[0]))

/*********************************************************************************/
/*
	@brief
	기준저항 10kohm, 10bit ADC, steinhart NTC 계산 함수.
	
	@example
	CurTemp = AdTempCalc_10(Adc);
*/
#ifdef __SDK_LITE_AD_TEMP_CALC_10__
double AdTempCalc_10(double x);
#endif
/*********************************************************************************/
/*
	@brief
	기준저항 27kohm, 10bit ADC, steinhart NTC 계산 함수.
	
	@example
	CurTemp = AdTempCalc_27(Adc);
*/
#ifdef __SDK_LITE_AD_TEMP_CALC_27__
double AdTempCalc_27(double x);
#endif
/*********************************************************************************/
/*
	@brief
	기준저항 4.7kohm, 10bit ADC, steinhart NTC 계산 함수.
	
	@example
	CurTemp = AdTempCalc_4_7(Adc);
*/
#ifdef __SDK_LITE_AD_TEMP_CALC_4_7__
double AdTempCalc_4_7(double x);
#endif
/*********************************************************************************/
/*
	@brief
	기준저항 20kohm, 11bit ADC, steinhart NTC 계산 함수.
	
	@example
	CurTemp = AdTempCalc_20k_TypeA(Adc);
*/
#ifdef __SDK_LITE_AD_TEMP_CALC_20K_TYPEA__
double AdTempCalc_20k_TypeA(double x);
#endif
/*********************************************************************************/
/*
	@brief
	기준저항 20kohm, 11bit ADC, steinhart NTC 계산 함수.
	
	@example
	CurTemp = AdTempCalc_20k_TypeA(Adc);
*/
#ifdef __SDK_LITE_AD_TEMP_CALC_20K_TYPEB__
double AdTempCalc_20k_TypeB(double x);
#endif
/*********************************************************************************/
/*
	@brief
	제어스텝 계산 함수.
	
	@example
	설정온도에서 OFF, 설정온도 + 편차에서 1스텝 조건의 냉방스텝
	CoolingStep = CalcCtrlStep(CurTemp, TempSet, TempSet + CoolDev, CoolingStep);
*/
#ifdef	__SDK_LITE_CALC_CTRL_STEP__
tU8 CalcCtrlStep(tS16 Data, tS16 Set, tS16 SetPlusDev, tU8 CurStep);
#endif
/*********************************************************************************/
/*
	@brief
	비례제어량 계산.
	
	@example
	0~100% 해상도의 냉방비례 제어. 설정온도에서 0% 설정온도+냉방편차에서 100%
	CoolingAo = CalcPercentage(CurTemp, SetTemp, SetTemp + CoolDev, 100);
*/
#ifdef	__SDK_LITE_CALC_PERCENTAGE__
float CalcPercentage(tS16 Data, tS16 Min, tS16 Max, tU16 Resolution);
#endif
/*********************************************************************************/
/*
	@brief
	레코드 입력 함수.
	4~20mA, 1~5V등 아날로그 비례입력에 대한 계산.
	
	Adc <= (AdcMin / 2) 조건일 경우 단선 에러로 판단하고
	(Min - 1) 값을 리턴한다.
	
	@example
	input data Type: 0~50% humidity, 1~5V
	target cpu adc input voltage range: 0~5V
	ADC resolution : 10bit
	
	HumiRecMin = 0;
	HumiRecMax = 50;
	CurHumi = RecordInput(Adc, HumiRecMin, HumiRecMax, 204, 1023);
	if(CurHumi < HumiRecMin) HumiSenError = true;
*/
#ifdef __SDK_LITE_RECORD_INTPUT__
float RecordInput(tU16 Adc, tS32 Min, tS32 Max, tU16 AdcMin, tU16 AdcMax);
#endif
/*********************************************************************************/
/*
	@brief
	데이터 샘플링 함수.
	주로 센서 데이터 입력에 적용.
	
	default 샘플링 횟수는 16회이고, 샘플링 횟수 변경을 원할 경우 SysSdkLiteConfig.h에 아래와 같이 선언.
	#define GET_SAMPLING_LEV 32
	
	@example
	CurTemp = GetSampling(AdTempCalc_4_7(Adc), &Smp);
*/
#ifdef __SDK_LITE_GET_SAMPLING__
#ifndef GET_SAMPLING_LEV
#define GET_SAMPLING_LEV	16
#endif
typedef	struct
{
	tU8	Init;
	tU16 Index;
	tS32 Sum;
	tS16 Buf[GET_SAMPLING_LEV];
}tag_SamplingVar;
tS16 GetSampling(tS16 Dat, tag_SamplingVar *Smp);
#endif
/*********************************************************************************/
/*
	@brief
	콤프 제어 함수. 콤프레서는 보통 전자변과 콤프레서를 한조로 묶어 제어한다.
	OFF 시 배관 내 압력을 떨어뜨리는 펌프다운이라는 특수 제어가 있고, 그 동안 시스트로닉스에서
	정의한 제어 동작을 함수로 정리.
	
	아래와 같이 상태입력이 필요하다.
	모든 입력은 선택사항이다. 사양에 따라 적절히 입력을 결정한다.
	
	tag_CompEachControl::In.Bit.LpInput							LP입력 상태, LP입
	tag_CompEachControl::In.Bit.LpInputOnUse				콤프 ON조건에서 LP입력에 따라 콤프ON되는 제어 조건 적용 여부.
	tag_CompEachControl::In.Bit.LpInputOffUse				펌프다운 중 LP입력에 따라 펌프다운 종료 제어 조건 적용 여부.
	tag_CompEachControl::In.Bit.HardKill						펌프다운 중 즉시OFF 할 수 있는 경보 flag.
	tag_CompEachControl::In.Bit.PumpDownTimeUse			설정한 시간에 따라 펌프다운 제어 조건 적용 여부. 만약 LP와 시간 모두 사용할 경우 or 조건으로 적용됨.
	tag_CompEachControl::In.CompDelaySet						콤프지연시간, 전자변 ON 후 콤프가 ON되기 까지 지연 시간.
	tag_CompEachControl::In.RestartDelaySet					콤프재기동시간, 콤프 OFF 후 설정한 재기동 시간 카운트가 진행된다. 잔여 카운트가 남았다면 콤프,전자변은 ON되지 않음.
	tag_CompEachControl::In.PumpDownSet							펌프다운 시간설정.
	
	@example
	if(TF.Bit.msec100)
	{
		CompEachCtrl.In.Bit.LpInput = LpInput;
		CompEachCtrl.In.Bit.LpInputOnUse = true;
		CompEachCtrl.In.Bit.LpInputOffUse = true;
		CompEachCtrl.In.Bit.HardKill = CompHardKillAlm;
		CompEachCtrl.In.Bit.PumpDownTimeUse = Sys.PumpDownTime ? true : false;
		CompEachCtrl.In.CompDelaySet = Sys.CompDelay * 10;
		CompEachCtrl.In.RestartDelaySet = 0;
		CompEachCtrl.In.PumpDownSet = Sys.PumpDownTime * 10;
		CompEachControl(&CompEachCtrl, CompOnFlag);
		Sys.Do1.Bit.Lsv = CompEachCtrl.Out.Bit.Sv;
		Sys.Do1.Bit.Comp = CompEachCtrl.Out.Bit.Comp;
	}
*/
#ifdef __SDK_LITE_COMP_EACH_CONTROL__
typedef struct
{
	struct
	{
		struct
		{
			tU8 LpInput							:		1;
			tU8 LpInputOnUse				:		1;
			tU8 LpInputOffUse				:		1;
			tU8 HardKill						:		1;
			tU8 PumpDownTimeUse			:		1;
		}Bit;

		tU16 CompDelaySet;
		tU16 RestartDelaySet;
		tU16 PumpDownSet;
	}In;

	struct
	{
		struct
		{
			tU8 Sv									:		1;
			tU8 Comp								:		1;
		}Bit;
	}Out;

	struct
	{
		tU8 Condition							:		1;
		tU8 FirstExe							:		1;
	}Bit;

	tU16 DelayCnt;
	tU16 LpDelay;
	tU16 RestartDelayCnt;
}tag_CompEachControl;
void CompEachControl(tag_CompEachControl *Comp, tU8 Run);
#endif
/*********************************************************************************/
/*
	@brief
	디지털 입력 감지 함수.
	주로 DI포트의 입력을 감지시간과 조건에 따라 상태를 결정하여 리턴한다.
	
	@example
	운전 중이고, 히터가 ON중일 때 감지하는 자동 복귀 히터 경보 DI포트.
	
	if(TF.Bit.msec100)
	{
		HeaterErr = CheckInputEvent(AlmDelay * 10, &Input, Sys.Do1.Bit.Heater, Sys.SF.Bit.ActualPower);
	}
*/
#ifdef __SDK_LITE_CHECK_INPUT_EVENT__
typedef	struct
{
	tU8	In;
	tU8	Result;
	tU16	Cnt;
}tag_InputEvent;
tU8	CheckInputEvent(tU16 Delay, tag_InputEvent *Input, tU8 Condition, tU8 PreCondition);
#endif
/*********************************************************************************/
/*
	@brief
	온도 섭씨를 화씨로 변환.
	
	@example
	배율이 -1인 tS16 온도변수 Temp;
	Temp = (tS16) (CnvTempC_To_F((float) Temp / 10) * 10);
*/
#ifdef __SDK_LITE_TEMP_C_TO_F__
float	CnvTempC_To_F(float Temp);
#endif
/*********************************************************************************/
/*
	@brief::GetDateAfterDay()
	특정날짜에서 n일 더한 후 날짜 계산.
	
	@example::GetDateAfterDay()
	Date.Year = Sys.DateYear + 2000;
	Date.Month = Sys.DateMonth;
	Date.Date = Sys.DateDate;
	GetDateAfterDay(&Date, AddDay);
	AfterYear = Date.Year - 2000;
	AfterMonth = Date.Month;
	AfterDate = Date.Date;
	
	@brief::GetDiffDaysEarlierToAfterDate()
	두개의 날짜가 며칠 차이가 나는지 일 단위로 반환.
	
	@example::GetDiffDaysEarlierToAfterDate()
	tS32 Days;
	tag_SysSdkDate Earlier = { .Year = 2000, .Month = 1, Date = 1 };
	tag_SysSdkDate After = { .Year = 2019, .Month = 12, Date = 12 };
	
	Days = GetDiffDaysEarlierToAfterDate(&Earlier, &After);
*/
#ifdef __SDK_LITE_CALC_TIME_DATE_FUNC__
typedef struct
{
	tU16 Year;
	tU8 Month;
	tU8 Date;
}tag_SysSdkDate;
void GetDateAfterDay(tag_SysSdkDate *Date, tU16 AddDay);
tS32 GetDiffDaysEarlierToAfterDate(tag_SysSdkDate *Earlier, tag_SysSdkDate *After);
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CHECK_SCHDULE_TIME_STOP__
/*
	@brief
	스케줄정지 여부 확인 함수
	현재시간과 설정시간을 비교하여 현재 시간이 운전 또는 정지조건인지 확인한다.
	
	@example
	스케줄 기능 사용여부를 사용할 수 있고, 시작시간과 종료시간 조건을 확인하여
	스케줄 정지 조건인지 확인한다.
	Sys.SchUse : 스케줄 기능 사용여부.
	Sys.SchTime[] : 요일별 시작시간, 종료시간.
	Sys.SchEachDayUse : 요일별 운전/정지 설정 (비트단위)
	
	if(TF.Bit.sec)
	{
		Idx = Sys.DateDay >= 1 ? Sys.DateDay - 1 : 0;
		if(Sys.SchUse && CheckScheduleTimeStop(&Sys.SchTime[Idx], Sys.SchEachDayUse & (1 << Idx), Sys.TimeHour, Sys.TimeMin))
		{
			Sys.SF.Bit.ScheduleStop = true;
		}
		else
		{
			Sys.SF.Bit.ScheduleStop = false;
		}
	}
*/
typedef struct
{
	tU16 Start;
	tU16 End;
}tag_CheckScheduleTime;
tU8 CheckScheduleTimeStop(tag_CheckScheduleTime *Sch, tU8 Run, tU8 CurHour, tU8 CurMin);
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_SET_RUNNING_TIME__
/*
	@brief
	적산시간 계산 함수. 초단위로 카운트하며 1시간 단위로 비휘발성 메모리에 저장할 수 있도록 신호 리턴.
	
	@example
	if(TF.Bit.sec)
	{
		if(SetRunningTime(Sys.Do1.Bit.Comp1,	&Sys.Actiontime[0],	&RunTimeManage[0])) SetEepWriteEnable(&EepSys);
		if(SetRunningTime(Sys.Do1.Bit.Comp2,	&Sys.Actiontime[1],	&RunTimeManage[1])) SetEepWriteEnable(&EepSys);
	}
*/
typedef struct
{
	tU8 SecCnt, MinCnt;
	tU16 Temp;
}tag_RuntimeVal;
tU8 SetRunningTime(tU8 Condition, tU16	*RunTime,	tag_RuntimeVal *Data);
#endif
/*********************************************************************************/
#endif //__SYS_SDK_LITE_H__
