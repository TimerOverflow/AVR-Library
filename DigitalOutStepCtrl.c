/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : DigitalOutStepCtrl.c
*/
/*********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "DigitalOutStepCtrl.h"
/*********************************************************************************/
#if(DIGITALOUT_STEP_CTRL_REVISION_DATE != 20200317)
#error wrong include file. (DigitalOutStepCtrl.h)
#endif
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static void MoveToNodeRight(tag_EachStepNode *TargetNode, tag_EachStepNode *MovingNode)
{
	tag_EachStepNode *Right = TargetNode->Next;

	if(TargetNode == MovingNode)
	{
		return;
		//no reason for move.
	}

	MovingNode->Prev->Next = MovingNode->Next;
	MovingNode->Next->Prev = MovingNode->Prev;

	MovingNode->Next = Right;
	MovingNode->Prev = TargetNode;

	Right->Prev = MovingNode;
	TargetNode->Next = MovingNode;
}
/*********************************************************************************/
static void MoveToNodeLeft(tag_EachStepNode *TargetNode, tag_EachStepNode *MovingNode)
{
	MoveToNodeRight(TargetNode->Prev, MovingNode);
}
/*********************************************************************************/
static void TargetStepOff(tag_StepCtrl *Step, tag_EachStepNode *EachStepNode)
{
	if(EachStepNode == null)
	{
		return;
		/* error */
	}

	EachStepNode->EachStep->Bit.Status = false;
	EachStepNode->EachStep->Bit.Command = false;
	MoveToNodeLeft(Step->ListOff.Tail, EachStepNode);
}
/*********************************************************************************/
static tU8 InitLinkedList(tag_StepLinkedList *LinkedList)
{
	tU8 Init = true;

	LinkedList->Head = (tag_EachStepNode *) malloc(sizeof(tag_EachStepNode));
	if(LinkedList->Head == null)
	{
		Init = false;
	}

	LinkedList->Tail = (tag_EachStepNode *) malloc(sizeof(tag_EachStepNode));
	if(LinkedList->Tail == null)
	{
		Init = false;
	}

	if(Init == false)
	{
		free(LinkedList->Head);
		free(LinkedList->Tail);
		return false;
	}

	memset(LinkedList->Head, 0, sizeof(tag_EachStepNode));
	memset(LinkedList->Tail, 0, sizeof(tag_EachStepNode));

	LinkedList->Head->Next = LinkedList->Tail;
	LinkedList->Tail->Prev = LinkedList->Head;

	return Init;
}
/*********************************************************************************/
static tU8 CheckStepDelay(tag_StepCtrl *Step, tU16 StepDelay)
{
	tag_EachStepNode *Target;

	if((Step->Bit.Init == false) || (Step->CurStep == 0))
	{
		return false;
		/* error */
	}

	Target = Step->ListOn.Head->Next;

	while(Target != Step->ListOn.Tail)
	{
		if((Target->EachStep->Bit.Command == true) && (Target->EachStep->Bit.Status == false))
		{
			if(++Step->StepDelayCnt >= StepDelay)
			{
				Step->StepDelayCnt = 0;
				Target->EachStep->Bit.Status = true;
			}
			return true;
		}
		else
		{
			Target = Target->Next;
		}
	}

	return false;
}
/*********************************************************************************/
static void CheckAllEachStepAlarm(tag_StepCtrl *Step)
{
	tag_EachStepNode *Target;

	if((Step->Bit.Init == false) || (Step->CurStep == 0))
	{
		return;
		/* error */
	}

	Target = Step->ListOn.Head;

	while(Target->Next != Step->ListOn.Tail)
	{
		if(Target->Next->EachStep->Bit.Alarm == true)
		{
			Step->CurStep--;
			TargetStepOff(Step, Target->Next);
		}
		else
		{
			Target = Target->Next;
		}
	}
}
/*********************************************************************************/
static tU8 AscendByRunTime(tag_EachStep *Cur, tag_EachStep *Next)
{
	return ((Cur->RunTime > Next->RunTime) && (Next->Bit.Alarm == false)) ? true : false;
}
/*********************************************************************************/
static tU8 DecendByRunTime(tag_EachStep *Cur, tag_EachStep *Next)
{
	return Cur->RunTime < Next->RunTime ? true : false;
}
/*********************************************************************************/
static tU8 AscendByNum(tag_EachStep *Cur, tag_EachStep *Next)
{
	return ((Cur->Num > Next->Num) && (Next->Bit.Alarm == false)) ? true : false;
}
/*********************************************************************************/
static void SortNode(tag_StepLinkedList *TargetList, tU8 (*Compare)(tag_EachStep *cur, tag_EachStep *next), tU8 MaxStep)
{
	tag_EachStepNode *Node;
	tU8 i, Changed;
	
	for(i = 0; i < MaxStep; i++)
	{
		Node = TargetList->Head->Next;
		Changed = false;
		
		while(Node != TargetList->Tail)
		{
			if((Compare(Node->EachStep, Node->Next->EachStep) == true) && (Node->Next != TargetList->Tail))
			{
				MoveToNodeRight(Node->Next, Node);
				Changed = true;
			}
			Node = Node->Next;
		}

		if(Changed == false) break;
	}
}
/*********************************************************************************/
static void CheckRunTime(tag_StepCtrl *Step)
{
	if(Step->AltType == ALT_RUN_TIME)
	{
		SortNode(&Step->ListOn, DecendByRunTime, Step->MaxStep);
		SortNode(&Step->ListOff, AscendByRunTime, Step->MaxStep);
	}
}
/*********************************************************************************/
static tU8 EachStepOn(tag_StepCtrl *Step)
{
	tU8 Result = false;

	if(Step->ListOff.Head->Next->EachStep->Bit.Alarm == true)
	{
		MoveToNodeLeft(Step->ListOff.Tail, Step->ListOff.Head->Next);
		Result = false;
	}
	else
	{
		switch(Step->AltType)
		{
			case	ALT_FIFO	:
			case	ALT_RUN_TIME	:

				Step->ListOff.Head->Next->EachStep->Bit.Command = true;
				MoveToNodeLeft(Step->ListOn.Tail, Step->ListOff.Head->Next);
				Result = true;
			break;

			case	ALT_LIFO	:

				SortNode(&Step->ListOff, AscendByNum, Step->MaxStep);
				Step->ListOff.Head->Next->EachStep->Bit.Command = true;
				MoveToNodeRight(Step->ListOn.Head, Step->ListOff.Head->Next);
				Result = true;
			break;
		}
	}

	return Result;
}
/*********************************************************************************/
static tU8 EachStepOff(tag_StepCtrl *Step)
{
	if(Step->ListOn.Head->Next == null)
	{
		return false;
	}

	TargetStepOff(Step, Step->ListOn.Head->Next);

	return true;
}
/*********************************************************************************/
static tag_EachStepNode* InsertNewNodeRight(tag_EachStepNode *Target, tag_EachStep *EachStep)
{
	tag_EachStepNode *New;
	tag_EachStepNode *Right;

	New = (tag_EachStepNode *) malloc(sizeof(tag_EachStepNode));
	New->EachStep = EachStep;

	Right = Target->Next;
	New->Next = Right;
	New->Prev = Target;
	Target->Next = New;

	if(Right != null)
	{
		Right->Prev = New;
	}

	return New;
}
/*********************************************************************************/
static tag_EachStepNode* InsertNewNodeLeft(tag_EachStepNode *Target, tag_EachStep *EachStep)
{
	tag_EachStepNode *Left;

	Left = Target->Prev;
	if(Left != null)
	{
		return InsertNewNodeRight(Left, EachStep);
	}

	return null;
}
/*********************************************************************************/
static void DeleteNode(tag_EachStepNode *Target)
{
	tag_EachStepNode *Left;
	tag_EachStepNode *Right;

	Left = Target->Prev;
	Right = Target->Next;

	Left->Next = Right;
	if(Right != null)
	{
		Right->Prev = Left;
	}

	free(Target);
}
/*********************************************************************************/
tU8 InitStepCtrl(tag_StepCtrl *Step, tU8 MaxStep)
{
	tU8 i;
	tU8 *pNumber;

	memset(Step, 0, sizeof(tag_StepCtrl));

	Step->EachStep = (tag_EachStep *) calloc(MaxStep, sizeof(tag_EachStep));
	if(Step->EachStep == null)
	{
		return false;
	}

	if(InitLinkedList(&Step->ListOn) == false) return false;
	if(InitLinkedList(&Step->ListOff) == false) return false;
	if(InitLinkedList(&Step->ListStdby) == false) return false;

	for(i = 0; i < MaxStep; i++)
	{
		pNumber = (tU8 *) &Step->EachStep[i].Num;
		*pNumber = i + 1;
		/* numbering of each step info */

		if(InsertNewNodeLeft(Step->ListStdby.Tail, &Step->EachStep[i]) == null) return false;
	}

	Step->MaxStep = MaxStep;
	Step->AltType = ALT_INIT;
	Step->StepDelayCnt = 0;

	Step->Bit.Init = true;

	return Step->Bit.Init;
}
/*********************************************************************************/
void SetUserSetStep(tag_StepCtrl *Step, tU8 SetStep)
{
	tU8 i;

	if(Step->Bit.Init == false)
	{
		return;
		/* error */
	}

	if(SetStep > Step->MaxStep)
	{
		SetStep = Step->MaxStep;
	}

	if(Step->SetStep != SetStep)
	{
		Step->SetStep = SetStep;
		Step->CurStep = 0;

		while(Step->ListOn.Head->Next != Step->ListOn.Tail)
		{
			Step->ListOn.Head->Next->EachStep->Bit.Status = false;
			Step->ListOn.Head->Next->EachStep->Bit.Command = false;
			Step->ListOn.Head->Next->EachStep->Bit.Alarm = false;
			MoveToNodeLeft(Step->ListStdby.Tail, Step->ListOn.Head->Next);
		}

		while(Step->ListOff.Head->Next != Step->ListOff.Tail)
		{
			Step->ListOff.Head->Next->EachStep->Bit.Status = false;
			Step->ListOff.Head->Next->EachStep->Bit.Command = false;
			Step->ListOff.Head->Next->EachStep->Bit.Alarm = false;
			MoveToNodeLeft(Step->ListStdby.Tail, Step->ListOff.Head->Next);
		}

		SortNode(&Step->ListStdby, AscendByNum, Step->MaxStep);

		for(i = 0; i < SetStep; i++)
		{
			MoveToNodeLeft(Step->ListOff.Tail, Step->ListStdby.Head->Next);
		}
	}
}
/*********************************************************************************/
void SetStepAltType(tag_StepCtrl *Step, enum_AlternateType AltType)
{
	if(Step->Bit.Init == false)
	{
		return;
		/* error */
	}

	Step->AltType = AltType;
}
/*********************************************************************************/
void SetEachStepAlarm(tag_StepCtrl *Step, tU8 TargetStep, tU8 Alarm)
{
	if((Step->Bit.Init == false) || (TargetStep > Step->MaxStep))
	{
		return;
		/* error */
	}

	Step->EachStep[TargetStep - 1].Bit.Alarm = Alarm;
}
/*********************************************************************************/
void SetEachStepRunTime(tag_StepCtrl *Step, tU8 TargetStep, tU16 RunTime)
{
	if((Step->Bit.Init == false) || (TargetStep > Step->MaxStep))
	{
		return;
	}

	Step->EachStep[TargetStep - 1].RunTime = RunTime;
}
/*********************************************************************************/
void CalcStepAlt(tag_StepCtrl *Step, tU8 GoalStep, tU16 StepDelay)
{
	if((Step->Bit.Init == false) || (Step->AltType == ALT_INIT))
	{
		return;
		/* error */
	}

	if(GoalStep > Step->SetStep)
	{
		GoalStep = Step->SetStep;
	}

	CheckAllEachStepAlarm(Step);
	CheckRunTime(Step);

	if((CheckStepDelay(Step, StepDelay) == false) && (Step->CurStep < GoalStep))
	{
		if(EachStepOn(Step) == true)
		{
			Step->CurStep++;
		}
	}
	else if(Step->CurStep > GoalStep)
	{
		if(EachStepOff(Step) == true)
		{
			Step->CurStep--;
		}
	}
}
/*********************************************************************************/
tU8 GetEachStepStatus(tag_StepCtrl *Step, tU8 TargetStep)
{
	if((Step->Bit.Init == false) || (TargetStep > Step->MaxStep))
	{
		return -1;
		/* error */
	}

	return Step->EachStep[TargetStep - 1].Bit.Status;
}
/*********************************************************************************/
tU8 GetEachStepAlarm(tag_StepCtrl *Step, tU8 TargetStep)
{
	if((Step->Bit.Init == false) || (TargetStep > Step->MaxStep))
	{
		return -1;
		/* error */
	}

	return Step->EachStep[TargetStep - 1].Bit.Alarm;
}
/*********************************************************************************/
