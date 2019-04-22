#ifndef C_CAN_527_H
#define C_CAN_527_H

//****************************************************************************************************
//Класс управления платой CAN 527
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <stdint.h>
#include <vector>
#include <hw/pci.h>
#include <sys/neutrino.h>

#include "cmemorycontrol.h"
#include "cuniqueptr.h"
#include "cthread.h"
#include "ccan527protectedpart.h"

//****************************************************************************************************
//протитипы функций
//****************************************************************************************************

static void* ThreadFunction(void *param);//функция потока

//****************************************************************************************************
//Класс управления платой CAN 527
//****************************************************************************************************
class CCAN527
{
 //-дружественные функции и классы---------------------------------------------------------------------
 friend void* ThreadFunction(void *param);
 //-переменные-----------------------------------------------------------------------------------------
 private:
  static const uint32_t THREAD_PRIORITY=50;//приоритет потока
  static const uint32_t RECEIVER_RING_BUFFER_SIZE=500;//размер очереди приёма данных
  static const uint32_t TRANSMITTER_RING_BUFFER_SIZE=500;//размер очереди передачи данных
  
  sigevent CANInterruptEvent;//событие прерывания CAN  
  CUniquePtr<CThread> cThread_Ptr;//указатель на поток управления
  //класс защищённой части
  CUniquePtr<CCAN527ProtectedPart> cCAN527ProtectedPart_Ptr;//указатель на класс защищённой части
 //-конструктор----------------------------------------------------------------------------------------
 public:
  CCAN527(void);
 //-деструктор-----------------------------------------------------------------------------------------
  ~CCAN527(void);
 //-открытые функции-----------------------------------------------------------------------------------
 public:
  bool Init(uintptr_t base_addr_ch1,uintptr_t base_addr_ch2,uint32_t irq_ch1,uint32_t irq_ch2);//инициализировать плату
  void Release(void);//освободить ресурсы
  void GetAllReceivedPackage(std::vector<CCAN527CANPackage> &vector_CCAN527CANPackage);//получить все принятые пакеты
  void GetPartReceivedPackage(std::vector<CCAN527CANPackage> &vector_CCAN527CANPackage,size_t counter);//получить не больше заданного количества принятых пакетов
  bool GetOneReceivedPackage(CCAN527CANPackage &cCAN527CANPackage);//получить один принятый пакет
  bool TransmittPackage(const CCAN527CANPackage &cCAN527CANPackage);//добавить пакет для отправки
  
  bool SetChannelMessage(uint32_t channel,uint32_t message_index,const CCAN527CANMessage &cCAN527CANMessage);//задать настройки сообщения
  bool SetChannelSpeed(uint32_t channel,CCAN527Channel::CAN_SPEED speed);//задать скорость обмена на канале
  bool SetChannelMask(uint32_t channel,uint32_t global_message_mask,uint32_t global_extended_message_mask,uint32_t message_15_mask);//задать маски на канале
  bool ApplyChannelSettings(uint32_t channel);//применить заданные настройки к каналу
  void ClearReceiverBuffer(uint32_t channel);//очистить буфер приёма
  void ClearTransmitterBuffer(uint32_t channel);//очистить буфер передачи
 //-закрытые функции-----------------------------------------------------------------------------------
 private:
  bool IsChannelValid(uint32_t channel);//получить, допустим ли такой номер канала
  bool CANInterruptAttach(void);//подключиться к прерыванию
  void CANInterruptDetach(void);//отключиться от прерывания
  bool IsExitThread(void);//получить, нужно ли выходить из потока
  void SetExitThread(bool state);//задать, нужно ли выходить из потока
  void StartThread(void);//запустить поток
  void StopThread(void);//остановить поток  
  void OnInterrupt(void);//выполнить обработку прерывания
  bool Processing(void);//обработка платы до прерывания
};



#endif


