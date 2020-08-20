/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : DigitalOutStepCtrl.h
*/
/*********************************************************************************/
#ifndef __DIGITALOUT_STEP_CTRL_H__
#define	__DIGITALOUT_STEP_CTRL_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define DIGITALOUT_STEP_CTRL_REVISION_DATE				20200317
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 03. 17.					- SortNode()함수에서 최소한 MaxStep까지는 정렬을 실행하도록 수정.
	Jeong Hyun Gu						SetUserSetStep()함수를 통해 설정 스텝을 변경할 때 정상적으로 정렬이 되지 않는 문제 수정.

	2020. 01. 08.					- SysTypedef.h 적용.
	Jeong Hyun Gu

	2019. 04. 15.					- SetUserSetStep()에서 SetStep이 변경 되었을 경우 tag_StepCtrl::ListOff노드의
	Jeong Hyun Gu						비트 값 초기화 추가.

	2019. 01. 28.					- SetUserSetStep()에서 SetStep이 변경 되었을 경우 tag_StepCtrl::CurStep 변수를
	Jeong Hyun Gu						0으로 초기화. (운전 중 스텝 변경 지원)

	2018. 10. 05.					- 모든 비트필드의 타입을 unsigned로 지정.
	Jeong Hyun Gu

	2018. 09. 14.					- SetUserSetStep()에서 전달받은 인수 SetStep가 tag_StepCtrl::MaxStep보다
	Jeong Hyun Gu						크지 않도록 제한.

	2017. 12. 22.					- InitStepCtrl()에서 InsertNewNodeLeft()의 동적할당 성공여부 확인 추가.
	Jeong Hyun Gu

	2017. 06. 28.					- AscendByNum() 함수에서 다음 노드에 에러가 있을 경우 정렬하지 않게 수정.
	Jeong Hyun Gu						(ALT_LIFO에서 경보 발생 시 오동작 수정)

	2017. 05. 29.					- SortNodeByRunTime(), SortAscendNode() 함수를 SortNode()함수로 병합.
	Jeong Hyun Gu					- 1호기 ENABLE, 2호기 DISABLE 상태이고 적산교번일 때 1호기 시간이 더 클 경우
													ON되지 않던 현상 수정.

	2017. 05. 10.					- SortAscendNode(), SortNodeByRunTime() 함수에서
	Jeong Hyun Gu						Tail 이전 노드가 정렬되지 않던 현상 수정.

	2017. 04. 11.					- 'CalcStepAlt()' 함수에서 GoalStep은 'SetStep' 이하로 제한.
	Jeong Hyun Gu					- 'SetUserSetStep()' 함수에서 'SetStep' 변경 시 모든 스텝 상태 비트 OFF.

	2017. 03. 22.					- "DigitalOutStepCtrl.h" 파일에서 private 함수 원형선언 삭제.
	Jeong Hyun Gu					- CalcStepAlt() 함수의 'GoalStep' 값을 'Step->MaxStep'과 같거나 작게 제한.

	2017. 01. 02.					- CheckStepDelay()함수에서 'StepDelay'가 0이면 에러처리 하던 부분 삭제.
	Jeong Hyun Gu

	2016. 11. 29.					- 적산교번 추가.
	Jeong Hyun Gu

	2016. 11. 09.					- revision valid check 추가.
	Jeong Hyun Gu

	2016. 10. 28.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define null		0
#define	true		1
#define	false		0

/*********************************************************************************/
/**Enum**/

typedef enum
{
	ALT_INIT = 0,
	ALT_RUN_TIME,
	ALT_LIFO,
	ALT_FIFO,
}enum_AlternateType;

typedef enum
{
	RUNTIME_SORT_ASCENT = 0,
	RUNTIME_SORT_DESCENT,
}enum_RunTimeSortDir;

/*********************************************************************************/
/**Struct**/

typedef struct
{
	tU8 Status				:		1;
	tU8 Command				:		1;
	tU8 Alarm					:		1;
}tag_EachStepBitField;

typedef struct
{
	const tU8 Num;
	tU32 RunTime;
	tag_EachStepBitField Bit;
}tag_EachStep;

typedef struct tag_EachStepNode
{
	tag_EachStep *EachStep;

	struct tag_EachStepNode *Next;
	struct tag_EachStepNode *Prev;
}tag_EachStepNode;

typedef struct
{
	tag_EachStepNode *Head;
	tag_EachStepNode *Tail;
}tag_StepLinkedList;

typedef struct
{
	tU8 Init						:		1;
}tag_StepCtrlBitField;

typedef struct
{
	tU8 MaxStep;
	tU8 SetStep;
	tU8 CurStep;

	tU16 StepDelayCnt;

	tag_EachStep *EachStep;

	tag_StepLinkedList ListOn;
	tag_StepLinkedList ListOff;
	tag_StepLinkedList ListStdby;

	enum_AlternateType AltType;
	tag_StepCtrlBitField Bit;
}tag_StepCtrl;

/*********************************************************************************/
/**Function**/

tU8 InitStepCtrl(tag_StepCtrl *Step, tU8 MaxStep);

void SetUserSetStep(tag_StepCtrl *Step, tU8 SetStep);
void SetStepAltType(tag_StepCtrl *Step, enum_AlternateType AltType);
void SetEachStepAlarm(tag_StepCtrl *Step, tU8 TargetStep, tU8 Alarm);
void SetEachStepRunTime(tag_StepCtrl *Step, tU8 TargetStep, tU16 RunTime);

void CalcStepAlt(tag_StepCtrl *Step, tU8 GoalStep, tU16 StepDelay);

tU8 GetEachStepStatus(tag_StepCtrl *Step, tU8 TargetStep);
tU8 GetEachStepAlarm(tag_StepCtrl *Step, tU8 TargetStep);

/*********************************************************************************/
#endif //__DIGITALOUT_STEP_CTRL_H__
