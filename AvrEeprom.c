/*********************************************************************************/
/*
 * Author : Jeong Hyun Gu
 * File name : AvrEeprom.c
*/
/*********************************************************************************/
#include <ina90.h>
#include "AvrEeprom.h"
/*********************************************************************************/
#if(AVR_EEPROM_REVISION_DATE != 20190114)
#error wrong include file. (AvrEeprom.h)
#endif
/*********************************************************************************/
/** Global variable **/


/*********************************************************************************/
char InitEepControl(tag_EepControl *Eep, const unsigned char *DataBase, unsigned int Length)
{
	const char GapOfAnotherSector = 10;
	static unsigned int AllocEepAddr = 0;
	char **pDataBase = (char **) &Eep->DataBase;
	unsigned int *pEepBase = (unsigned int *) &Eep->EepBase;
	unsigned int *pLength = (unsigned int *) &Eep->Length;
	tag_EepBitField *pBit = (tag_EepBitField *) &Eep->Bit;


	/*
		1) 인수
			- Eep : tag_EepControl 인스턴스의 주소
			- DataBase : eeprom을 통해 관리할 대상의 시작주소
			- Length : 관리할 대상의 크기

		2) 반환
			- 초기화 성공 여부

		3) 설명
			- tag_EepControl 인스턴스를 초기화한다.
			- tag_EepControl 인스턴스에 eeprom으로 부터 읽거나 쓸 대상의 주소와, 할당할 eeprom의 시작과 길이를 초기화한다.
	*/


	pBit->Init = false;

	if((DataBase != 0) && (Length != 0) && ((AllocEepAddr + Length + GapOfAnotherSector) < EEPROM_SIZE))
	{
		*pLength = Length;
		*pDataBase = (char *) DataBase;
		*pEepBase = AllocEepAddr;
		AllocEepAddr += (Length + GapOfAnotherSector);

		pBit->Init = true;
	}

	return pBit->Init;
}
/*********************************************************************************/
void DoEepReadControl(tag_EepControl *Eep)
{
	unsigned int Index = 0;

	/*
		1) 인수
			- Eep : tag_EepControl 인스턴스의 주소

		2) 반환
			- 없음.

		3) 설명
			- 해당 인스턴스의 eeprom으로 부터 데이터를 읽어와 대상 버퍼에 값을 대입.
	*/

	if(Eep->Bit.Init == false)
	{
		return;
		/* error or disabled */
	}

	do
	{
		Eep->DataBase[Index] = Eeprom_Read(Eep->EepBase + (Index));
	}while(++Index < Eep->Length);
}
/*********************************************************************************/
void GetDataFromEeprom(char* const Dest, const int EepBase, int Length)
{
	unsigned int Index = 0;

	/*
		1) 인수
			- Dest : eeprom으로 부터 읽은 데이터를 대입할 대상의 주소
			- EepBase : 읽을 eeprom의 시작주소 (메모리 주소가 아님)
			- Length : 읽어올 데이터의 길이.

		2) 반환
			- 없음.

		3) 설명
			- tag_EepControl 구조체를 사용하지 않고 임의 eeprom 영역으로 부터 데이터를 읽어와 대상 버퍼에 값을 대입한다.
	*/

	do
	{
		Dest[Index] = Eeprom_Read(EepBase + Index);
	}while(++Index < Length);
}
/*********************************************************************************/
char DoEepWriteControl(tag_EepControl *Eep)
{
	unsigned int *pIndex = (unsigned int *) &Eep->Index;
	tag_EepBitField *pBit;

	/*
		1) 인수
			- Eep : tag_EepControl 인스턴스의 주소

		2) 반환
			- 쓰기동작 중일 때 true, 그외 상황일 때 false.

		3) 설명
			- 해당 인스턴스의 eeprom 영역에 관리할 대상 데이터를 쓰기를 진행한다.
			- SetEepWriteEnable() 함수를 통해 'Eep.Bit.Write'를 활성화 시켜야 실행된다.
			- 쓰기전 해당주소의 값이 쓰려고 하는 값과 일치하지 않을 경우에만 유효하다고 판단하며,
				유효한 데이터만 쓰기를 진행하고 유효하지 않을 경우 다음 주소로 건너뛴다.
			- 값이 유효하여 쓰기를 진행하거나 해당영역의 마지막에 도달할 경우에만 while loop를 탈출한다.
	*/


	if((Eep->Bit.Init == false) || (Eep->Bit.Write == false))
	{
		return false;
		/* error or disabled */
	}

	while(true)
	{
		if(Eeprom_Read(Eep->EepBase + (*pIndex)) != Eep->DataBase[*pIndex])
		{
			Eeprom_Write(Eep->EepBase + (*pIndex), Eep->DataBase[*pIndex]);
			return true;
			/* check valid data */
		}

		if(++(*pIndex) >= Eep->Length)
		{
			pBit = (tag_EepBitField *) &Eep->Bit;
			pBit->Write = false;
			return false;
			/* end of sector */
		}
	}
}
/*********************************************************************************/
void SetEepWriteEnable(tag_EepControl *Eep)
{
	/*
		1) 인수
			- Eep : tag_EepControl 인스턴스의 주소

		2) 반환
			- 없음.

		3) 설명
			- 해당 인스턴스에 쓰기 동작을 활성화한다.
			- 'Eep.Bit.Write' 상태가 'true'일 때 DoEepWriteControl()함수가 쓰기 동작을 실행한다.
	*/

	tag_EepBitField *pBit = (tag_EepBitField *) &Eep->Bit;
	unsigned int *pIndex = (unsigned int *) &Eep->Index;

	pBit->Write = true;
	*pIndex = 0;
}
/*********************************************************************************/
void Eeprom_Write(unsigned int Addr, char Data)
{
	char cSREG;

	while(EECR & 0x02) _WDR();
	EEAR = Addr;
	EEDR = Data;

	cSREG = SREG;
	/* stored status register */

	SREG &= ~0x80;
	/* disable global interrupt */

	EECR |= 0x04;
	/* enable EEPROM Master Write EEMWE */

	EECR |= 0x02;
	/* enable EEPROM Write EEWE */

	SREG = cSREG;
	/* restored register */
}
/*********************************************************************************/
char Eeprom_Read(unsigned int Addr)
{
	while(EECR & 0x02) _WDR();
	EEAR = Addr;
	EECR |= 0x01;
	return EEDR;
}
/*********************************************************************************/
