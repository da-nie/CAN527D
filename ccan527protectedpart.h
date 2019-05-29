#ifndef C_CAN_527_PROTECTED_PART_H
#define C_CAN_527_PROTECTED_PART_H

//****************************************************************************************************
//Класс защищённой части платы CAN 527
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <stdint.h>
#include <vector>
#include <hw/pci.h>
#include <sys/neutrino.h>

#include "cmemorycontrol.h"
#include "ccan527canpackage.h"
#include "ccan527canmessage.h"
#include "ccan527channel.h"
#include "cuniqueptr.h"
#include "cmutex.h"
#include "cringbuffer.h"

//****************************************************************************************************
//Класс защищённой части платы CAN 527
//****************************************************************************************************

class CCAN527ProtectedPart
{
 //-переменные-----------------------------------------------------------------------------------------
 public:  	  	  	
  CMutex cMutex;//мютекс для доступа к классу
  static const uint32_t CAN_CHANNEL_AMOUNT=2;//количество каналов на плате
 private: 
  CUniquePtr<CCAN527Channel> cCAN527Channel_Ptr[CAN_CHANNEL_AMOUNT];//каналы платы CAN
  
  bool Enabled;//работа платы разрешена
  bool ExitThread;//нужно ли выходить из потока  
  CUniquePtr<CRingBuffer<CCAN527CANPackage> > cRingBuffer_Receiver_Ptr;//указатель на класс буфера принятых данных
 //-конструктор----------------------------------------------------------------------------------------
 public:
  CCAN527ProtectedPart(uint32_t receiver_buffer_size=1,uint32_t transmitter_buffer_size=1);//конструктор
 //-деструктор-----------------------------------------------------------------------------------------
  ~CCAN527ProtectedPart();//деструктор
 //-открытые функции-----------------------------------------------------------------------------------
 public:
  bool Init(uintptr_t base_addr_ch1,uintptr_t base_addr_ch2,uint32_t irq_ch1,uint32_t irq_ch2);//инициализировать плату
  void Release(void);//освободить ресурсы
  bool IsEnabled(void);//получить, подключена ли плата 
  uint8_t GetIRQ(uint32_t channel);//получить номер прерывания
  void ClearIRQ(uint32_t channel);//сбросить флаг прерывания
  bool CANInterruptAttach(sigevent *CANInterruptEvent,uint32_t channel);//подключить прерывание
  void CANInterruptDetach(uint32_t channel);//отключить прерывание
  bool IsExitThread(void);//получить, нужно ли выходить из потока
  void SetExitThread(bool state);//задать, нужно ли выходить из потока
  bool SetChannelMessage(uint32_t channel,uint32_t message_index,const CCAN527CANMessage &cCAN527CANMessage);//задать настройки сообщения
  bool SetChannelSpeed(uint32_t channel,CCAN527Channel::CAN_SPEED speed);//задать скорость обмена на канале
  bool SetChannelMask(uint32_t channel,uint32_t global_message_mask,uint32_t global_extended_message_mask,uint32_t message_15_mask);//задать маски на канале
  bool ApplyChannelSettings(uint32_t channel);//применить заданные настройки к каналу
  bool GetReceivedPackage(CCAN527CANPackage &cCAN527CANPackage);//получить принятый пакет
  bool TransmittPackage(const CCAN527CANPackage &cCAN527CANPackage);//добавить пакет для отправки
  void TransmittProcessing(uint32_t channel);//выполнить передачу данных, если это возможно
  void OnInterrupt(uint32_t channel);//обработчик прерывания
  void ClearReceiverBuffer(uint32_t channel);//очистить буфер приёма
  void ClearTransmitterBuffer(uint32_t channel);//очистить буфер передачи
  bool BussOffControl(uint32_t channel);//проверить работоспособность контроллера канала и сбросить канал при необходимости
  void Processing(uint32_t channel);//цикл обработки  
 //-закрытые функции-----------------------------------------------------------------------------------
 private:
};
#endif



