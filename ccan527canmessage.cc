//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include "ccan527canmessage.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//конструктор и деструктор класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//конструктор
//----------------------------------------------------------------------------------------------------
CCAN527CANMessage::CCAN527CANMessage(bool extended_mode,uint32_t arbitration,bool receive_mode)
{
 ExtendedMode=extended_mode;
 Arbitration=arbitration;
 ReceiveMode=receive_mode;
}
//----------------------------------------------------------------------------------------------------
//деструктор
//----------------------------------------------------------------------------------------------------
CCAN527CANMessage::~CCAN527CANMessage()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//закрытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//открытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//получить, расширенный ли режим 
//----------------------------------------------------------------------------------------------------
bool CCAN527CANMessage::IsExtendedMode(void) const
{
 return(ExtendedMode);	
}
//----------------------------------------------------------------------------------------------------
//получить, на приём ли сообщение
//----------------------------------------------------------------------------------------------------
bool CCAN527CANMessage::IsReceiveMode(void) const
{
 return(ReceiveMode);	
}
