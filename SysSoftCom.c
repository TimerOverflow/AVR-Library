/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : SysSoftCom.c
*/
/*********************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "SysSoftCom.h"
/*********************************************************************************/
#if(SYS_SOFTCOM_REVISION_DATE != 20200108)
#error wrong include file. (SysSoftCom.h)
#endif
/*********************************************************************************/
#define CTRL_TX_PIN()						RawData->HalTxPin(RawData->Tx.Bit.HiLow ? SOFTCOM_PIN_CLR : SOFTCOM_PIN_SET)
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
static tU8 CheckAllOfRawDataInit(tag_SysSoftComRawDataCtrl *RawData)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소

		2) 반환
			- 필수 항목 초기화 여부.

		3) 설명
			- 'tag_SysSoftComRawDataCtrl' 인스턴스의 필수 항목 초기화 여부 확인.
	*/

	return (RawData->Bit.InitBuffer && RawData->Bit.InitHal) ? true : false;
}
/*********************************************************************************/
static tU8 CheckRxPin(tag_SysSoftComRawDataCtrl *RawData)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소

		2) 반환
			-	Rx핀 상태 변경 여부.

		3) 설명
			- Rx핀 제어.
	*/

	tU8 Change = false;

	if(RawData->HalRxPin(SOFTCOM_PIN_READ))
	{
		if(RawData->Rx.Bit.HiLow) Change = true;
		RawData->Rx.Bit.HiLow = LOW;
	}
	else
	{
		if(!RawData->Rx.Bit.HiLow) Change = true;
		RawData->Rx.Bit.HiLow = HIGH;
	}

	return Change;
}
/*********************************************************************************/
static void RawDataTx(tag_SysSoftComRawDataCtrl *RawData)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소

		2) 반환
			- 없음.

		3) 설명
			- low level 송신 처리.
	*/

	tag_RawData *Tx = &RawData->Tx;

	RawData->HalEnPin(SOFTCOM_PIN_SET);

	if(Tx->Delay)
	{
		Tx->Delay--;
		CTRL_TX_PIN();
	}

	if(!Tx->Delay)
	{
		switch(Tx->Step)
		{
			case	0	:

				Tx->Bit.HiLow = HIGH; Tx->Bit.End = false;
				Tx->Delay = SYS_SOFTCOM_TX_HEAD_H; Tx->Step++;
			break;

			case	1	:

				Tx->Bit.HiLow = LOW; Tx->DataCnt = 0; Tx->BitCnt = 0; Tx->ChkSum = 0;
				Tx->Delay = SYS_SOFTCOM_TX_HEAD_L; Tx->Step++;
			break;

			case	2	:

				if(Tx->Bit.HiLow)
				{
					Tx->Bit.HiLow = LOW;
					if(Tx->Buf[Tx->DataCnt] & (0x80 >> Tx->BitCnt)) Tx->Delay = SYS_SOFTCOM_TX_SH;
					else Tx->Delay = SYS_SOFTCOM_TX_SL;

					if(++Tx->BitCnt >= 8)
					{
						Tx->BitCnt = 0;
						Tx->ChkSum ^= Tx->Buf[Tx->DataCnt];
						if(++Tx->DataCnt >= RawData->Length) Tx->Bit.End = true;
						if(Tx->DataCnt == (RawData->Length - 1)) Tx->Buf[(RawData->Length - 1)] = Tx->ChkSum;
					}
				}
				else
				{
					Tx->Bit.HiLow = HIGH; Tx->Delay = SYS_SOFTCOM_TX_SL;
					if(Tx->Bit.End) Tx->Step++;
				}
			break;

			case	3	:

				Tx->Bit.End = false; Tx->Bit.HiLow = LOW; Tx->Step = 0; RawData->Bit.DataSend = false;
				CTRL_TX_PIN(); RawData->HalEnPin(SOFTCOM_PIN_CLR);
			break;
		}
	}
}
/*********************************************************************************/
static void RawDataRx(tag_SysSoftComRawDataCtrl *RawData)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소

		2) 반환
			- 없음.

		3) 설명
			- low level 수신 처리.
	*/

	tU8 Exit = false;
	tag_RawData *Rx = &RawData->Rx;

	if(RawData->Bit.DataReceive) return;

	Rx->Delay++; Rx->Delay &= 0x7F;

	if(CheckRxPin(RawData) == true || Rx->Bit.End)
	{
		Rx->Bit.End = false;

		switch(Rx->Step)
		{
			case	0	:

				if((!Rx->Bit.HiLow) && (SYS_SOFTCOM_RX_SH_L <= Rx->Delay) && (Rx->Delay <= SYS_SOFTCOM_RX_SH_H))
				{
					Rx->Step++;
				}
				else
				{
					Exit = true;
				}
			break;

			case	1	:

				if((Rx->Bit.HiLow) && (SYS_SOFTCOM_RX_SL_L <= Rx->Delay) && (Rx->Delay <= SYS_SOFTCOM_RX_SL_H))
				{
					Rx->DataCnt = 0; Rx->BitCnt = 0; Rx->ChkSum = 0;
					Rx->Step++;
				}
				else
				{
					Exit = true;
				}
			break;

			case	2	:

				if((SYS_SOFTCOM_RX_SHL <= Rx->Delay) && (Rx->Delay <= SYS_SOFTCOM_RX_SHH))
				{
					if(Rx->Bit.HiLow)
					{
						Rx->Buf[Rx->DataCnt] <<= 1;
						if(Rx->Delay > SYS_SOFTCOM_RX_SHC) Rx->Buf[Rx->DataCnt] |= 0x01;

						if(++Rx->BitCnt >= 8)
						{
							Rx->BitCnt = 0;
							if(++Rx->DataCnt >= RawData->Length)
							{
								Rx->Bit.End = true;
								Rx->Step++;
								break;
							}
							Rx->ChkSum ^= Rx->Buf[Rx->DataCnt - 1];
						}
					}
				}
				else
				{
					Exit = true;
				}
			break;

			case	3	:

				if(Rx->Buf[RawData->Length - 1] == Rx->ChkSum)
				{
					RawData->Bit.DataReceive = true;
					RawData->ResponseDelayCnt = RawData->ResponseDelaySet;
				}
				Exit = true;
			break;
		}

		Rx->Delay = 0;
	}

	if(Exit == true)
	{
		Rx->Step = 0;
		Rx->Bit.End = false;
	}
}
/*********************************************************************************/
tU8 SysSoftComRawDataLinkBuffer(tag_SysSoftComRawDataCtrl *RawData, tU8 *TxBuf, tU16 TxBufSize, tU8 *RxBuf, tU16 RxBufSize)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소
			- TxBuf : Tx 버퍼의 주소
			- TxBufSize : Tx 버퍼의 크기
			- RxBuf : Rx 버퍼의 주소
			- RxBufSize : Rx 버퍼의 크기

		2) 반환
			- 버퍼 연결 성공 여부.

		3) 설명
			- 'tag_SysSoftComRawDataCtrl' 인스턴스에 low level 송,수신 버퍼 연결.
	*/

	RawData->Tx.Buf = TxBuf;
	RawData->Tx.BufSize = TxBufSize;
	memset(RawData->Tx.Buf, 0, TxBufSize);

	RawData->Rx.Buf = RxBuf;
	RawData->Rx.BufSize = RxBufSize;
	memset(RawData->Rx.Buf, 0, RxBufSize);

	RawData->Bit.InitBuffer = true;
	RawData->Bit.InitComplete = CheckAllOfRawDataInit(RawData);

	return RawData->Bit.InitBuffer;
}
/*********************************************************************************/
tU8 SysSoftComRawDataLinkHal(tag_SysSoftComRawDataCtrl *RawData, tU8 (*HalEnPin)(enum_SoftComPinCtrl Ctrl), tU8 (*HalTxPin)(enum_SoftComPinCtrl Ctrl), tU8 (*HalRxPin)(enum_SoftComPinCtrl Ctrl))
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소.
			- HalEnPin : 485 Enable Pin HAL.
			- HalTxPin : 485 TX Pin HAL.
			- HalRxPin : 485 RX Pin HAL.


		2) 반환
			- 'tag_SysSoftComRawDataCtrl' 인스턴스에 HAL 연결 성공 여부.

		3) 설명
			- low level 송,수신 제어를 위한 HAL 연결.
	*/

	RawData->HalEnPin = HalEnPin;
	RawData->HalTxPin = HalTxPin;
	RawData->HalRxPin = HalRxPin;

	RawData->Bit.InitHal = true;
	RawData->Bit.InitComplete = CheckAllOfRawDataInit(RawData);

	return RawData->Bit.InitHal;
}
/*********************************************************************************/
void SysSoftComRawDataControl(tag_SysSoftComRawDataCtrl *RawData)
{
	/*
		1) 인수
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소

		2) 반환
			- 없음.

		3) 설명
			-	low level 송수신 처리 함수 호출.
			- Master와 Slave는 같은 주기의 타이머 인터럽트에서 본 함수 호출.
	*/

	if(RawData->Bit.InitComplete == false)
	{
		return;
	}

	if(RawData->Bit.DataSend == true)
	{
		RawDataTx(RawData);
	}
	else
	{
		RawDataRx(RawData);
	}
}
/*********************************************************************************/
#if(SYS_SOFTCOM_SLAVE == true)

static tU8 CheckAllOfSlaveInit(tag_SysSoftComSlaveCtrl *Slave)
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 주소

		2) 반환
			- 필수 항목 초기화 여부.

		3) 설명
			- 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 필수 항목 초기화 여부 확인.
	*/

	return (Slave->Bit.InitGeneral && Slave->Bit.InitUserTxFunc && Slave->Bit.InitUserRxFunc) ? true : false;
}
/*********************************************************************************/
tU8 SysSoftComSlaveInitGeneral(tag_SysSoftComSlaveCtrl *Slave, tag_SysSoftComRawDataCtrl *RawData, tU16 Length)
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입의 인스턴스 주소.
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입의 인스턴스 주소.
			- Length : 송,수신 길이 (Byte)

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- 일반정보 초기화.
	*/

	if((RawData != null) && (RawData->Bit.InitComplete == true))
	{
		Slave->RawData = RawData;
		Slave->RawData->Length = Length;

		Slave->Bit.InitGeneral = true;
	}

	Slave->Bit.InitComplete = CheckAllOfSlaveInit(Slave);

	return Slave->Bit.InitGeneral;
}
/*********************************************************************************/
tU8 SysSoftComSlaveUserTxFunc(tag_SysSoftComSlaveCtrl *Slave, void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id))
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 주소.
			- UserTxFunction : 사용자 정의 송신함수 주소.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- 'tag_SysSoftComSlaveCtrl' 타입 인스턴스에 사용자 정의 송신 함수 연결.
	*/

	Slave->UserTxFunction = UserTxFunction;

	Slave->Bit.InitUserTxFunc = true;
	Slave->Bit.InitComplete = CheckAllOfSlaveInit(Slave);

	return Slave->Bit.InitUserTxFunc;
}
/*********************************************************************************/
tU8 SysSoftComSlaveUserRxFunc(tag_SysSoftComSlaveCtrl *Slave, void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id))
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 주소.
			- UserRxFunction : 사용자 정의 수신함수 주소.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- 'tag_SysSoftComSlaveCtrl' 타입 인스턴스에 사용자 정의 수신 함수 연결.
	*/

	Slave->UserRxFunction = UserRxFunction;

	Slave->Bit.InitUserRxFunc = true;
	Slave->Bit.InitComplete = CheckAllOfSlaveInit(Slave);

	return Slave->Bit.InitUserRxFunc;
}
/*********************************************************************************/
tU8 SysSoftComSlaveSetResponseDelay(tag_SysSoftComSlaveCtrl *Slave, tU16 ResponseDelay_ms, tU16 SlaveProcTick_ms)
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 주소.
			- ResponseDelay_ms : 설정 응답지연시간.
			- SlaveProcTick_ms : AvrSoftComSlaveProc() 함수 실행 주기.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- Slave는 수신 완료 후 설정한 시간 동안 응답을 지연함.
	*/

	if(Slave->Bit.InitComplete == false)
	{
		return false;
	}

	Slave->RawData->ResponseDelaySet = ResponseDelay_ms / SlaveProcTick_ms;

	return true;
}
/*********************************************************************************/
void SysSoftComSlaveProc(tag_SysSoftComSlaveCtrl *Slave, tU8 Id)
{
	/*
		1) 인수
			- Slave : 'tag_SysSoftComSlaveCtrl' 타입 인스턴스의 주소.

		2) 반환
			- 없음.

		3) 설명
			- application level slave 송,수신 처리.
	*/

	if(Slave->Bit.InitComplete == false)
	{
		return;
	}

	if(Slave->RawData->Bit.DataSend == false && Slave->RawData->HalEnPin(SOFTCOM_PIN_READ))
	{
		Slave->RawData->HalEnPin(SOFTCOM_PIN_CLR);
	}

	if(Slave->RawData->ResponseDelayCnt) Slave->RawData->ResponseDelayCnt--;
	if((Slave->RawData->Bit.DataReceive == true) && (Slave->RawData->ResponseDelayCnt == 0))
	{
		Slave->RawData->Bit.DataReceive = false;

		if(Slave->RawData->Rx.Buf[0] == Id)
		{
			Slave->UserRxFunction(&Slave->RawData->Tx.Buf[1], &Slave->RawData->Rx.Buf[1], Id);
			Slave->UserTxFunction(&Slave->RawData->Tx.Buf[1], &Slave->RawData->Rx.Buf[1], Id);

			Slave->RawData->Tx.Buf[0] = Id;
			Slave->RawData->Bit.DataSend = true;
		}
	}
}

#endif
/*********************************************************************************/
#if(SYS_SOFTCOM_MASTER == true)

static tU8 CheckAllOfMasterInit(tag_SysSoftComMasterCtrl *Master)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소

		2) 반환
			- 필수 항목 초기화 여부.

		3) 설명
			- 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 필수 항목 초기화 여부 확인.
	*/

	return (Master->Bit.InitGeneral && Master->Bit.InitUserTxFunc && Master->Bit.InitUserRxFunc && Master->Bit.InitWriteFlag) ? true : false;
}
/*********************************************************************************/
static tag_SysSoftComSlaveInfo* GetAddedSlaveInfo(tag_SysSoftComMasterCtrl *Master, tag_SysSoftComSlaveInfo *Slave)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- Slave : 'tag_SysSoftComSlaveInfo' 타입 인스턴스의 주소.

		2) 반환
			- 'tag_SysSoftComSlaveInfo' 타입 포인터.

		3) 설명
			- 추가 되어 있는 다음 slave의 주소를 찾아 반환.
	*/

	tU8 i = 0;
	tag_SysSoftComSlaveInfo *SlaveTemp = Slave;

	do
	{
		if(Slave == &Master->SlaveArray[Master->MaxSlave - 1])
		{
			Slave = Master->SlaveArray;
		}
		else
		{
			Slave++;
		}

		if(++i > Master->MaxSlave)
		{
			Slave = SlaveTemp;
			break;
		}
	}while(Slave->Id == 0);

	return Slave;
}
/*********************************************************************************/
static tag_SysSoftComSlaveInfo* FindSlaveById(tag_SysSoftComMasterCtrl *Master, tU8 Id)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- Id : 찾고자 하는 slave의 id

		2) 반환
			- 'tag_SysSoftComSlaveInfo' 타입 포인터.

		3) 설명
			- 인수로 받은 id를 추가된 slave 중 검색하여 일치하는 slave의 주소 반환.
			- 검색결과 일치하지 않을 경우 null 반환.
	*/

	tU8 i, Find = false;
	tag_SysSoftComSlaveInfo *Slave = Master->SlaveArray;

	for(i = 0; i < Master->AddedSlave; i++)
	{
		if(Slave->Id == Id)
		{
			Find = true;
			break;
		}
		Slave = GetAddedSlaveInfo(Master, Slave);
	}

	return Find ? Slave : null;
}
/*********************************************************************************/
tU8 SysSoftComMasterInitGeneral(tag_SysSoftComMasterCtrl *Master, tag_SysSoftComRawDataCtrl *RawData, tU8 MaxSlave, tU16 PollDelay_ms, tU16 MasterProcTick_ms)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- RawData : 'tag_SysSoftComRawDataCtrl' 타입 인스턴스의 주소.
			- MaxSlave : 최대 slave의 수.
			- PollDelay_ms : 설정 polling delay.
			- MasterProcTick_ms : 'AvrSoftComMasterProc()' 함수가 실행되는 주기 (단위 ms)

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- 일반정보 초기화.
	*/

	if(RawData->Bit.InitComplete == true)
	{
		Master->RawData = RawData;
	}

	Master->PollDelay = PollDelay_ms / MasterProcTick_ms;
	Master->MaxSlave = MaxSlave;
	Master->AddedSlave = 0;
	Master->SlaveArray = (tag_SysSoftComSlaveInfo *) calloc(Master->MaxSlave, sizeof(tag_SysSoftComSlaveInfo));

	Master->Bit.InitGeneral = (Master->RawData == null || Master->SlaveArray == null) ? false : true;
	Master->Bit.InitComplete = CheckAllOfMasterInit(Master);

	return Master->Bit.InitGeneral;
}
/*********************************************************************************/
tU8 SysSoftComMasterUserTxFunc(tag_SysSoftComMasterCtrl *Master, void (*UserTxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id))
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- UserTxFunction : 사용자 정의 송신 함수 주소.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- application level 송신 처리를 위한 사용자 정의 송신 함수 연결.
	*/

	Master->UserTxFunction = UserTxFunction;

	Master->Bit.InitUserTxFunc = true;
	Master->Bit.InitComplete = CheckAllOfMasterInit(Master);

	return Master->Bit.InitUserTxFunc;
}
/*********************************************************************************/
tU8 SysSoftComMasterUserRxFunc(tag_SysSoftComMasterCtrl *Master, void (*UserRxFunction)(tU8 *TxBuf, tU8 *RxBuf, tU8 Id))
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- UserRxFunction : 사용자 정의 수신 함수 주소.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- application level 수신 처리를 위한 사용자 정의 수신 함수 연결.
	*/

	Master->UserRxFunction = UserRxFunction;

	Master->Bit.InitUserRxFunc = true;
	Master->Bit.InitComplete = CheckAllOfMasterInit(Master);

	return Master->Bit.InitUserRxFunc;
}
/*********************************************************************************/
tU8 SysSoftComMasterLinkWriteFlag(tag_SysSoftComMasterCtrl *Master, tU8 *WriteFlag)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- WriteFlag : Master Write Flag 주소.

		2) 반환
			- 초기화 성공 여부.

		3) 설명
			- application level에서 핸들링하는 master write flag를 연결한다.
			- master는 연결된 flag를 검사하여 set일 경우 UserRxFunction()함수를 실행하지 않는다.
	*/

	Master->WriteFlag = WriteFlag;
	Master->Bit.InitWriteFlag = true;
	Master->Bit.InitComplete = CheckAllOfMasterInit(Master);

	return Master->Bit.InitWriteFlag;
}
/*********************************************************************************/
tU8 SysSoftComMasterAddSlave(tag_SysSoftComMasterCtrl *Master, tU8 Id, tU16 Length, void *TargetData)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- Id : 추가하는 slave의 id.
			- Length : 추가하는 slave의 송수신 길이. (Byte)
			- TargetData : 예비.

		2) 반환
			- slave 추가 성공 여부.

		3) 설명
			- master가 관리할 slave 추가.
			- 추가된 slave 수가 최대치 이거나, id가 중복 되어 있을 경우 추가에 실패.
	*/

	tU8 i;
	tag_SysSoftComSlaveInfo *Slave = Master->SlaveArray;

	if((Master->Bit.InitComplete == false) || (Master->AddedSlave >= Master->MaxSlave))
	{
		return false;
	}

	for(i = 0; i < Master->AddedSlave; i++)
	{
		if(Slave->Id == Id)
		{
			return false;
		}
		Slave = GetAddedSlaveInfo(Master, Slave);
	}

	for(i = 0; i < Master->MaxSlave; i++)
	{
		if(Master->SlaveArray[i].Id == 0)
		{
			Master->SlaveArray[i].Id = Id;
			Master->SlaveArray[i].Length = Length;
			Master->SlaveArray[i].TargetData = null;
			Master->SlaveArray[i].NoResponseCnt = 0;
			Master->SlaveArray[i].NoResponseLimit = SYS_SOFTCOM_DEFAULT_SLAVE_NO_RESPONSE;

			Master->SlaveReceive = Master->SlavePoll = &Master->SlaveArray[i];
			Master->AddedSlave++;

			return true;
		}
	}

	return false;
}
/*********************************************************************************/
void SysSoftComMasterRemoveSlave(tag_SysSoftComMasterCtrl *Master, tU8 Id)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- Id : 제거할 slave의 id.

		2) 반환
			- 없음.

		3) 설명
			- 이미 추가된 slave 중 인수로 받은 id를 검색하여 일치하는 slave 제거.
	*/

	tag_SysSoftComSlaveInfo *Slave;

	if((Master->Bit.InitComplete == false) || (Master->AddedSlave == 0))
	{
		return;
	}

	Slave = FindSlaveById(Master, Id);

	if(Slave != null)
	{
		memset(Slave, 0, sizeof(tag_SysSoftComSlaveInfo));
		Master->AddedSlave--;
	}
}
/*********************************************************************************/
tU8 SysSoftComMasterCheckSlaveNoResponse(tag_SysSoftComMasterCtrl *Master, tU8 Id)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.
			- Id : 무응답 여부를 확인할 slave의 id.

		2) 반환
			- 무응답 여부.

		3) 설명
			- 추가된 slave 중 인수로 받아들인 id를 검색하여 일치하는 slave의 무응답 여부 확인.
	*/

	tag_SysSoftComSlaveInfo *Slave;

	if((Master->Bit.InitComplete == false) || (Master->AddedSlave == 0))
	{
		return false;
	}

	Slave = FindSlaveById(Master, Id);

	if((Slave != null) && (Slave->NoResponseCnt >= Slave->NoResponseLimit))
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*********************************************************************************/
void SysSoftComMasterSetSlaveNoResponse(tag_SysSoftComMasterCtrl *Master, tU8 Id, tU8 NoResponseLimit)
{
	/*
		1) 인수
			- Master : tag_SysSoftComMasterCtrl 인스턴스의 주소.
			- Id : Slave의 ID.
			- NoResponseLimit : 무응답 횟수.

		2) 반환
		  - 없음.

		3) 설명
			- 인수로 받은 ID와 동일한 Slave를 검색하여 일치하는 Slave의 무응답 횟수 설정.
	*/

	tag_SysSoftComSlaveInfo *Slave;

	if((Master->Bit.InitComplete == false) || (Master->AddedSlave == 0))
	{
		return;
	}

	Slave = FindSlaveById(Master, Id);

	if(Slave != null)
	{
		Slave->NoResponseLimit = NoResponseLimit;
		Slave->NoResponseCnt = 0;
	}
}
/*********************************************************************************/
void SysSoftComMasterProc(tag_SysSoftComMasterCtrl *Master)
{
	/*
		1) 인수
			- Master : 'tag_SysSoftComMasterCtrl' 타입 인스턴스의 주소.

		2) 반환
			- 없음.

		3) 설명
			- application level master 송,수신 함수 처리.
	*/

	if((Master->Bit.InitComplete == false) || (Master->AddedSlave == 0))
	{
		return;
	}

	if(Master->RawData->Bit.DataReceive == true)
	{
		Master->RawData->Bit.DataReceive = false;
		Master->SlaveReceive = FindSlaveById(Master, Master->RawData->Rx.Buf[0]);

		if((Master->SlaveReceive != null) && (*Master->WriteFlag == false))
		{
			Master->SlaveReceive->NoResponseCnt = 0;
			Master->UserRxFunction(&Master->RawData->Tx.Buf[1], &Master->RawData->Rx.Buf[1], Master->SlaveReceive->Id);
		}
	}
	else
	{
		if(Master->PollCnt)
		{
			if(Master->RawData->Bit.DataSend == false)
			{
				Master->PollCnt--;
			}
		}
		else
		{
			Master->PollCnt = Master->PollDelay;
			Master->SlavePoll = GetAddedSlaveInfo(Master, Master->SlavePoll);
			if(Master->SlavePoll->NoResponseCnt < Master->SlavePoll->NoResponseLimit)
			{
				Master->SlavePoll->NoResponseCnt++;
			}

			Master->RawData->Length = Master->SlavePoll->Length;
			Master->RawData->Tx.Buf[0] = Master->SlavePoll->Id;
			Master->RawData->Bit.DataSend = true;

			Master->UserTxFunction(&Master->RawData->Tx.Buf[1], &Master->RawData->Rx.Buf[1], Master->SlavePoll->Id);
		}
	}
}

#endif
/*********************************************************************************/
