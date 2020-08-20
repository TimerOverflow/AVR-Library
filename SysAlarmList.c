/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysAlarmList.c
*/
/*********************************************************************************/
#include <stdlib.h>
#include "SysAlarmList.h"
/*********************************************************************************/
#if(SYS_ALARM_LIST_REVISION_DATE != 20200225)
#error wrong include file. (SysAlarmList.h)
#endif
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static tU8 CheckAllOfInit(tag_SysAlarmList *List)
{
	return (!List->Bit.InitGeneral || (List->Bit.TimeUse && !List->Bit.LinkGetTimeFunc)) ? false : true;
}
/*********************************************************************************/
static void AddListItem(tag_SysAlarmList *List, tU8 Alarm)
{
	tag_AlamListItem Item;
	
	Item.Alarm = Alarm;
	Item.Target = List->IndexOfTarget;
	Item.Status = ALARMLIST_OCCUR;
	
	if(List->Bit.TimeUse)
	{
		List->GetTime(&Item.OccurTime);
	}
	SysQueuePush(&List->Que, &Item);
}
/*********************************************************************************/
static void ModifyListItem(tag_SysAlarmList *List, tU8 Alarm)
{
	tag_AlamListItem Item;
	tU8 Move;
	
	for(Move = 0; Move < List->AddedListItem; Move++)
	{
		SysAlarmListGetItemFromNewest(List, &Item, Move);
		if((Item.Alarm == Alarm) && (Item.Target == List->IndexOfTarget))
		{
			Item.Status = ALARMLIST_RELEASE;
			if(List->Bit.TimeUse)
			{
				List->GetTime(&Item.ReleaseTime);
			}
			SysQueuePeek(&List->Que, &Item, QUE_PEEK_FROM_IN, Move, QUE_PEEK_WRITE);
		}
	}
}
/*********************************************************************************/
tU8 SysAlarmListGeneralInit(tag_SysAlarmList *List, tU8 MaxListItem, tU8 MaxTarget, tU8 TimeUse, tag_EepCommonConfig *EepConfig)
{
	if(List->Bit.InitComplete == true || (MaxTarget < 1))
	{
		return false;
	}
	
	List->PrevAlm = calloc(MaxTarget, sizeof(tU32));
	List->CurAlm = calloc(MaxTarget, sizeof(tU32 *));
	if((List->PrevAlm == null) || (List->CurAlm == null))
	{
		return false;
	}
	List->MaxTarget = MaxTarget;
	List->AddedTarget = 0;
	
	List->Bit.TimeUse = TimeUse;
	if(SysQueueGeneralInit(&List->Que, List->Bit.TimeUse ? sizeof(tag_AlamListItem) : sizeof(tag_AlamListItem) - (sizeof(tag_AlarmListTime) * 2), MaxListItem, QUE_TYPE_LIFO, true))
	{
		List->MaxListItem = MaxListItem;
		List->Bit.InitGeneral = true;
	}
	
	if((EepConfig != null) && (EepConfig->Bit.Init == true))
	{
		InitEepControl(&List->EepListItem, (const tU8 *) List->Que.Data, List->Que.SizeOfQue, EepConfig);
		InitEepControl(&List->EepListData, (const tU8 *) &List->Que.In, sizeof(tU16) * 3, EepConfig);
		InitEepControl(&List->EepPrevAlm, (const tU8 *) List->PrevAlm, sizeof(tU32) * MaxTarget, EepConfig);
		if(CheckEepromFirstExecuteSignature(EepConfig) == true)
		{
			SetEepWriteEnable(&List->EepListItem);
			SetEepWriteEnable(&List->EepListData);
			SetEepWriteEnable(&List->EepPrevAlm);
		}
		else
		{
			DoEepReadControl(&List->EepListItem);
			DoEepReadControl(&List->EepListData);
			DoEepReadControl(&List->EepPrevAlm);
		}
		List->Bit.EepromUse = true;
	}
	
	List->AddedListItem = List->Que.Cnt;
	if(List->AddedListItem > List->MaxListItem)
	{
		EraseEepCommonConfigSignature(EepConfig);
		//List->AddedListItem이 List->MaxListItem 보다 클 경우 비정상으로 판단 초기화 시도.
	}
	
	List->Bit.InitComplete = CheckAllOfInit(List);
	return List->Bit.InitGeneral;
}
/*********************************************************************************/
tU8 SysAlarmListAddTarget(tag_SysAlarmList *List, tU32 *CurAlm)
{
	if((List->Bit.InitComplete == false) || (List->AddedTarget >= List->MaxTarget))
	{
		return false;
	}
	
	List->CurAlm[List->AddedTarget] = CurAlm;
	List->AddedTarget++;
	
	return true;
}
/*********************************************************************************/
tU8 SysAlarmListLinkUserExceptionFunc(tag_SysAlarmList *List, void (*UserException)(struct tag_SysAlarmList *List))
{
	if(List->Bit.InitGeneral == false)
	{
		return false;
	}
	
	List->UserException = UserException;
	List->Bit.LinkUserExceptionFunc = true;
	return List->Bit.LinkUserExceptionFunc;
}
/*********************************************************************************/
tU8 SysAlarmListLinkGetTimeFunc(tag_SysAlarmList *List, void (*GetTime)(tag_AlarmListTime *Time))
{
	if((List->Bit.InitGeneral == false) || (List->Bit.TimeUse == false))
	{
		return false;
	}
	
	List->GetTime = GetTime;
	List->Bit.LinkGetTimeFunc = true;
	List->Bit.InitComplete = CheckAllOfInit(List);
	return List->Bit.LinkGetTimeFunc;
}
/*********************************************************************************/
void SysAlarmListProc(tag_SysAlarmList *List)
{
	const static tU8 NumberOfBit = 32;
	tU8 i;
	
	if(List->Bit.InitGeneral == false || List->AddedTarget == 0)
	{
		return;
	}
	
	if(List->PrevAlm[List->IndexOfTarget] != *List->CurAlm[List->IndexOfTarget])
	{
		for(i = 0; i < NumberOfBit; i++)
		{
			if((List->PrevAlm[List->IndexOfTarget] & (1UL << i)) != (*List->CurAlm[List->IndexOfTarget] & (1UL << i)))
			{
				if(*List->CurAlm[List->IndexOfTarget] & (1UL << i))
				{
					AddListItem(List, i);
				}
				else
				{
					ModifyListItem(List, i);
				}
				if(List->Bit.LinkUserExceptionFunc) List->UserException(List);
				if(List->Bit.EepromUse){ SetEepWriteEnable(&List->EepListItem); SetEepWriteEnable(&List->EepListData); SetEepWriteEnable(&List->EepPrevAlm); }
			}
		}
		List->PrevAlm[List->IndexOfTarget] = *List->CurAlm[List->IndexOfTarget];
	}
	
	if(++List->IndexOfTarget >= List->AddedTarget) List->IndexOfTarget = 0;
	List->AddedListItem = List->Que.Cnt;
	
	if(List->Bit.EepromUse)
	{
		if(DoEepWriteControl(&List->EepListItem) == true) return;
		if(DoEepWriteControl(&List->EepListData) == true) return;
		if(DoEepWriteControl(&List->EepPrevAlm) == true) return;
	}
}
/*********************************************************************************/
tU8 SysAlarmListGetItemFromNewest(tag_SysAlarmList *List, tag_AlamListItem *Item, tU8 Move)
{
	if(List->Bit.InitComplete == false || List->AddedListItem == 0)
	{
		return false;
	}
	
	return SysQueuePeek(&List->Que, Item, QUE_PEEK_FROM_IN, Move, QUE_PEEK_READ);
}
/*********************************************************************************/
tU8 SysAlarmListDelItem(tag_SysAlarmList *List, tag_AlarmListActionDirection Dir)
{
	enum_QueueType PrevType = List->Que.StoreType;
	tU8 Result;
	
	if(List->Bit.InitComplete == false || List->AddedListItem == 0)
	{
		return false;
	}
	
	List->Que.StoreType = Dir == ALARMLIST_ACT_FIFO ? QUE_TYPE_FIFO : QUE_TYPE_LIFO;
	Result = SysQueueDrop(&List->Que);
	List->Que.StoreType = PrevType;
	
	if(List->Bit.LinkUserExceptionFunc) List->UserException(List);
	if(List->Bit.EepromUse){ SetEepWriteEnable(&List->EepListItem); SetEepWriteEnable(&List->EepListData); SetEepWriteEnable(&List->EepPrevAlm); }
	
	return Result;
}
/*********************************************************************************/
void SysAlarmListDelAllItem(tag_SysAlarmList *List)
{
	while(SysAlarmListDelItem(List, ALARMLIST_ACT_FIFO));
}
/*********************************************************************************/
