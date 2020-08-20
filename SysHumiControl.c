/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysHumiControl.c
*/
/*********************************************************************************/
#include "SysHumiControl.h"
/*********************************************************************************/
#if(SYS_HUMI_CONTROL_REVISION_DATE != 20200729)
#error wrong include file. (SysHumiControl.h)
#endif
/*********************************************************************************/
/** Global variable **/

#if(__SYS_HUMI_CONTROL_TARGET_IAR_AVR__ == true)
#include <ina90.h>
flash tS16 EtypeCurrentTable[][46] =
#else
tS16 EtypeCurrentTable[][46] =
#endif
{
	{19,19,38,57,76,95,114,133,152,171,190,209,228,247,266,285,304,323,342,361,380,399,418,437,456,475,494,513,532,551,570,589,608,627,646,665,665,665,665,665,665,665,665,665,665,665},
	{11,11,22,33,44,55, 66, 77, 88, 99,110,121,132,143,154,165,176,187,198,209,220,231,242,253,264,275,286,297,308,319,330,341,352,363,374,385,396,407,418,429,440,451,462,473,484,495},
	{9, 9,19,28,38,47, 57, 66, 76, 85, 95,104,114,123,133,142,152,161,171,180,190,199,209,218,228,237,247,256,266,275,285,294,304,313,323,332,342,351,361,370,380,389,399,409,419,428}
};

/*********************************************************************************/
static void Besu_Time_Setting_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->In.Set_seoljeongyongryang < 10) Humi->besu10sectime = 5;
	else if(Humi->In.Set_seoljeongyongryang < 13) Humi->besu10sectime = 10;
	else if(Humi->In.Set_seoljeongyongryang < 20) Humi->besu10sectime = 12;
	else if(Humi->In.Set_seoljeongyongryang < 30) Humi->besu10sectime = 15;
	else Humi->besu10sectime = 15;

	if(Humi->besuerrchat == 0) Humi->autosavecurrent = Humi->In.etypecurrentvalue;
}
/*********************************************************************************/
static void Over_Current_Check_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.6)
	{
		if(Humi->overctxsegicnt >= 3)
		{
			Humi->overctxsegicnt = 0;
			if(Humi->overcurrentchat >= 5){ Humi->overcurrentchat = 0; Humi->Out.Bit.Alm_OverCurrent = true; }
			else{ Humi->overcurrentchat++; Humi->Bit.besu10sectimeflag = true; Besu_Time_Setting_Proc(Humi); Humi->besu10sectime *= 2; }
		}
		else if(Humi->Bit.Flag_Sec) Humi->overctxsegicnt++;
	}
	else if(Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.4)
	{
		if(Humi->overctxsegicnt >= 3)
		{
			Humi->overctxsegicnt = 0;
			if(Humi->overcurrentchat >= 5){ Humi->overcurrentchat = 0; Humi->Out.Bit.Alm_WaterDrain = true; }
			else{ Humi->overcurrentchat++; Humi->Bit.besu10sectimeflag = true; Besu_Time_Setting_Proc(Humi); }
		}
		else if(Humi->Bit.Flag_Sec) Humi->overctxsegicnt++;
	}
	else if(Humi->In.etypecurrentvalue <= (float) Humi->TargetCurrent)
	{
		Humi->Out.Bit.Alm_OverCurrent = false; Humi->overcurrentchat = Humi->overctxsegicnt = 0;
		Humi->Out.Bit.Alm_WaterDrain = false;
	}
}
/*********************************************************************************/
static void Auto_Besu_Time_Control_Proc(tag_SysHumiEtype *Humi)
{
	Humi->Out.Bit.Do_WaterSupply = false; Humi->backupcurrent = 0; Humi->xsegidelaytime = 5;

	if(Humi->besu10sectime == 0)
	{
		Humi->Bit.besu10sectimeflag = false;
		Humi->Out.Bit.Do_WaterDrain = false;
	}
	else
	{
		if(Humi->Bit.Flag_Sec) Humi->besu10sectime--;
		Humi->Out.Bit.Do_WaterDrain = true;
	}
}
/*********************************************************************************/
static void Xsegi_Kill_Control_Proc(tag_SysHumiEtype *Humi)
{
	static tU8 xsegikillcount;

	if(Humi->xsegidelaytime)
	{
		if(Humi->Bit.Flag_Sec) Humi->xsegidelaytime--;
		Humi->backupcurrent = Humi->In.etypecurrentvalue;
		return;
	}

	if(Humi->backupcurrent)
	{
		if((Humi->In.etypecurrentvalue - Humi->backupcurrent) <= -70) Humi->Bit.xsegikillflag = true;
	}

	if(Humi->Bit.xsegikillflag)
	{
		Humi->Out.Bit.Do_WaterSupply = false;

		if(Humi->Bit.Flag_Twink)
		{
			Humi->Out.Bit.Do_WaterSupply = true; Humi->Bit.xsegidspflag = true;
		}
		else
		{
			Humi->Out.Bit.Do_WaterSupply = false;
			if(Humi->Bit.xsegidspflag) xsegikillcount++;
			Humi->Bit.xsegidspflag = false;
			if(xsegikillcount >= 4){ xsegikillcount = 0; Humi->Bit.xsegikillflag = false; }
		}
	}

	Humi->backupcurrent = Humi->In.etypecurrentvalue;
}
/*********************************************************************************/
static void Low_Current_Check_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->xsegichecktime) return;

	if(Humi->In.etypecurrentvalue <= 3)
	{
		if(Humi->currentlacktime >= 5)
		{
			if(Humi->lowcurrentchktime >= 3) Humi->Out.Bit.Alm_CurrentSen = true;
			else Humi->lowcurrentchktime++;
		}
		else if(Humi->Bit.Flag_Min) Humi->currentlacktime++;
	}
	else
	{
		Humi->currentlacktime = 0; Humi->lowcurrentchktime = 0;
	}
}
/*********************************************************************************/
static void BeSu10Min_Cotrol_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->besu10time)
	{
		Humi->mansusavecurrent = 0; Humi->currentlacktime = 0;
		Humi->Out.Bit.Do_Humi = false; Humi->etypestepcount = 0;
		Humi->Out.Bit.Do_WaterDrain = true;
		if(Humi->Bit.Flag_Min && --Humi->besu10time == 0)
		{
			Humi->Bit.besu10mintimeflag = false; Humi->etype72hour = 0;
			Humi->Bit.besuendchkflag = true;
			Humi->Out.Bit.Do_WaterDrain = false;
		}
	}
}
/*********************************************************************************/
static void Gaseup_Test_Mode_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->InOut.gaseupgistatus & 0x01)
	{
		Humi->InOut.gaseupgistatus = 0;
		Humi->Out.Bit.Do_WaterSupply = Humi->Out.Bit.Do_WaterDrain = false;
		Humi->Out.Bit.Alm_WaterDrain = Humi->Out.Bit.Alm_CurrentSen = Humi->Out.Bit.Alm_OverCurrent = false;
		Humi->ManualCtrlCnt = 0;
	}
	else if(Humi->InOut.gaseupgistatus & 0x02)
	{
		Humi->Out.Bit.Do_WaterDrain = false;
		if(Humi->Out.Bit.Do_WaterSupply == false){ Humi->ManualCtrlCnt = 30; Humi->Out.Bit.Do_WaterSupply = true; }
		if((Humi->ManualCtrlCnt == 0) || Humi->Bit.mansucheckflag){ Humi->InOut.gaseupgistatus = 0; Humi->Out.Bit.Do_WaterSupply = false; }
		else if(Humi->Bit.Flag_Sec) Humi->ManualCtrlCnt--;
	}
	else if(Humi->InOut.gaseupgistatus & 0x04)
	{
		Humi->Out.Bit.Do_WaterSupply = false;
		if(Humi->Out.Bit.Do_WaterDrain == false){ Humi->ManualCtrlCnt = 180; Humi->Out.Bit.Do_WaterDrain = true; }
		if(Humi->ManualCtrlCnt == 0){ Humi->InOut.gaseupgistatus = 0; Humi->Out.Bit.Do_WaterDrain = false; }
		else if(Humi->Bit.Flag_Sec) Humi->ManualCtrlCnt--;
	}
	else
	{
		Humi->ManualCtrlCnt = 0;
		Humi->Out.Bit.Do_Humi = Humi->Out.Bit.Do_WaterSupply = Humi->Out.Bit.Do_WaterDrain = false;
	}

	Humi->etypestepcount = 0;
}
/*********************************************************************************/
static void E_Type_Step_Zero_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->Bit.Flag_Sec)
	{
		Humi->etypechecktime++;
		if(Humi->etypechecktime & 0x01 && Humi->etypechecktime >= 3)
		{
			if(Humi->mansusavecurrent * 0.9 > Humi->In.etypecurrentvalue)
			{
				if(Humi->backchecktime >= 3) Humi->etypestepcount = 1;
				else Humi->backchecktime++;
			}
			else Humi->backchecktime = 0;

			if(Humi->mansusavecurrent < 3 && Humi->In.etypecurrentvalue < 3 && Humi->etypechecktime >= 10)
			{
				Humi->etypestepcount = 1; Humi->mansusavecurrent = 3;
			}

			if(Humi->In.etypecurrentvalue >= Humi->TargetCurrent * 1.3)
			{
				Humi->etypestepcount = 1; Humi->mansusavecurrent = 3;
			}
		}
		if(Humi->In.etypecurrentvalue >= Humi->mansusavecurrent) Humi->mansusavecurrent = Humi->In.etypecurrentvalue;
		Humi->backupcurrent = Humi->In.etypecurrentvalue;
	}
}
/*********************************************************************************/
static void First_Water_Supply_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->mansusavecurrent < Humi->In.etypecurrentvalue) Humi->mansusavecurrent = Humi->In.etypecurrentvalue;

	if(Humi->Bit.mansucheckflag)
	{
		Humi->Out.Bit.Do_WaterSupply = false;
		Low_Current_Check_Control_Proc(Humi);
		Humi->geupsu20mintime = 0;
		if(Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.1) Humi->etypestepcount = 2;
	}
	else
	{
		if(Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.1){ Humi->etypestepcount = 2; Humi->Out.Bit.Do_WaterSupply = false; }
		else{ Humi->Out.Bit.Do_WaterSupply = true; }
	}

	if(Humi->geupsu20mintime >= 20)
	{
		if(Humi->In.etypecurrentvalue <= 5 && Humi->In.Bit.Di_WaterLevelHigh == false)
		{
			Humi->Out.Bit.Alm_WaterNeed = true; Humi->watercheckerrtime = 0;
		}
		else Humi->geupsu20mintime = 0;
	}
	else if(Humi->In.etypecurrentvalue <= 5 || (Humi->In.Bit.Di_WaterLevelHigh == false))
	{
		if(Humi->Bit.Flag_Min) Humi->geupsu20mintime++;
	}
	else
	{
		Humi->geupsu20mintime = 0;
	}

	Xsegi_Kill_Control_Proc(Humi);
	Humi->backchecktime = Humi->etypechecktime = 0;
}
/*********************************************************************************/
static void Normal_Water_Supply_Control_Proc(tag_SysHumiEtype *Humi)
{
	if(Humi->Bit.besu10sectimeflag)
	{
		if(Humi->TargetDrainTime == 0){ Humi->Bit.besu10sectimeflag = false; }
		else{ Auto_Besu_Time_Control_Proc(Humi); }
	}
	else
	{
		Humi->besuerrchat = 0;
		if((Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.3) && Humi->xsegichecktime == 0)
		{
			Humi->Bit.besu10sectimeflag = true; Besu_Time_Setting_Proc(Humi);
			Humi->autobesutime = 0; Humi->Out.Bit.Do_WaterSupply = false;
		}
		else if(Humi->In.etypecurrentvalue >= (float) Humi->TargetCurrent * 1.1)
		{
			if(Humi->Out.Bit.Do_WaterSupply){ Humi->autobesutime = 0; Humi->Bit.autobesuchkflag = true; }
			Humi->Out.Bit.Do_WaterSupply = false;
		}
		else if((Humi->In.etypecurrentvalue <= (float) Humi->TargetCurrent * 0.9) && Humi->xsegichecktime == 0)
		{
			if(Humi->Out.Bit.Do_WaterSupply == false)
			{
				if(Humi->autobesutime < Humi->TargetDrainTime && Humi->Bit.autobesuchkflag)
				{
					Humi->Bit.besu10sectimeflag = true; Besu_Time_Setting_Proc(Humi);
				}
				Humi->autobesutime = 0; Humi->Bit.autobesuchkflag = false;
			}
			if(Humi->Bit.mansucheckflag == false && Humi->Bit.besu10sectimeflag == false) Humi->Out.Bit.Do_WaterSupply = true;
			else Humi->Out.Bit.Do_WaterSupply = false;
		}

		if(Humi->Out.Bit.Do_WaterSupply == false)
		{
			if(Humi->Bit.Flag_Sec && Humi->autobesutime < 1200) Humi->autobesutime++;
		}
		else if(Humi->Bit.mansucheckflag) Humi->Out.Bit.Do_WaterSupply = false;

		Over_Current_Check_Control_Proc(Humi);
		if(Humi->Bit.mansucheckflag) Low_Current_Check_Control_Proc(Humi);
		Xsegi_Kill_Control_Proc(Humi);
	}

	Humi->mansusavecurrent = 0;
}
/*********************************************************************************/
static void TimeFlagControl(tag_SysHumiEtype *Humi)
{
	Humi->Bit.Flag_Sec = Humi->Bit.Flag_Min = Humi->Bit.Flag_Hour = false;
	
	if(Humi->In.Bit.Flag_msec100 && ++Humi->TimeTwinkCnt >= 5){ Humi->TimeTwinkCnt = 0; Humi->Bit.Flag_Twink = ~Humi->Bit.Flag_Twink; }
	
	if(Humi->In.Bit.Flag_msec100 && ++Humi->TimeSecCnt >= 10){ Humi->TimeSecCnt = 0; Humi->Bit.Flag_Sec = true; }
	if(Humi->Bit.Flag_Sec && ++Humi->TimeMinCnt >= 60){ Humi->TimeMinCnt = 0; Humi->Bit.Flag_Min = true; }
	if(Humi->Bit.Flag_Min && ++Humi->TimeHourCnt >= 60){ Humi->TimeHourCnt = 0; Humi->Bit.Flag_Hour = true; }
}
/*********************************************************************************/
void SysHumiEtypeProc(tag_SysHumiEtype *Humi, tU8 Run)
{
	static tU8 mansuchktime;
	static tS16 etypeofftime;
	tU8 seoljeongjeonab;

	TimeFlagControl(Humi);

	if(Humi->In.Bit.Di_WaterLevelHigh)
	{
		if(mansuchktime >= 7){ Humi->Bit.mansucheckflag = true; Humi->geupsu20mintime = 0; Humi->Out.Bit.Alm_WaterNeed = false; }
		else if(Humi->Bit.Flag_Sec) mansuchktime++;
	}
	else
	{
		if(mansuchktime == 0) Humi->Bit.mansucheckflag = false;
		else if(Humi->Bit.Flag_Sec) mansuchktime--;
	}

	if(Humi->In.Bit.Flag_TestMode)
	{
		Gaseup_Test_Mode_Control_Proc(Humi);
		return;
	}
	else
	{
		if(Humi->InOut.gaseupgistatus)
		{
			Humi->InOut.gaseupgistatus = 0; Humi->ManualCtrlCnt = 0;
			Humi->Out.Bit.Do_WaterSupply = Humi->Out.Bit.Do_WaterDrain = false;
		}
	}

	if(Humi->Out.Bit.Alm_WaterNeed)
	{
		if(Humi->watercheckerrtime >= 120)
		{
			Humi->watercheckerrtime = 0; Humi->Out.Bit.Alm_WaterNeed = false;
			Humi->geupsu20mintime = 0; Humi->etypestepcount = 0;
		}
		else if(Humi->Bit.Flag_Min) Humi->watercheckerrtime++;
	}

	if(Humi->In.Bit.Flag_HumiOff || Humi->Out.Bit.Alm_OverCurrent || Humi->Out.Bit.Alm_CurrentSen || Humi->Out.Bit.Alm_WaterNeed ||
		Humi->Out.Bit.Alm_WaterDrain)
	{
		Humi->mansusavecurrent = Humi->xsegidelaytime = Humi->xsegichecktime = Humi->besu10sectime = 0;
		Humi->Out.Bit.Status_Run = Humi->Bit.besu10sectimeflag = Humi->Out.Bit.Do_WaterSupply = Humi->Out.Bit.Do_Humi = false;
		if(Humi->Bit.besu10mintimeflag) BeSu10Min_Cotrol_Proc(Humi);
		else Humi->Out.Bit.Do_WaterDrain = false;
	}
	else
	{
		if(Humi->In.Set_seoljeongjeonab > 2) seoljeongjeonab = 2;
		else seoljeongjeonab = Humi->In.Set_seoljeongjeonab;

		Humi->TargetCurrent = EtypeCurrentTable[seoljeongjeonab][Humi->In.Set_seoljeongyongryang];
		Humi->TargetDrainTime = Humi->In.Set_baesuhoesu * 20;

		if(Humi->Bit.besu10mintimeflag)
		{
			if(Humi->TargetDrainTime == 0){ Humi->Bit.besu10mintimeflag = false; }
			else{ BeSu10Min_Cotrol_Proc(Humi); Humi->Out.Bit.Status_Run = false; }
		}
		else
		{
			Humi->Out.Bit.Status_Run = Run;
			if(Humi->Out.Bit.Status_Run)
			{
				Humi->etype72hour = 0;
				Humi->Bit.besuendchkflag = false;

				if(Humi->Bit.besu10sectimeflag == false){ Humi->Out.Bit.Do_Humi = true; }
				else{ Humi->Out.Bit.Do_Humi = false; Humi->xsegichecktime = 5; }

				if(Humi->Bit.Flag_Sec)
				{
					if(Humi->xsegichecktime) Humi->xsegichecktime--;
				}

				if(Humi->etypestepcount == 0) E_Type_Step_Zero_Control_Proc(Humi);
				else if(Humi->etypestepcount == 1) First_Water_Supply_Control_Proc(Humi);
				else Normal_Water_Supply_Control_Proc(Humi);
			}
			else
			{
				Humi->Out.Bit.Do_WaterSupply = false; Humi->xsegidelaytime = 10; Humi->xsegichecktime = 5;
				Humi->Out.Bit.Do_WaterDrain = false; Humi->autobesutime = 0; Humi->Out.Bit.Do_Humi = false;
			}
		}
	}

	if(Humi->Out.Bit.Status_Run == false)
	{
		if(etypeofftime >= 40)
		{
			Humi->etypestepcount = 0; Humi->etypechecktime = 0; Humi->backchecktime = 0;
			Humi->Bit.besu10sectimeflag = false; Humi->besu10sectime = 0;
		}else if(Humi->Bit.Flag_Min) etypeofftime++;

		if(Humi->etype72hour >= 72)
		{
			if(!Humi->Bit.besu10mintimeflag && !Humi->Bit.besuendchkflag)
			{
				Humi->Bit.besu10mintimeflag = true; Humi->besu10time = 10; Humi->etype72hour = 0;
			}
		}else if(Humi->Bit.Flag_Hour) Humi->etype72hour++;
	}else etypeofftime = 0;
}
/*********************************************************************************/
void SysHumiEtypeAlmClear(tag_SysHumiEtype *Humi)
{
	Humi->Out.Bit.Alm_WaterDrain = Humi->Out.Bit.Alm_WaterNeed = false;
	Humi->Out.Bit.Alm_OverCurrent = Humi->Out.Bit.Alm_CurrentSen = false;

	Humi->lowcurrentchktime = Humi->currentlacktime = Humi->besuerrchat = 0;
	Humi->geupsu20mintime = Humi->overctxsegicnt = Humi->overcurrentchat = 0;
}
/*********************************************************************************/
