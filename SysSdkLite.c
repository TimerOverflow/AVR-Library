/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysSdkLite.c
*/
/*********************************************************************************/
#include <string.h>
#include <math.h>
#include "SysSdkLite.h"
/*********************************************************************************/
#if(SYS_SDK_LITE_REVISION_DATE != 20200922)
#error wrong include file. (SysSdkLite.h)
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_AD_TEMP_CALC_10__
double AdTempCalc_10(double x)
{
	/*
		type : temperature
		Rref : 10K ohm
		resolution : 10bit

		T1 : -30
		T2 : 0
		T3 : 30
	*/

	double result, temp;

	temp = (x * 10) / (1024 - x);
	temp = log(temp);

	result	= (double) 0.0026764;
	//c1a

	result += ((double) 0.00029149) * temp;
	//c2a

	result += ((double) 0.0000005387) * temp * temp * temp;
	//c3a

	result = 1/result;
	result -= (double) 273.15;
	result *= 10;

	return result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_AD_TEMP_CALC_27__
double AdTempCalc_27(double x)
{
	/*
		type : temperature
		Rref : 27K ohm
		resolution : 10bit

		T1 : -25
		T2 : 0
		T3 : 25
	*/

	double result, temp;

	temp = (x * 27) / (1024 - x);
	temp = log(temp);

	result	= (double) 0.002675838;
	//c1a

	result += ((double) 0.000291685) * temp;
	//c2a

	result += ((double) 0.0000005364) * temp * temp * temp;
	//c3a

	result = 1/result;
	result -= (double) 273.15;
	result *= 10;

	return result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_AD_TEMP_CALC_4_7__
double AdTempCalc_4_7(double x)
{
	/*
		type : temperature
		Rref : 4.7K ohm
		resolution : 10bit

		T1 : 3
		T2 : 52
		T3 : 101

		adc 1000: 약 -42도
		adc 50: 약 165도
		권장 사용 범위는 -40~160도.
		에러검출은 -45도 이하, 165이상일 경우.
	*/

	double result, temp;

	temp = (x * 4.7) / (1024 - x);
	temp = log(temp);

	result	= (double) 0.002687626;
	//c1a

	result += ((double) 0.000283519) * temp;
	//c2a

	result += ((double) 0.0000009757) * temp * temp * temp;
	//c3a

	result = 1/result;
	result -= ((double) 273.15);
	result *= 10;

	return result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_AD_TEMP_CALC_20K_TYPEA__
double AdTempCalc_20k_TypeA(double x)
{
	/*
		type : temperature
		Ntc source: 3.3V
		Vref : 2.7V
		Rref : 20K ohm
		resolution : 11bit

		T1 : 0
		T2 : 20
		T3 : 60

		adc 1585: 약 -20도
		adc 158: 약 80도
		권장 사용 범위는 -20~80도.
		에러검출은 -45도 이하, 85도 이상일 경우.
	*/

	double result, temp;

	x = x / 3.3 * 2.7;
	//VSOURCE, VREF가 다른 부분을 처리한다.

	temp = (x * 20) / (2047 - x);
	temp = log(temp);

	result	= (double) 0.002682401;
	//c1a

	result += ((double) 0.0002881) * temp;
	//c2a

	result += ((double) 0.0000006826) * temp * temp * temp;
	//c3a

	result = 1/result;
	result -= ((double) 273.15);
	result *= 10;

	return result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_AD_TEMP_CALC_20K_TYPEB__
double AdTempCalc_20k_TypeB(double x)
{
	/*
		type : temperature
		Ntc source: 3.3V
		Vref : 2.7V
		Rref : 20K ohm
		resolution : 11bit
		Sensor: Thermokon

		T1 : 0
		T2 : 20
		T3 : 60

		권장 사용 범위는 -10~80도.
		에러검출은 -15도 이하, 85도 이상일 경우.
	*/

	double result, temp;

	x = x / 3.3 * 2.7;
	//VSOURCE, VREF가 다른 부분을 처리한다.

	temp = (x * 20) / (2047 - x);
	temp = log(temp);

	result	= (double) 0.002772599;
	//c1a

	result += ((double) 0.000250797) * temp;
	//c2a

	result += ((double) 0.0000003341) * temp * temp * temp;
	//c3a

	result = 1/result;
	result -= ((double) 273.15);
	result *= 10;

	return result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CALC_CTRL_STEP__
tU8 CalcCtrlStep(tS16 Data, tS16 Set, tS16 SetPlusDev, tU8 CurStep)
{
	tS16 Sub, Dev;
	tU8 GetStep;

	/*
		Data: current data
		Set: set data , goal data, target data
		SetPlus:
		CurStep: current step

		Sub: subtract between 'Data' and 'Set'
		Dev: deviation
	*/

	Data = ((Set > SetPlusDev) && (Data > Set)) ? Set : Data;
	Data = ((Set < SetPlusDev) && (Data < Set)) ? Set : Data;

	Sub = abs(Set - Data);
	Dev = abs(Set - SetPlusDev);

	if(Dev == 0)
	{
		return 0;
		/* error */
	}

	GetStep = (Sub / Dev) > CurStep ? (Sub / Dev) : CurStep;

	if(GetStep != 0)
	{
		if(Sub >= (Dev * GetStep))
		{
			if(CurStep <= GetStep)
			{
				CurStep = GetStep;
			}
		}
		else if(Sub <= (Dev * (GetStep - 1)))
		{
			if(CurStep >= GetStep)
			{
				CurStep = GetStep - 1;
			}
		}
	}

	return CurStep;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CALC_PERCENTAGE__
float CalcPercentage(tS16 Data, tS16 Min, tS16 Max, tU16 Resolution)
{
	float Result;
	tS16 Range, Big, Small;

	/*
		Data: Current Data;
		Min: Minimum Calculrate Condition
		Max: Maximum Calculrate Condition

		Example:	CalcPercentage(100, 100, 100 + 50) = 0%
							CalcPercentage(125, 100, 100 + 50) = 50%
							CalcPercentage(150, 100, 100 + 50) = 100%

							CalcPercentage(100, 100, 100 - 50) = 0%
							CalcPercentage(75, 100, 100 - 50) = 50%
							CalcPercentage(50, 100, 100 - 50) = 100%
	*/

	Big = Max > Min ? Max : Min;
	Small = Max < Min ? Max : Min;
	Data = Data > Big ? Big : Data;
	Data = Data < Small ? Small : Data;

	Range = abs(Max - Min);
	Result = Data - Min;
	Result = fabs((Result * Resolution) / Range);

	return Result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_RECORD_INTPUT__
float RecordInput(tU16 Adc, tS32 Min, tS32 Max, tU16 AdcMin, tU16 AdcMax)
{
	tU16 ADCRANGE	= (AdcMax - AdcMin);
	float Result;

	if(Adc < (AdcMin / 2))
	{
		Result = Min - 1;
	}
	else if(Adc < AdcMin)
	{
		Result = Min;
	}
	else
	{
		if(Adc > AdcMax) Adc = AdcMax;
		Result = Max - Min;
		Adc -= AdcMin;
		Result = (float) Adc * (Result / ADCRANGE) + Min;
	}

	return Result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_GET_SAMPLING__
tS16 GetSampling(tS16 Dat, tag_SamplingVar *Smp)
{
	static const tU16 SAMPLING_LEVEL = sizeof(Smp->Buf) / sizeof(Smp->Buf[0]);
	tS16 Result;
	tU16 i;

	if(Smp->Init == false)
	{
		Smp->Init = true;
		Smp->Index = 0;
		for(i = 0; i < SAMPLING_LEVEL; i++)
		{
			Smp->Buf[i] = Dat;
			Smp->Sum += Dat;
		}
	}
	//초기화

	Smp->Sum -= Smp->Buf[Smp->Index];
	Smp->Buf[Smp->Index] = Dat;
	Smp->Sum += Smp->Buf[Smp->Index];
	Result = Smp->Sum / SAMPLING_LEVEL;

	if(++(Smp->Index) >= SAMPLING_LEVEL) Smp->Index = 0;

	return Result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_COMP_EACH_CONTROL__
void CompEachControl(tag_CompEachControl *Comp, tU8 Run)
{
	if(Comp->Bit.FirstExe == false)
	{
		Comp->Bit.FirstExe = true;
		Comp->RestartDelayCnt = Comp->In.RestartDelaySet;
	}

	if(Run == true)
	{
		if(Comp->RestartDelayCnt == 0)
		{
			Comp->Out.Bit.Sv = true;

			if(Comp->Bit.Condition == false){ Comp->Out.Bit.Comp = false; Comp->DelayCnt = Comp->In.CompDelaySet; }
			else if(Comp->DelayCnt == 0){ Comp->Out.Bit.Comp = true; }

			if(Comp->In.Bit.LpInputOnUse == true)
			{
				if(Comp->In.Bit.LpInput)
				{
					if(Comp->LpDelay >= 30) Comp->Bit.Condition = true;
					else Comp->LpDelay++;
				}
				else
				{
					if(Comp->LpDelay == 0) Comp->Bit.Condition = false;
					else Comp->LpDelay--;
				}
			}
			else
			{
				Comp->Bit.Condition = true;
			}
		}
	}
	else
	{
		Comp->LpDelay = 0;

		if(Comp->Out.Bit.Sv == true)
		{
			Comp->Out.Bit.Sv = false;
			Comp->DelayCnt = Comp->In.PumpDownSet;
			Comp->RestartDelayCnt = Comp->In.RestartDelaySet;
		}

		if((Comp->DelayCnt == 0 && Comp->In.Bit.PumpDownTimeUse == true) || (Comp->In.Bit.LpInputOffUse == true && Comp->In.Bit.LpInput == false) || Comp->In.Bit.HardKill == true)
		{
			Comp->Bit.Condition = Comp->Out.Bit.Comp = false;
		}
	}

	if(Comp->DelayCnt) Comp->DelayCnt--;
	if(Comp->RestartDelayCnt) Comp->RestartDelayCnt--;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CHECK_INPUT_EVENT__
tU8	CheckInputEvent(tU16 Delay, tag_InputEvent *Input, tU8 Condition, tU8 PreCondition)
{
	if((Input->In == false) && (Delay != 0) && (PreCondition == true))
	{
		if(Input->Cnt >= Delay) Input->Result = true;
		else if(Condition == true) Input->Cnt++;
		else Input->Cnt = 0;
	}
	else
	{
		Input->Cnt = 0;
		Input->Result = false;
	}
	return Input->Result;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_TEMP_C_TO_F__
float	CnvTempC_To_F(float Temp)
{
	return Temp * (float) 1.8 + 32;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CALC_TIME_DATE_FUNC__
tU16 DaysForMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static tU8 IsLeapYear(tU16 Year)
{
	if((Year % 4 == 0) && ((Year % 100 != 0) || (Year % 400 == 0))) return true;
	else return false;
}

static tS32 CnvDateToDay(tag_SysSdkDate *Date)
{
	tU8 i;
	tS32 Day = 0;

	Day = ((Date->Year - 1) * 365) + ((Date->Year - 1) / 4) - ((Date->Year - 1) / 100) + ((Date->Year - 1) / 400);

	if(IsLeapYear(Date->Year) == true)	DaysForMonth[2] = 29;
	else DaysForMonth[2] = 28;

	for(i = 1; i < Date->Month; i++)
	{
		Day += DaysForMonth[i];
	}

	Day += Date->Date;

	return Day;
}

void GetDateAfterDay(tag_SysSdkDate *Date, tU16 AddDay)
{
	tU16 YearSize;
	tU16 y = Date->Year;
	tU8 m = Date->Month;
	tU8 d = Date->Date;

	while(1)
	{
		if(IsLeapYear(y) == true) YearSize = 366;
		else YearSize = 365;

		if(AddDay < YearSize){ break; }
		else{ AddDay -= YearSize; y++; }
	}
	//연

	while(1)
	{
		if(IsLeapYear(y) == true) DaysForMonth[2] = 29;
		else DaysForMonth[2] = 28;

		if(AddDay <= DaysForMonth[m]) break;
		AddDay -= DaysForMonth[m];
		if(++m >= 13){ m = 1; y++; }
	}
	//월

	if((d + AddDay) > DaysForMonth[m])
	{
		d = d + AddDay - DaysForMonth[m];
		m++;
	}
	else
	{
		d += AddDay;
	}
	//일

	Date->Year = y;
	Date->Month = m;
	Date->Date = d;
}

tS32 GetDiffDaysEarlierToAfterDate(tag_SysSdkDate *Earlier, tag_SysSdkDate *After)
{
	return CnvDateToDay(After) - CnvDateToDay(Earlier);
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_CHECK_SCHDULE_TIME_STOP__
tU8 CheckScheduleTimeStop(tag_CheckScheduleTime *Sch, tU8 Run, tU8 CurHour, tU8 CurMin)
{
	tU16 CurTime, StartTime, EndTime;

	CurTime = (CurHour * 60) + CurMin;
	StartTime = ((Sch->Start >> 8) * 60) + (Sch->Start & 0x00FF);
	EndTime = ((Sch->End >> 8) * 60) + (Sch->End & 0x00FF);

	if(Run == false)
	{
		return true;
	}
	else if(StartTime < EndTime)
	{
		if((StartTime > CurTime) || (CurTime >= EndTime))
		{
			return true;
		}
	}
	else if(StartTime > EndTime)
	{
		if((EndTime <= CurTime) && (CurTime < StartTime))
		{
			return true;
		}
	}

	return false;
}
#endif
/*********************************************************************************/
#ifdef __SDK_LITE_SET_RUNNING_TIME__
tU8 SetRunningTime(tU8 Condition, tU16	*RunTime,	tag_RuntimeVal *Data)
{
	const	tU16 Maximum = 65000;

	if(Condition == true)
	{
		if(++Data->SecCnt >= 60)
		{
			Data->SecCnt = 0;
			if(++Data->MinCnt >= 60)
			{
				Data->MinCnt = 0;
				if(*RunTime < Maximum) (*RunTime)++;
				return true;
			}
		}
	}

	if(Data->Temp != *RunTime)
	{
		Data->Temp = *RunTime;
		if(Data->Temp == 0)
		{
			Data->SecCnt = Data->MinCnt = 0;
		}
	}

	return false;
}
#endif
/*********************************************************************************/
