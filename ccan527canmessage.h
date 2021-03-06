#ifndef C_CAN_527_CAN_MESSAGE_H
#define C_CAN_527_CAN_MESSAGE_H

//****************************************************************************************************
//Класс настройки сообщения
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <stdint.h>

//****************************************************************************************************
//Класс настройки сообщения
//****************************************************************************************************

class CCAN527CANMessage
{
 //-переменные-----------------------------------------------------------------------------------------
 public:
  bool ExtendedMode;//расширенный режим
  uint32_t Arbitration;//арбитраж
  bool ReceiveMode;//сообщение на приём
 //-конструктор----------------------------------------------------------------------------------------
 public:
  CCAN527CANMessage(bool extended_mode=false,uint32_t arbitration=0,bool receive_mode=false);
 //-деструктор-----------------------------------------------------------------------------------------
  ~CCAN527CANMessage(void);
 //-открытые функции-----------------------------------------------------------------------------------
 public:
  bool IsExtendedMode(void) const;//получить, расширенный ли режим 
  bool IsReceiveMode(void) const;//получить, на приём ли сообщение
 //-закрытые функции-----------------------------------------------------------------------------------
 private: 
};

#endif


