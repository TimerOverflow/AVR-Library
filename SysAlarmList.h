/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysAlarmList.h
*/
/*********************************************************************************/
#ifndef __SYS_ALARM_LIST_H__
#define	__SYS_ALARM_LIST_H__
/*********************************************************************************/
#include "SysTypedef.h"
#include "SysQueue.h"
#include "SysEeprom.h"
/*********************************************************************************/
#define SYS_ALARM_LIST_REVISION_DATE		20200225
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 02. 25.					- 시간을 사용 하지 않으면 SysAlarmListAddTarget() 함수를 호출해도 타겟이
	Jeong Hyun Gu						추가 되지 않는 문제.

	2019. 11. 27.					- tag_SysAlarmList::AddedListItem > tag_SysAlarmList::MaxListItem 조건이면
	Jeong Hyun Gu						EraseEepCommonConfigSignature() 호출. 비정상 조건으로 판단 초기화 시도.

	2019. 10. 23.					- SysQueue(20191023) 이상 버전 사용해야 함.
	Jeong Hyun Gu					- 이제 에러 해제 여부 및 해제시간도 기록함.
												- 'tag_AlamListItemAndTime' -> 'tag_AlamListItem'으로 변경.
												-	SysAlarmListGetItemFromNewest()의 인자 Item의 타입
													(void *) -> (tag_AlamListItem *)으로 변경.

	2019. 09. 19.					- 외부 모듈 'SysEeprom' 내장. 이제 별도로 어플리케이션에서 
	Jeong Hyun Gu						알람리스트 관련 Eeprom을 핸들링 하지 않고, 'SysAlarmList' 모듈
													내에서 자동으로 관리.

	2019. 07. 23.					- SysTypedef.h 인클루드 추가.
	Jeong Hyun Gu					- 변수 선언 타입 변경.
												-	SysAlarmListDelAllItem() 추가.

	2019. 04. 10.					- 복수의 에러정보를 감시할 수 있도록 수정.
	Jeong Hyun Gu					- SysAlarmListAddTarget() 추가.

	2019. 03. 19.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define	false				0
#define	true				1
#define null				0

/*********************************************************************************/
/**Enum**/

typedef enum
{
	ALARMLIST_ACT_FIFO = 0,
	ALARMLIST_ACT_LIFO,
}tag_AlarmListActionDirection;

typedef enum
{
	ALARMLIST_OCCUR = 0,
	ALARMLIST_RELEASE,
}tag_AlarmStatus;

/*********************************************************************************/
/**Struct**/

typedef struct
{
	tU8 Year, Month, Date;
	tU8 Hour, Min, Sec;
}tag_AlarmListTime;

typedef struct
{
	tU8 Alarm;
	tU8 Target;
	tag_AlarmStatus Status;
	tag_AlarmListTime OccurTime, ReleaseTime;
}tag_AlamListItem;

typedef struct tag_SysAlarmList
{
	struct
	{
		tU8	InitGeneral						:		1;
		tU8	InitComplete					:		1;
		tU8	LinkUserExceptionFunc	:		1;
		tU8	LinkGetTimeFunc				:		1;
		tU8	TimeUse								:		1;
		tU8 EepromUse							:		1;
	}Bit;
	
	void (*UserException)(struct tag_SysAlarmList *List);
	void (*GetTime)(tag_AlarmListTime *Time);
	
	tag_Queue Que;
	
	tU32 *PrevAlm;
	tU32 **CurAlm;
	
	tU8 AddedListItem;
	tU8 MaxListItem;
	
	tU8 AddedTarget;
	tU8 MaxTarget;
	tU8 IndexOfTarget;
	
	tag_EepControl EepListItem, EepListData, EepPrevAlm;
}tag_SysAlarmList;

/*********************************************************************************/
/**Function**/

tU8 SysAlarmListGeneralInit(tag_SysAlarmList *List, tU8 MaxListItem, tU8 MaxTarget, tU8 TimeUse, tag_EepCommonConfig *EepConfig);
tU8 SysAlarmListAddTarget(tag_SysAlarmList *List, tU32 *CurAlm);
tU8 SysAlarmListLinkUserExceptionFunc(tag_SysAlarmList *List, void (*UserException)(struct tag_SysAlarmList *List));
tU8 SysAlarmListLinkGetTimeFunc(tag_SysAlarmList *List, void (*GetTime)(tag_AlarmListTime *Time));
void SysAlarmListProc(tag_SysAlarmList *List);
tU8 SysAlarmListGetItemFromNewest(tag_SysAlarmList *List, tag_AlamListItem *Item, tU8 Move);
tU8 SysAlarmListDelItem(tag_SysAlarmList *List, tag_AlarmListActionDirection Dir);
void SysAlarmListDelAllItem(tag_SysAlarmList *List);

/*********************************************************************************/
#endif //__SYS_ALARM_LIST_H__
