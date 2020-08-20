/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysSoftCom.h
*/
/*********************************************************************************/
#ifndef __SYS_SOFT_COM_H__
#define	__SYS_SOFT_COM_H__
/*********************************************************************************/
#include "SysTypedef.h"
/*********************************************************************************/
#define SYS_SOFTCOM_REVISION_DATE				20200108
/*********************************************************************************/
/** REVISION HISTORY **/
/*
	2020. 01. 08.					- SysTypedef.h 적용.
	Jeong Hyun Gu

	2017. 09. 19.					- SysSoftComMasterLinkWriteFlag() 함수 추가.
	Jeong Hyun Gu					- tag_SysSoftComMasterCtrl::WriteFlag 추가.
													master 수신 중 write 동작이 정상 처리 되지 않던 현상 보완.

	2017. 09. 01.					- 멀티 플랫폼 지원을 위해 Hardware Abstraction Layer 추가.
	Jeong Hyun Gu						파일명 AvrSoftCom -> SysSoftCom, 접두어 AVR->SYS 변경.
												- AVR 레지스터 연결 삭제.
												- CtrlTxPin()함수를 매크로 함수로 변경.

	2017. 06. 01.					- tag_AvrSoftComSlaveCtrl::Id 삭제.
	Jeong Hyun Gu					- AvrSoftComSlaveSetId() 함수 삭제.
												- AvrSoftComSlaveProc() 함수에 'Id' 인수 추가.
												- tag_AvrSoftComRawDataCtrl::ResponseDelaySet 추가.
												- tag_AvrSoftComRawDataCtrl::ResponseDelayCnt 추가.
												- AvrSoftComSlaveSetResponseDelay() 함수 추가.
												  Slave 응답지연시간 설정은 필수가 아니라 선택 사항.

	2017. 04. 18.					- tag_RawData::DataCnt 자료형 변경 char -> int
	Jeong Hyun Gu					- AvrSoftComMasterSetSlaveNoResponse() 함수 추가.

	2016. 11. 15.					- RawData 처리 관련 전처리명에 접두어 'AVR_SOFTCOM' 추가.
	Jeong Hyun Gu					- 'AVR_REGISTER' 타입 삭제 -> 'char *' 타입으로 변경.
												- Slave 무응답 카운트가 오버플로우 하던 버그 수정.
												- Slave 수신 대기 중 enable port 비활성화 추가.

	2016.	11. 08.					- revision valid check 추가.
	Jeong Hyun Gu

	2016. 11. 04.					- Master, Slave, RawDataControl 파트 분리.
	Jeong Hyun Gu					- 1:N 통신을 고려하여 Master에 Slave 추가 및 삭제 기능 추가.
												- 하위호환 불가능.

	2016. 10. 28.					- 초기버전.
	Jeong Hyun Gu
*/
/*********************************************************************************/
/**Define**/

#define null						0
#define	false						0
#define	true						1

#define LOW 	          0
#define HIGH            1

#define SYS_SOFTCOM_MASTER	true
#define	SYS_SOFTCOM_SLAVE		false

#define SYS_SOFTCOM_DEFAULT_SLAVE_NO_RESPONSE			20

#define SYS_SOFTCOM_RX_HEAD_H       5000
#define SYS_SOFTCOM_RX_HEAD_L       3000
#define SYS_SOFTCOM_RX_PULSE        250
#define SYS_SOFTCOM_RX_HIGH         1500
#define SYS_SOFTCOM_RX_LOW          800

#define SYS_SOFTCOM_RX_SH_L         ((SYS_SOFTCOM_RX_HEAD_H / SYS_SOFTCOM_RX_PULSE) - 4)
#define SYS_SOFTCOM_RX_SH_H         ((SYS_SOFTCOM_RX_HEAD_H / SYS_SOFTCOM_RX_PULSE) + 4)
#define SYS_SOFTCOM_RX_SL_L         ((SYS_SOFTCOM_RX_HEAD_L / SYS_SOFTCOM_RX_PULSE) - 3)
#define SYS_SOFTCOM_RX_SL_H         ((SYS_SOFTCOM_RX_HEAD_L / SYS_SOFTCOM_RX_PULSE) + 3)
#define SYS_SOFTCOM_RX_SHL          ((SYS_SOFTCOM_RX_LOW / SYS_SOFTCOM_RX_PULSE) - 2)
#define SYS_SOFTCOM_RX_SHH          ((SYS_SOFTCOM_RX_HIGH / SYS_SOFTCOM_RX_PULSE) + 2)
#define SYS_SOFTCOM_RX_SHC          (((SYS_SOFTCOM_RX_HIGH + SYS_SOFTCOM_RX_LOW) / SYS_SOFTCOM_RX_PULSE ) / 2)

#define SYS_SOFTCOM_TX_HEAD_H       (SYS_SOFTCOM_RX_HEAD_H / SYS_SOFTCOM_RX_PULSE)
#define SYS_SOFTCOM_TX_HEAD_L       (SYS_SOFTCOM_RX_HEAD_L / SYS_SOFTCOM_RX_PULSE)
#define SYS_SOFTCOM_TX_SH           (SYS_SOFTCOM_RX_HIGH / SYS_SOFTCOM_RX_PULSE)
#define SYS_SOFTCOM_TX_SL           (SYS_SOFTCOM_RX_LOW / SYS_SOFTCOM_RX_PULSE)

/*********************************************************************************/
/**Enum**/

typedef enum
{
	SOFTCOM_PIN_CLR = 0,
	SOFTCOM_PIN_SET,
	SOFTCOM_PIN_READ,
}enum_SoftComPinCtrl;

/*********************************************************************************/
/**Struct**/

typedef struct
{
	tU8 *Buf;
	tU16 BufSize;
	tU16 DataCnt;
	tU8 Step;
	tU8 Delay;
	tU8 BitCnt;
	tU8 ChkSum;

	struct
	{
		tU8 HiLow					:		1;
		tU8 End						:		1;
	}Bit;
}tag_RawData;

typedef struct
{
	struct
	{
		tU8 InitBuffer			:			1;			//버퍼연결 확인
		tU8 InitHal					:			1;			//제어핀 초기화 확인
		tU8 InitComplete		:			1;			//전체 초기화 항목 확인

		tU8 DataSend				:			1;			//송신 플래그
		tU8 DataReceive			:			1;			//수신완료 플래그
	}Bit;

	tU8 (*HalEnPin)(enum_SoftComPinCtrl Ctrl);
	tU8 (*HalTxPin)(enum_SoftComPinCtrl Ctrl);
	tU8 (*HalRxPin)(enum_SoftComPinCtrl Ctrl);

	tU16 Length;						//송수신 길이 (Byte)
	tU16 ResponseDelaySet;	//응답지연시간
	tU16 ResponseDelayCnt;	//응답지연시간

	tag_RawData	Tx;
	tag_RawData	Rx;
}tag_SysSoftComRawDataCtrl;

#if(SYS_SOFTCOM_SLAVE == true)

typedef struct
{
	struct
	{
		tU8 InitGeneral				:			1;			//일반정보 초기화 확인
		tU8 InitUserTxFunc		:			1;			//사용자 정의 송신 함수 연결 확인
		tU8 InitUserRxFunc		:			1;			//사용자 정의 수신 함수 연결 확인
		tU8 InitComplete			:			1;			//전체 초기화 항목 확인
	}Bit;

	tag_SysSoftComRawDataCtrl *RawData;
	void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id);		//사용자 정의 송신 함수 포인터
	void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id);		//사용자 정의 수신 함수 포인터
}tag_SysSoftComSlaveCtrl;

#endif

#if(SYS_SOFTCOM_MASTER == true)

typedef struct
{
	tU8 Id;											//ID
	tU8 NoResponseCnt;					//무응답 카운트
	tU8 NoResponseLimit;				//무응답 횟수
	tU16 Length;													//송수신 길이 (Byte)
	void *TargetData;											//
}tag_SysSoftComSlaveInfo;

typedef struct
{
	struct
	{
		tU8 InitGeneral				:			1;			//일반정보 초기화 확인
		tU8 InitUserTxFunc		:			1;			//사용자 정의 송신 함수 연결 확인
		tU8 InitUserRxFunc		:			1;			//사용자 정의 수신 함수 연결 확인
		tU8 InitWriteFlag			:			1;			//
		tU8 InitComplete			:			1;			//전체 초기화 항목 확인
	}Bit;

	tU8 MaxSlave;												//최대 Slave 수
	tU8 AddedSlave;											//추가된 Slave 수
	tU8 *WriteFlag;

	tU16 PollDelay;												//
	tU16 PollCnt;													//

	tag_SysSoftComSlaveInfo *SlaveArray;			//Slave list
	tag_SysSoftComSlaveInfo *SlavePoll;				//송신 Slave 포인터
	tag_SysSoftComSlaveInfo *SlaveReceive;		//수신 Slave 포인터

	tag_SysSoftComRawDataCtrl *RawData;
	void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id);		//사용자 정의 송신 함수 포인터
	void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id);		//사용자 정의 수신 함수 포인터
}tag_SysSoftComMasterCtrl;

#endif

/*********************************************************************************/
/**Function**/

tU8 SysSoftComRawDataLinkBuffer(tag_SysSoftComRawDataCtrl *RawData, tU8 *TxBuf, tU16 TxBufSize, tU8 *RxBuf, tU16 RxBufSize);
tU8 SysSoftComRawDataLinkHal(tag_SysSoftComRawDataCtrl *RawData, tU8 (*HalEnPin)(enum_SoftComPinCtrl Ctrl), tU8 (*HalTxPin)(enum_SoftComPinCtrl Ctrl), tU8 (*HalRxPin)(enum_SoftComPinCtrl Ctrl));
void SysSoftComRawDataControl(tag_SysSoftComRawDataCtrl *RawData);


#if(SYS_SOFTCOM_SLAVE == true)

tU8 SysSoftComSlaveInitGeneral(tag_SysSoftComSlaveCtrl *Slave, tag_SysSoftComRawDataCtrl *RawData, tU16 Length);
tU8 SysSoftComSlaveUserTxFunc(tag_SysSoftComSlaveCtrl *Slave, void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id));
tU8 SysSoftComSlaveUserRxFunc(tag_SysSoftComSlaveCtrl *Slave, void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id));
void SysSoftComSlaveProc(tag_SysSoftComSlaveCtrl *Slave, tU8 Id);
tU8 SysSoftComSlaveSetResponseDelay(tag_SysSoftComSlaveCtrl *Slave, tU16 ResponseDelay_ms, tU16 SlaveProcTick_ms);

#endif

#if(SYS_SOFTCOM_MASTER == true)

tU8 SysSoftComMasterInitGeneral(tag_SysSoftComMasterCtrl *Master, tag_SysSoftComRawDataCtrl *RawData, tU8 MaxSlave, tU16 PollDelay_ms, tU16 MasterProcTick_ms);
tU8 SysSoftComMasterUserTxFunc(tag_SysSoftComMasterCtrl *Master, void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id));
tU8 SysSoftComMasterUserRxFunc(tag_SysSoftComMasterCtrl *Master, void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id));
tU8 SysSoftComMasterLinkWriteFlag(tag_SysSoftComMasterCtrl *Master, tU8 *WriteFlag);
tU8 SysSoftComMasterAddSlave(tag_SysSoftComMasterCtrl *Master, tU8 Id, tU16 Length, void *TargetData);
void SysSoftComMasterRemoveSlave(tag_SysSoftComMasterCtrl *Master, tU8 Id);
void SysSoftComMasterSetSlaveNoResponse(tag_SysSoftComMasterCtrl *Master, tU8 Id, tU8 NoResponseLimit);
tU8 SysSoftComMasterCheckSlaveNoResponse(tag_SysSoftComMasterCtrl *Master, tU8 Id);
void SysSoftComMasterProc(tag_SysSoftComMasterCtrl *Master);

#endif

/*********************************************************************************/
#endif //__SYS_SOFT_COM_H__
