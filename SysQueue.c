/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysQueue.c
*/
/*********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "SysQueue.h"
/*********************************************************************************/
#if(SYS_QUEUE_REVISION_DATE != 20191023)
#error wrong include file. (SysQueue.h)
#endif
/*********************************************************************************/
#define INC_IDX(ctr, end, start)	if (ctr < (end-1))	{ ctr++; }		\
									else				{ ctr = start; }	//!< Increments buffer index \b ctr rolling back to \b start when limit \b end is reached

#define DEC_IDX(ctr, end, start)	if (ctr > (start))	{ ctr--; }		\
									else				{ ctr = end-1; }	//!< Decrements buffer index \b ctr rolling back to \b end when limit \b start is reached
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static tU8 CheckAllOfInit(tag_Queue *Que)
{
	return Que->Bit.InitGeneral ? true : false;
}
/*********************************************************************************/
tU8 SysQueueGeneralInit(tag_Queue *Que, tU16 SizeOfRecord, tU16 NumberOfRecord, enum_QueueType StoreType, tU8 Overwrite)
{
	if(Que->Bit.InitComplete)
	{
		return false;
	}
	
	Que->Data = (tU8 *) calloc(NumberOfRecord, SizeOfRecord);
	if(Que->Data != null)
	{
		Que->SizeOfRecord = SizeOfRecord;
		Que->NumberOfRecord = NumberOfRecord;
		Que->Bit.Overwrite = Overwrite ? true : false;
		Que->SizeOfQue = SizeOfRecord * NumberOfRecord;
		Que->StoreType = StoreType;
		Que->Bit.InitGeneral = true;
	}
	
	Que->Bit.InitComplete = CheckAllOfInit(Que);
	return Que->Bit.InitGeneral;
}
/*********************************************************************************/
tU8 SysQueuePush(tag_Queue *Que, void *Record)
{
	tU8 *pStart;
	
	if((Que->Bit.InitComplete == false) || (Que->Bit.Overwrite == false && SYS_QUEUE_IS_FULL(Que)))
	{
		return false;
	}
	
	pStart = Que->Data + (Que->SizeOfRecord * Que->In);
	memcpy(pStart, Record, Que->SizeOfRecord);
	
	INC_IDX(Que->In, Que->NumberOfRecord, 0);
	if(SYS_QUEUE_IS_FULL(Que) == false)
	{
		Que->Cnt++;
	}
	else if(Que->Bit.Overwrite)
	{
		switch(Que->StoreType)
		{
			case	QUE_TYPE_FIFO	:	//as oldest record is overwritten, increment out
				INC_IDX(Que->Out, Que->NumberOfRecord, 0);	
			break;
			
			case	QUE_TYPE_LIFO	:	// Nothing to do in this case
			break;
			
			default	: return false;
		}
	}
	
	return true;
}
/*********************************************************************************/
tU8 SysQueuePop(tag_Queue *Que, void *Record)
{
	tU8 *pStart;
	
	if((Que->Bit.InitComplete == false) || SYS_QUEUE_IS_EMPTY(Que))
	{
		return false;
	}
	
	switch(Que->StoreType)
	{
		case	QUE_TYPE_FIFO	:
			pStart = Que->Data + (Que->SizeOfRecord * Que->Out);
			INC_IDX(Que->Out, Que->NumberOfRecord, 0);
		break;
		
		case	QUE_TYPE_LIFO	:
			DEC_IDX(Que->In, Que->NumberOfRecord, 0);
			pStart = Que->Data + (Que->SizeOfRecord * Que->In);
		break;
		
		default	:	return false; 
	}
	
	memcpy(Record, pStart, Que->SizeOfRecord);
	Que->Cnt--;

	return true;
}
/*********************************************************************************/
tU8 SysQueuePeek(tag_Queue *Que, void *Record, enum_PeekDirection Dir, tU16 Move, enum_PeekAction Action)
{
	tU8 *pStart, i;
	tU16 Ctr;
	
	if((Que->Bit.InitComplete == false) || SYS_QUEUE_IS_EMPTY(Que) || (Move >= Que->Cnt))
	{
		return false;
	}
	
	switch(Dir)
	{
		case	QUE_PEEK_FROM_IN	:
			Ctr = Que->In; Move += 1;
			for(i = 0; i < Move; i++) DEC_IDX(Ctr, Que->NumberOfRecord, 0);
		break;
	
		default	:
		case	QUE_PEEK_FROM_OUT	:
			Ctr = Que->Out;
			for(i = 0; i < Move; i++) INC_IDX(Ctr, Que->NumberOfRecord, 0);
		break;
	}
	
	pStart = Que->Data + (Que->SizeOfRecord * Ctr);
	if(Action == QUE_PEEK_READ)
	{
		memcpy(Record, pStart, Que->SizeOfRecord);
	}
	else
	{
		memcpy(pStart, Record, Que->SizeOfRecord);
	}
	
	return true;
}
/*********************************************************************************/
tU8 SysQueueDrop(tag_Queue *Que)
{
	if((Que->Bit.InitComplete == false) || SYS_QUEUE_IS_EMPTY(Que))
	{
		return false;
	}
	
	switch(Que->StoreType)
	{
		case	QUE_TYPE_FIFO	:	INC_IDX(Que->Out, Que->NumberOfRecord, 0);	break;
		case	QUE_TYPE_LIFO	: DEC_IDX(Que->In, Que->NumberOfRecord, 0);	break;
		default	:	return false;
	}
	
	Que->Cnt--;
	return true;
}
/*********************************************************************************/
