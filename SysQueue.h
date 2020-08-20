/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysQueue.h
*/
/*********************************************************************************/
#ifndef __SYS_QUEUE_H__
#define	__SYS_QUEUE_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define SYS_QUEUE_REVISION_DATE		20191023
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2019. 10. 23.					- SysQueuePeek() 함수에 'enum_PeekAction Action'인자 추가.
	Jeong Hyun Gu						이제 추가된 record의 데이터 변경 가능.

	2019. 07. 23.					- SysTypedef.h 인클루드 추가.
	Jeong Hyun Gu					- 변수 선언 타입 변경.

	2019. 03. 21.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define	false				0
#define	true				1
#define null				0

#define SYS_QUEUE_IS_FULL(__QUE__)		(__QUE__->Cnt == __QUE__->NumberOfRecord ? true : false)
#define SYS_QUEUE_IS_EMPTY(__QUE__)		(__QUE__->Cnt == 0 ? true : false)

/*********************************************************************************/
/**Enum**/

typedef enum
{
	QUE_TYPE_LIFO = 0,
	QUE_TYPE_FIFO,
}enum_QueueType;

typedef enum
{
	QUE_PEEK_FROM_IN = 0,
	QUE_PEEK_FROM_OUT,
}enum_PeekDirection;

typedef enum
{
	QUE_PEEK_READ = 0,
	QUE_PEEK_WRITE,
}enum_PeekAction;

/*********************************************************************************/
/**Struct**/

typedef struct
{
	struct
	{
		tU8 InitGeneral				:			1;
		tU8 InitComplete			:			1;
		tU8 Overwrite					:			1;
	}Bit;
	
	enum_QueueType StoreType;
	
	tU8 *Data;
	
	tU16 NumberOfRecord;
	tU16 SizeOfRecord;
	tU16 SizeOfQue;

	tU16 In; 
	tU16 Out;
	tU16 Cnt;
}tag_Queue;

/*********************************************************************************/
/**Function**/

tU8 SysQueueGeneralInit(tag_Queue *Que, tU16 SizeOfRecord, tU16 NumberOfRecord, enum_QueueType StoreType, tU8 Overwrite);
tU8 SysQueuePush(tag_Queue *Que, void *Record);
tU8 SysQueuePop(tag_Queue *Que, void *Record);
tU8 SysQueuePeek(tag_Queue *Que, void *Record, enum_PeekDirection Dir, tU16 Move, enum_PeekAction Action);
tU8 SysQueueDrop(tag_Queue *Que);

/*********************************************************************************/
#endif //SYS_QUEUE_REVISION_DATE
