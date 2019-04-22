//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include "ccan527channel.h"
#include <string.h>
#include <time.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//конструктор и деструктор класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//конструктор
//----------------------------------------------------------------------------------------------------
CCAN527Channel::CCAN527Channel(CUniquePtr<CRingBuffer<CCAN527CANPackage> > &cRingBuffer_Receiver_Ptr_Set,uint32_t transmitter_buffer_size):cRingBuffer_Receiver_Ptr(cRingBuffer_Receiver_Ptr_Set)
{
 CPS=SYSPAGE_ENTRY(qtime)->cycles_per_sec;	
 ISR_CANInterruptID=-1;	
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  cRingBuffer_Transmitter_Ptr[n].Set(new CRingBuffer<CCAN527CANPackage>(transmitter_buffer_size));
  TransmittIsDone[n]=true;
  TransmittStartTime[n]=0;
 }
 BussOffTime=0;
}
//----------------------------------------------------------------------------------------------------
//деструктор
//----------------------------------------------------------------------------------------------------
CCAN527Channel::~CCAN527Channel()
{
 Release();	
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  cRingBuffer_Transmitter_Ptr[n].Release();
 }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//закрытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//получить, можно ли выполнять действия с каналом
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::IsWaitable(void)
{
 long double delta=(1000.0*(ClockCycles()-BussOffTime))/CPS;
 if (delta<static_cast<long double>(CAN_CHANNEL_BUSS_OFF_DELAY_MS)) return(true);//слишком рано
 return(false);
}

//----------------------------------------------------------------------------------------------------
//получить, закончилось ли время ожидания передачи
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::IsOverTransmittWaitTime(uint32_t message_index)
{
 long double delta=(1000.0*(ClockCycles()-TransmittStartTime[message_index]))/CPS;
 if (delta<static_cast<long double>(TRANSMITT_WAIT_TIME_MS)) return(false);//время ожидания канала ещё не вышло
 return(true);
}

//----------------------------------------------------------------------------------------------------
//запустить счётчик запрета операций с каналом
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::SetWaitState(void)
{
 BussOffTime=ClockCycles();
}

//----------------------------------------------------------------------------------------------------
//сбросить канал
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::Reset(void)
{
 //делаем сброс канала
 sCANMemoryMap_Ptr->ControlRegister=(1<<CONTROL_RG_CCE)|(0<<CONTROL_RG_EIE)|(0<<CONTROL_RG_SIE)|(1<<CONTROL_RG_IE)|(1<<CONTROL_RG_INIT);
 sCANMemoryMap_Ptr->ControlRegister=(1<<CONTROL_RG_CCE)|(0<<CONTROL_RG_EIE)|(0<<CONTROL_RG_SIE)|(1<<CONTROL_RG_IE)|(0<<CONTROL_RG_INIT);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//применить заданные маски
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::ApplyMessageMask(void)
{
 sCANMemoryMap_Ptr->GlobalMaskStandard[0]=GlobalMessageMask&0xFF;
 sCANMemoryMap_Ptr->GlobalMaskStandard[1]=(GlobalMessageMask>>8)&0xFF;

 sCANMemoryMap_Ptr->GlobalMaskExtended[0]=GlobalExtendedMessageMask&0xFF;
 sCANMemoryMap_Ptr->GlobalMaskExtended[1]=(GlobalExtendedMessageMask>>8)&0xFF;
 sCANMemoryMap_Ptr->GlobalMaskExtended[2]=(GlobalExtendedMessageMask>>16)&0xFF;
 sCANMemoryMap_Ptr->GlobalMaskExtended[3]=(GlobalExtendedMessageMask>>24)&0xFF;

 sCANMemoryMap_Ptr->Message15Mask[0]=Message15Mask&0xFF;
 sCANMemoryMap_Ptr->Message15Mask[1]=(Message15Mask>>8)&0xFF;
 sCANMemoryMap_Ptr->Message15Mask[2]=(Message15Mask>>16)&0xFF;
 sCANMemoryMap_Ptr->Message15Mask[3]=(Message15Mask>>24)&0xFF;

 return(true);	
}
//----------------------------------------------------------------------------------------------------
//применить заданные параметры сообщений
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::ApplyMessage(void)
{
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  CCAN527CANMessage *cCAN527CANMessage_Ptr=&cCAN527CANMessage[n];
  
  sCANMessage_Ptr[n]->Arbitration=cCAN527CANMessage_Ptr->Arbitration;
  sCANMessageControl_Ptr[n]->Control1=(RG_RES<<CONTROL1_RG_RMTPND)|(RG_RES<<CONTROL1_RG_TXRQST)|(RG_RES<<CONTROL1_RG_MSGLST)|(RG_RES<<CONTROL1_RG_NEWDAT);
  if (cCAN527CANMessage_Ptr->IsReceiveMode()==true)//режим приёма
  {
   if (cCAN527CANMessage_Ptr->IsExtendedMode()==true) sCANMessage_Ptr[n]->MessageConfiguration=BIN8(1,0,0,0,0,1,0,0);
                                                 else sCANMessage_Ptr[n]->MessageConfiguration=BIN8(1,0,0,0,0,0,0,0);
   sCANMessageControl_Ptr[n]->Control0=(RG_SET<<CONTROL0_RG_MSGVAL)|(RG_RES<<CONTROL0_RG_TXIE)|(RG_SET<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
  }
  else//режим передачи
  {
   if (cCAN527CANMessage_Ptr->IsExtendedMode()==true) sCANMessage_Ptr[n]->MessageConfiguration=BIN8(1,0,0,0,1,1,0,0);
                                                 else sCANMessage_Ptr[n]->MessageConfiguration=BIN8(1,0,0,0,1,0,0,0);
   sCANMessageControl_Ptr[n]->Control0=(RG_SET<<CONTROL0_RG_MSGVAL)|(RG_SET<<CONTROL0_RG_TXIE)|(RG_RES<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
  }
 }
 return(true);	
}
//----------------------------------------------------------------------------------------------------
//обработчик прерывания по завершению передачи
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::TransmittInterrupt(void)
{
 //узнаем, с чем связано прерывание
 uint8_t ir=sCANMemoryMap_Ptr->InterruptRegister;
 if (ir==1)//прерывание по смене состояния (регистр статуса ОБЯЗАТЕЛЬНО НУЖНО прочитать)
 {
  uint8_t sr=sCANMemoryMap_Ptr->StatusRegister;
  ir=0;
 }
 if (ir!=0)//прерывание в слоте
 {
  if (ir==2) ir=15;
        else ir-=2;          
 }
 if (ir>=1 && ir<=15)
 {
  //требуется сбросить CONTROL0_RG_INTPND
  sCANMessageControl_Ptr[ir-1]->Control0=(RG_UNC<<CONTROL0_RG_MSGVAL)|(RG_UNC<<CONTROL0_RG_TXIE)|(RG_UNC<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
 }
 TransmittProcessing();
}
//----------------------------------------------------------------------------------------------------
//отправить сообщение
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::StartTransmittPackage(const CCAN527CANPackage &cCAN527CANPackage)
{
 uint32_t message_index=cCAN527CANPackage.MessageIndex;
 if (cCAN527CANMessage[message_index].IsReceiveMode()==true) return(false);//этот слот для приёма данных 
 //настраиваем слово управления, чтобы остановить возможную передачу
 sCANMessageControl_Ptr[message_index]->Control0=(RG_RES<<CONTROL0_RG_MSGVAL)|(RG_RES<<CONTROL0_RG_TXIE)|(RG_RES<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
 sCANMessageControl_Ptr[message_index]->Control1=(RG_SET<<CONTROL1_RG_RMTPND)|(RG_RES<<CONTROL1_RG_TXRQST)|(RG_RES<<CONTROL1_RG_MSGLST)|(RG_RES<<CONTROL1_RG_NEWDAT);
 //записываем нужные данные в поля контроллера 
 sCANMemoryMap_Ptr->StatusRegister=(0<<STATUS_RG_BOFF)|(0<<STATUS_RG_WARN)|(0<<STATUS_RG_WAKE)|(0<<STATUS_RG_RXOK)|(0<<STATUS_RG_TXOK)|(0<<STATUS_RG_LEC2)|(0<<STATUS_RG_LEC1)|(0<<STATUS_RG_LEC0);
 cCAN527CANMessage[message_index].Arbitration=cCAN527CANPackage.Arbitration;
 sCANMessage_Ptr[message_index]->Arbitration=cCAN527CANPackage.Arbitration;
 uint32_t extendedmode=0;
 if (cCAN527CANMessage[message_index].IsExtendedMode()==true) extendedmode=1;
 sCANMessage_Ptr[message_index]->MessageConfiguration=(cCAN527CANPackage.Length<<4)|(1<<3)|(extendedmode<<2);
 memcpy(const_cast<uint8_t *>(sCANMessage_Ptr[message_index]->Data),cCAN527CANPackage.Data,cCAN527CANPackage.Length);//копируем данные
 //настраиваем слово управления для передачи
 sCANMessageControl_Ptr[message_index]->Control1=(RG_RES<<CONTROL1_RG_RMTPND)|(RG_SET<<CONTROL1_RG_TXRQST)|(RG_RES<<CONTROL1_RG_MSGLST)|(RG_SET<<CONTROL1_RG_NEWDAT);
 sCANMessageControl_Ptr[message_index]->Control0=(RG_SET<<CONTROL0_RG_MSGVAL)|(RG_SET<<CONTROL0_RG_TXIE)|(RG_RES<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
 
 TransmittIsDone[message_index]=false; 
 TransmittStartTime[message_index]=ClockCycles();
 return(true);
}


//----------------------------------------------------------------------------------------------------
//обработчик прерывания по получению пакетов
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::ReceiveInterrupt(void)
{
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  if (cCAN527CANMessage[n].IsReceiveMode()==false) continue;//этот слот для передачи
  if (((sCANMessageControl_Ptr[n]->Control1>>CONTROL1_RG_NEWDAT)&RG_MASK)==RG_RES) continue;//нет новых данных
  uint32_t arbitration=sCANMessage_Ptr[n]->Arbitration;
  uint32_t length=(sCANMessage_Ptr[n]->MessageConfiguration&BIN8(1,1,1,1,0,0,0,0))>>4;
  CCAN527CANPackage cCAN527CANPackage(arbitration,length,ChannelIndex,n);
  memcpy(cCAN527CANPackage.Data,const_cast<uint8_t*>(sCANMessage_Ptr[n]->Data),8);
  //очищаем сообщение
  sCANMessageControl_Ptr[n]->Control0=(RG_SET<<CONTROL0_RG_MSGVAL)|(RG_UNC<<CONTROL0_RG_TXIE)|(RG_UNC<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
  sCANMessageControl_Ptr[n]->Control1=(RG_UNC<<CONTROL1_RG_RMTPND)|(RG_RES<<CONTROL1_RG_TXRQST)|(RG_RES<<CONTROL1_RG_MSGLST)|(RG_RES<<CONTROL1_RG_NEWDAT);
  cRingBuffer_Receiver_Ptr.Get()->Push(cCAN527CANPackage);
 }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//открытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//инициализировать канал
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::Init(uintptr_t base_addr,uint32_t irq,uint32_t channel_index)
{
 Release();	
 if (MESSAGE_AMOUNT!=15) return(false);
 ChannelIndex=channel_index;
 //создаём класс работы с адресами
 if (cMemoryControl.SetAddr(base_addr,CHANNEL_MEMORY_SIZE)==false) return(false); 	
 IRQ=irq;
 //получим адреса платы
 sCANMemoryMap_Ptr=reinterpret_cast<volatile SCANMemoryMap *>(cMemoryControl.GetAddr());
 //сообщения
 sCANMessage_Ptr[0]=&sCANMemoryMap_Ptr->sCANMessage1;
 sCANMessage_Ptr[1]=&sCANMemoryMap_Ptr->sCANMessage2;
 sCANMessage_Ptr[2]=&sCANMemoryMap_Ptr->sCANMessage3;
 sCANMessage_Ptr[3]=&sCANMemoryMap_Ptr->sCANMessage4;
 sCANMessage_Ptr[4]=&sCANMemoryMap_Ptr->sCANMessage5;
 sCANMessage_Ptr[5]=&sCANMemoryMap_Ptr->sCANMessage6;
 sCANMessage_Ptr[6]=&sCANMemoryMap_Ptr->sCANMessage7;
 sCANMessage_Ptr[7]=&sCANMemoryMap_Ptr->sCANMessage8;
 sCANMessage_Ptr[8]=&sCANMemoryMap_Ptr->sCANMessage9;
 sCANMessage_Ptr[9]=&sCANMemoryMap_Ptr->sCANMessage10;
 sCANMessage_Ptr[10]=&sCANMemoryMap_Ptr->sCANMessage11;
 sCANMessage_Ptr[11]=&sCANMemoryMap_Ptr->sCANMessage12;
 sCANMessage_Ptr[12]=&sCANMemoryMap_Ptr->sCANMessage13;
 sCANMessage_Ptr[13]=&sCANMemoryMap_Ptr->sCANMessage14;
 sCANMessage_Ptr[14]=&sCANMemoryMap_Ptr->sCANMessage15;
 //и их управляющие слова
 sCANMessageControl_Ptr[0]=&sCANMemoryMap_Ptr->sCANMessageControl1;
 sCANMessageControl_Ptr[1]=&sCANMemoryMap_Ptr->sCANMessageControl2;
 sCANMessageControl_Ptr[2]=&sCANMemoryMap_Ptr->sCANMessageControl3;
 sCANMessageControl_Ptr[3]=&sCANMemoryMap_Ptr->sCANMessageControl4;
 sCANMessageControl_Ptr[4]=&sCANMemoryMap_Ptr->sCANMessageControl5;
 sCANMessageControl_Ptr[5]=&sCANMemoryMap_Ptr->sCANMessageControl6;
 sCANMessageControl_Ptr[6]=&sCANMemoryMap_Ptr->sCANMessageControl7;
 sCANMessageControl_Ptr[7]=&sCANMemoryMap_Ptr->sCANMessageControl8;
 sCANMessageControl_Ptr[8]=&sCANMemoryMap_Ptr->sCANMessageControl9;
 sCANMessageControl_Ptr[9]=&sCANMemoryMap_Ptr->sCANMessageControl10;
 sCANMessageControl_Ptr[10]=&sCANMemoryMap_Ptr->sCANMessageControl11;
 sCANMessageControl_Ptr[11]=&sCANMemoryMap_Ptr->sCANMessageControl12;
 sCANMessageControl_Ptr[12]=&sCANMemoryMap_Ptr->sCANMessageControl13;
 sCANMessageControl_Ptr[13]=&sCANMemoryMap_Ptr->sCANMessageControl14;
 sCANMessageControl_Ptr[14]=&sCANMemoryMap_Ptr->sCANMessageControl15;
   
 //маски
 GlobalMessageMask=0xFFFF;
 GlobalExtendedMessageMask=0xFFFFFFFF;
 Message15Mask=0xFFFFFFFF;
  
 Speed=CAN_SPEED_1MBS;
 
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++) 
 {
  TransmittIsDone[n]=true;
  cRingBuffer_Transmitter_Ptr[n].Get()->Reset();  
 }
 
 BussOffTime=0;
 return(true); 
}
//----------------------------------------------------------------------------------------------------
//освободить ресурсы
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::Release(void)
{
 cMemoryControl.Release();   
}
//----------------------------------------------------------------------------------------------------
//получить номер прерывания
//----------------------------------------------------------------------------------------------------
uint8_t CCAN527Channel::GetIRQ(void)
{
 return(IRQ);
}
//----------------------------------------------------------------------------------------------------
//сбросить флаг прерывания
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::ClearIRQ(void)
{
 InterruptUnmask(GetIRQ(),ISR_CANInterruptID);
}
//----------------------------------------------------------------------------------------------------
//подключить прерывание
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::CANInterruptAttach(sigevent *CANInterruptEvent)
{
 //подключаем прерывание
 SIGEV_INTR_INIT(CANInterruptEvent);//настраиваем событие  
 ISR_CANInterruptID=InterruptAttachEvent(GetIRQ(),CANInterruptEvent,_NTO_INTR_FLAGS_END);
 if (ISR_CANInterruptID<0) return(false);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//отключить прерывание
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::CANInterruptDetach(void)
{
 if (ISR_CANInterruptID>=0) InterruptDetach(ISR_CANInterruptID);//отключим прерывание
 ISR_CANInterruptID=-1; 
}
//----------------------------------------------------------------------------------------------------
//задать настройки сообщения
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::SetChannelMessage(uint32_t message_index,const CCAN527CANMessage &cCAN527CANMessage_Set)
{
 cCAN527CANMessage[message_index]=cCAN527CANMessage_Set;
 return(true);
}
//----------------------------------------------------------------------------------------------------
//задать скорость обмена на канале
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::SetChannelSpeed(CAN_SPEED speed)
{
 Speed=speed;
 return(true);
}
//----------------------------------------------------------------------------------------------------
//задать маски на канале
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::SetChannelMask(uint32_t global_message_mask,uint32_t global_extended_message_mask,uint32_t message_15_mask)
{
 GlobalMessageMask=global_message_mask;
 GlobalExtendedMessageMask=global_extended_message_mask;
 Message15Mask=message_15_mask;
 return(true);
}
//----------------------------------------------------------------------------------------------------
//применить заданные настройки к каналу
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::ApplyChannelSettings(void)
{
 uint8_t btr0=0;
 if (Speed==CAN_SPEED_1MBS) btr0=BIN8(0,1,0,0,0,0,0,0);
 if (Speed==CAN_SPEED_500KBS) btr0=BIN8(0,1,0,0,0,0,0,1);
 if (Speed==CAN_SPEED_250KBS) btr0=BIN8(0,1,0,0,0,0,1,1);
 if (Speed==CAN_SPEED_125KBS) btr0=BIN8(0,1,0,0,0,1,1,1);
 if (btr0==0) return(false);
 //настраиваем канал
 if (Reset()==false) return(false);
 sCANMemoryMap_Ptr->ControlRegister=(1<<CONTROL_RG_CCE)|(0<<CONTROL_RG_EIE)|(0<<CONTROL_RG_SIE)|(0<<CONTROL_RG_IE)|(0<<CONTROL_RG_INIT);
 sCANMemoryMap_Ptr->StatusRegister=(0<<STATUS_RG_BOFF)|(0<<STATUS_RG_WARN)|(0<<STATUS_RG_WAKE)|(0<<STATUS_RG_RXOK)|(0<<STATUS_RG_TXOK)|(0<<STATUS_RG_LEC2)|(0<<STATUS_RG_LEC1)|(0<<STATUS_RG_LEC0);
 sCANMemoryMap_Ptr->CPUInterfaceRegister=(0<<CPU_IFC_RG_RSTST)|(0<<CPU_IFC_RG_DSC)|(0<<CPU_IFC_RG_DMC)|(0<<CPU_IFC_RG_PWD)|(0<<CPU_IFC_RG_SLEEP)|(0<<CPU_IFC_RG_MUX)|(1<<CPU_IFC_RG_CEN);
 sCANMemoryMap_Ptr->BitTimingRegister0=btr0;
 sCANMemoryMap_Ptr->BitTimingRegister1=BIN8(0,0,1,0,0,0,1,1);
 //настраиваем маски сообщений
 ApplyMessageMask();
 //настраиваем слоты сообщений
 ApplyMessage();
 
 sCANMemoryMap_Ptr->ControlRegister=(0<<CONTROL_RG_CCE)|(0<<CONTROL_RG_EIE)|(0<<CONTROL_RG_SIE)|(1<<CONTROL_RG_IE)|(0<<CONTROL_RG_INIT);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//добавить пакет для отправки
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::TransmittPackage(const CCAN527CANPackage &cCAN527CANPackage)
{
 uint32_t message_index=cCAN527CANPackage.MessageIndex;
 if (cCAN527CANMessage[message_index].IsReceiveMode()==true) return(false);//этот слот для приёма данных
 cRingBuffer_Transmitter_Ptr[message_index].Get()->Push(cCAN527CANPackage);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//обработчик прерывания
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::OnInterrupt(void)
{
 ReceiveInterrupt();
 TransmittInterrupt();
 ClearIRQ();
}
//----------------------------------------------------------------------------------------------------
//выполнить передачу данных, если это возможно
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::TransmittProcessing(void)
{
 if (IsWaitable()==true) return;//мы отрабатываем Buss Off, передача запрещена
 
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  if (cCAN527CANMessage[n].IsReceiveMode()==true) continue;//этот слот для приёма
  if (((sCANMessageControl_Ptr[n]->Control1>>CONTROL1_RG_TXRQST)&RG_MASK)==RG_RES) TransmittIsDone[n]=true;//сообщение передано
  if (IsOverTransmittWaitTime(n)==true) TransmittIsDone[n]=true;
  if (TransmittIsDone[n]==false) continue;  
  //читаем список для передачи
  CCAN527CANPackage cCAN527CANPackage; 
  if (cRingBuffer_Transmitter_Ptr[n].Get()->Pop(cCAN527CANPackage)==false) continue;//нечего передавать
  //запускаем передачу
  StartTransmittPackage(cCAN527CANPackage);
 }
}
//----------------------------------------------------------------------------------------------------
//очистить буфер передачи
//----------------------------------------------------------------------------------------------------
void CCAN527Channel::ClearTransmitterBuffer(void)
{
 for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
 {
  cRingBuffer_Transmitter_Ptr[n].Get()->Reset();
  TransmittIsDone[n]=true;  
 }
}
//----------------------------------------------------------------------------------------------------
//проверить работоспособность контроллера канала и сбросить канал при необходимости
//----------------------------------------------------------------------------------------------------
bool CCAN527Channel::BussOffControl(void)
{
 uint8_t st_rg=sCANMemoryMap_Ptr->StatusRegister;
 if ((st_rg&(1<<STATUS_RG_BOFF))!=0)//BussOff
 {
  if (IsWaitable()==true) return(true);//мы уже отрабатываем Buss Off
  static int32_t counter=0;
  counter++;
  if ((st_rg&(1<<STATUS_RG_BOFF))!=0)
  {
   time_t time_main=time(NULL);
   tm *tm_main=localtime(&time_main);
   printf("[%02i.%02i.%04i %02i:%02i:%02i] ChannelIndex:%lx Buss Off: %ld!\r\n",tm_main->tm_mday,tm_main->tm_mon+1,tm_main->tm_year+1900,tm_main->tm_hour,tm_main->tm_min,tm_main->tm_sec,ChannelIndex,static_cast<long>(counter));
  }
  //if ((st_rg&(1<<STATUS_RG_WARN))!=0) printf("Warning: %ld!\r\n",counter);
  //отменяем передачу данных
  for(uint32_t n=0;n<MESSAGE_AMOUNT;n++)
  {
   //очищаем ошибки сообщения
   sCANMessageControl_Ptr[n]->Control0=(RG_SET<<CONTROL0_RG_MSGVAL)|(RG_RES<<CONTROL0_RG_TXIE)|(RG_RES<<CONTROL0_RG_RXIE)|(RG_RES<<CONTROL0_RG_INTPND);
   sCANMessageControl_Ptr[n]->Control1=(RG_RES<<CONTROL1_RG_RMTPND)|(RG_RES<<CONTROL1_RG_TXRQST)|(RG_RES<<CONTROL1_RG_MSGLST)|(RG_RES<<CONTROL1_RG_NEWDAT);
  }  
  //переинициализируем канал
  ApplyChannelSettings();
  SetWaitState();
  return(false);
 }
 return(true);
}