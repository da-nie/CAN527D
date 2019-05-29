#ifndef C_CAN_527_CHANNEL_H
#define C_CAN_527_CHANNEL_H

//****************************************************************************************************
//Класс зканала платы CAN 527
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <stdint.h>
#include <vector>
#include <hw/pci.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#include "cmemorycontrol.h"
#include "ccan527canpackage.h"
#include "ccan527canmessage.h"
#include "cuniqueptr.h"
#include "cmutex.h"
#include "cringbuffer.h"

//****************************************************************************************************
//макроопределения
//****************************************************************************************************
#define BIN16(b15,b14,b13,b12,b11,b10,b9,b8,b7,b6,b5,b4,b3,b2,b1,b0) ((b15<<15)|(b14<<14)|(b13<<13)|(b12<<12)|(b11<<11)|(b10<<10)|(b9<<9)|(b8<<8)|(b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|b0)
#define BIN8(b7,b6,b5,b4,b3,b2,b1,b0) ((b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|b0)

//****************************************************************************************************
//Класс канала платы CAN 527
//****************************************************************************************************

class CCAN527Channel
{
 //-переменные-----------------------------------------------------------------------------------------
 public:  	  	  	
  static const uint32_t MESSAGE_AMOUNT=15;//количество слотов сообщений
  //скорость
  enum CAN_SPEED
  {
   CAN_SPEED_125KBS,//скорость 125 кбит/с
   CAN_SPEED_250KBS,//скорость 250 кбит/с
   CAN_SPEED_500KBS,//скорость 500 кбит/с
   CAN_SPEED_1MBS//скорость 1 Мбит/с
  };	
 private: 
  static const size_t CHANNEL_MEMORY_SIZE=0xff;//размер области памяти канала
  static const long double CAN_CHANNEL_BUSS_OFF_DELAY_MS=500.0;//интервал перезапуска линии при Buss-off
  static const long double TRANSMITT_WAIT_TIME_MS=1.0;//время ожидания завершения передачи
  //адреса регистров
  static const uint32_t CONTROL_RG_CCE=6;
  static const uint32_t CONTROL_RG_EIE=3;
  static const uint32_t CONTROL_RG_SIE=2;
  static const uint32_t CONTROL_RG_IE=1;
  static const uint32_t CONTROL_RG_INIT=0;

  static const uint32_t STATUS_RG_BOFF=7;
  static const uint32_t STATUS_RG_WARN=6;
  static const uint32_t STATUS_RG_WAKE=5;
  static const uint32_t STATUS_RG_RXOK=4;
  static const uint32_t STATUS_RG_TXOK=3;
  static const uint32_t STATUS_RG_LEC2=2;
  static const uint32_t STATUS_RG_LEC1=1;
  static const uint32_t STATUS_RG_LEC0=0;

  static const uint32_t CPU_IFC_RG_RSTST=7;
  static const uint32_t CPU_IFC_RG_DSC=6;
  static const uint32_t CPU_IFC_RG_DMC=5;
  static const uint32_t CPU_IFC_RG_PWD=4;
  static const uint32_t CPU_IFC_RG_SLEEP=3;
  static const uint32_t CPU_IFC_RG_MUX=2;
  static const uint32_t CPU_IFC_RG_CEN=0;

  static const uint32_t RG_SET=2;
  static const uint32_t RG_RES=1;
  static const uint32_t RG_UNC=3;
  static const uint32_t RG_NOT=0;
  static const uint32_t RG_MASK=3;

  static const uint32_t CONTROL0_RG_MSGVAL=6;
  static const uint32_t CONTROL0_RG_TXIE=4;
  static const uint32_t CONTROL0_RG_RXIE=2;
  static const uint32_t CONTROL0_RG_INTPND=0;

  static const uint32_t CONTROL1_RG_RMTPND=6;
  static const uint32_t CONTROL1_RG_TXRQST=4;
  static const uint32_t CONTROL1_RG_MSGLST=2;
  static const uint32_t CONTROL1_RG_NEWDAT=0;
  
  #pragma pack(1)
  struct SCANMessageControl
  {
   uint8_t Control0;
   uint8_t Control1;
  };//управляющие слова в структуре CAN
  struct SCANMessage
  {
   uint32_t Arbitration;
   uint8_t MessageConfiguration;
   uint8_t Data[8];
  };//сообщения в структуре CAN;
  struct SCANMemoryMap
  {
   uint8_t ControlRegister;
   uint8_t StatusRegister;
   uint8_t CPUInterfaceRegister;
   uint8_t Reserved0;
   uint8_t HightSpeedRead[2];
   uint8_t GlobalMaskStandard[2];
   uint8_t GlobalMaskExtended[4];
   uint8_t Message15Mask[4];
   SCANMessageControl sCANMessageControl1;
   SCANMessage sCANMessage1;
   uint8_t CLKOUTRegister;	
   SCANMessageControl sCANMessageControl2;
   SCANMessage sCANMessage2;
   uint8_t BusConfigRegister; 	
   SCANMessageControl sCANMessageControl3;
   SCANMessage sCANMessage3;
   uint8_t BitTimingRegister0; 	
   SCANMessageControl sCANMessageControl4;
   SCANMessage sCANMessage4;
   uint8_t BitTimingRegister1; 	
   SCANMessageControl sCANMessageControl5;
   SCANMessage sCANMessage5;
   uint8_t InterruptRegister; 	
   SCANMessageControl sCANMessageControl6;
   SCANMessage sCANMessage6;
   uint8_t Reserved1; 	
   SCANMessageControl sCANMessageControl7;
   SCANMessage sCANMessage7;
   uint8_t Reserved2; 	
   SCANMessageControl sCANMessageControl8;
   SCANMessage sCANMessage8;
   uint8_t Reserved3;  	
   SCANMessageControl sCANMessageControl9;
   SCANMessage sCANMessage9;
   uint8_t P1CONF; 	
   SCANMessageControl sCANMessageControl10;
   SCANMessage sCANMessage10;
   uint8_t P2CONF; 	
   SCANMessageControl sCANMessageControl11;
   SCANMessage sCANMessage11;
   uint8_t P1IN; 	
   SCANMessageControl sCANMessageControl12;
   SCANMessage sCANMessage12;
   uint8_t P2IN; 	
   SCANMessageControl sCANMessageControl13;
   SCANMessage sCANMessage13;
   uint8_t P1OUT; 	
   SCANMessageControl sCANMessageControl14;
   SCANMessage sCANMessage14;
   uint8_t P2OUT; 	
   SCANMessageControl sCANMessageControl15;
   SCANMessage sCANMessage15;
   uint8_t SerialResetAddress;
  };//структура CAN
  #pragma pack()
 
  //сообщение CAN - управляющее слово
  volatile SCANMessageControl *sCANMessageControl_Ptr[MESSAGE_AMOUNT];//управляющие слова в структуре CAN
  //сообщение CAN - данные
  volatile SCANMessage *sCANMessage_Ptr[MESSAGE_AMOUNT];//сообщения в структуре CAN;
  //структура памяти контроллера
  volatile SCANMemoryMap *sCANMemoryMap_Ptr;//структура CAN
  //настройки канала
  CCAN527CANMessage cCAN527CANMessage[MESSAGE_AMOUNT];//параметры слотов сообщений
   
  uint32_t GlobalMessageMask;//глобальная маска стандартных сообщений
  uint32_t GlobalExtendedMessageMask;//глобальная маска расширенных сообщений
  uint32_t Message15Mask;//маска 15-го сообщения
  CAN_SPEED Speed;//скорость обмена
   
  int32_t ISR_CANInterruptID;//идентификатор прерывания CAN 
   
  CMemoryControl cMemoryControl;//адрес канала
  uint32_t IRQ;//прерывание канала
  
  uint32_t ChannelIndex;//индекс канала
   
  bool TransmittIsDone[MESSAGE_AMOUNT];//завершена ли передача в канале в слоте
  uint64_t TransmittStartTime[MESSAGE_AMOUNT];//время начала передачи
  CUniquePtr<CRingBuffer<CCAN527CANPackage> > cRingBuffer_Transmitter_Ptr[MESSAGE_AMOUNT];//указатели на классы буферов передаваемых данных   
    
  CUniquePtr<CRingBuffer<CCAN527CANPackage> > &cRingBuffer_Receiver_Ptr;//ссылка на указатель на буфер приёма
  
  uint64_t BussOffTime;//время Buss Off
  
  uint64_t CPS;//частота процессора
 //-конструктор----------------------------------------------------------------------------------------
 public:
  CCAN527Channel(CUniquePtr<CRingBuffer<CCAN527CANPackage> > &cRingBuffer_Receiver_Ptr_Set,uint32_t transmitter_buffer_size=1);//конструктор
 //-деструктор-----------------------------------------------------------------------------------------
  ~CCAN527Channel();//деструктор
 //-открытые функции-----------------------------------------------------------------------------------
 public:
  bool Init(uintptr_t base_addr,uint32_t irq,uint32_t channel_index);//инициализировать канал
  void Release(void);//освободить ресурсы  
  uint8_t GetIRQ(void);//получить номер прерывания
  void ClearIRQ(void);//сбросить флаг прерывания
  bool CANInterruptAttach(sigevent *CANInterruptEvent);//подключить прерывание
  void CANInterruptDetach(void);//отключить прерывание
  bool SetChannelMessage(uint32_t message_index,const CCAN527CANMessage &cCAN527CANMessage_Set);//задать настройки сообщения
  bool SetChannelSpeed(CAN_SPEED speed);//задать скорость обмена на канале
  bool SetChannelMask(uint32_t global_message_mask,uint32_t global_extended_message_mask,uint32_t message_15_mask);//задать маски на канале
  bool ApplyChannelSettings();//применить заданные настройки к каналу
  bool TransmittPackage(const CCAN527CANPackage &cCAN527CANPackage);//добавить пакет для отправки
  void OnInterrupt(void);//обработчик прерывания  
  void TransmittProcessing(void);//выполнить передачу данных, если это возможно
  void ClearTransmitterBuffer(void);//очистить буфер передачи
  bool BussOffControl(void);//проверить работоспособность контроллера канала и сбросить канал при необходимости
  void Processing(void);//цикл обработки
 //-закрытые функции-----------------------------------------------------------------------------------
 private:
  bool IsWaitable(void);//получить, можно ли выполнять действия с каналом
  bool IsOverTransmittWaitTime(uint32_t message_index);//получить, закончилось ли время ожидания передачи
  void SetWaitState(void);//запустить счётчик запрета операций с каналом
  bool Reset(void);//выполнить сброс канала
  bool ApplyMessageMask(void);//применить заданные маски
  bool ApplyMessage(void);//применить заданные параметры сообщений
  void TransmittInterrupt(void);//обработчик прерывания по завершению передачи
  bool StartTransmittPackage(const CCAN527CANPackage &cCAN527CANPackage);//отправить сообщение
  void ReceiveInterrupt(void);//обработчик прерывания по получению пакетов
};
#endif


