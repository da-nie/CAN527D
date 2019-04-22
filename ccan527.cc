//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include "ccan527.h"
#include "craiicmutex.h"
#include <string.h>
#include <unistd.h>

#include <stdio.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//конструктор и деструктор класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//конструктор
//----------------------------------------------------------------------------------------------------
CCAN527::CCAN527(void)
{
 cThread_Ptr.Set(NULL);
 cCAN527ProtectedPart_Ptr.Set(new CCAN527ProtectedPart(RECEIVER_RING_BUFFER_SIZE,TRANSMITTER_RING_BUFFER_SIZE));
}
//----------------------------------------------------------------------------------------------------
//деструктор
//----------------------------------------------------------------------------------------------------
CCAN527::~CCAN527()
{
 Release();
 cCAN527ProtectedPart_Ptr.Release();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//закрытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//получить, допустим ли такой номер канала
//----------------------------------------------------------------------------------------------------
bool CCAN527::IsChannelValid(uint32_t channel)
{
 if (channel>=CCAN527ProtectedPart::CAN_CHANNEL_AMOUNT) return(false);
 return(true);	
}

//----------------------------------------------------------------------------------------------------
//подключиться к прерыванию
//----------------------------------------------------------------------------------------------------
bool CCAN527::CANInterruptAttach(void)
{
 CANInterruptDetach();
 bool ret=true;
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 { 	
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);  
  for(uint32_t n=0;n<CCAN527ProtectedPart::CAN_CHANNEL_AMOUNT;n++)
  {
   if (cCAN527ProtectedPart_Ptr.Get()->CANInterruptAttach(&CANInterruptEvent,n)==false) ret=false;
  }
 }
 return(ret);
}
//----------------------------------------------------------------------------------------------------
//отключиться от прерывания
//----------------------------------------------------------------------------------------------------
void CCAN527::CANInterruptDetach(void)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  for(uint32_t n=0;n<CCAN527ProtectedPart::CAN_CHANNEL_AMOUNT;n++) cCAN527ProtectedPart_Ptr.Get()->CANInterruptDetach(n); 	
 }
}
//----------------------------------------------------------------------------------------------------
//получить, нужно ли выходить из потока
//----------------------------------------------------------------------------------------------------
bool CCAN527::IsExitThread(void)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  return(cCAN527ProtectedPart_Ptr.Get()->IsExitThread());
 } 
}
//----------------------------------------------------------------------------------------------------
//задать, нужно ли выходить из потока
//----------------------------------------------------------------------------------------------------
void CCAN527::SetExitThread(bool state)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {   	
  cCAN527ProtectedPart_Ptr.Get()->SetExitThread(state);
 }
}
//----------------------------------------------------------------------------------------------------
//запустить поток
//----------------------------------------------------------------------------------------------------
void CCAN527::StartThread(void)
{
 StopThread();
 SetExitThread(false);
 {
  CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
  {   	
   if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  }
 }
 cThread_Ptr.Set(new CThread(ThreadFunction,THREAD_PRIORITY,this)); 
}
//----------------------------------------------------------------------------------------------------
//остановить поток
//----------------------------------------------------------------------------------------------------
void CCAN527::StopThread(void)
{
 SetExitThread(true);
 {
  CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
  {   	
   if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  }
 }
 if (cThread_Ptr.Get()!=NULL)
 {
  cThread_Ptr.Get()->Join();
  cThread_Ptr.Set(NULL);
 }
}

//----------------------------------------------------------------------------------------------------
//выполнить обработку прерывания
//----------------------------------------------------------------------------------------------------
void CCAN527::OnInterrupt(void)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  for(uint32_t n=0;n<CCAN527ProtectedPart::CAN_CHANNEL_AMOUNT;n++)
  {
   cCAN527ProtectedPart_Ptr.Get()->OnInterrupt(n);
  }
 }
}
//----------------------------------------------------------------------------------------------------
//обработка платы до прерывания
//----------------------------------------------------------------------------------------------------
bool CCAN527::Processing(void)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsExitThread()==true) return(false);
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(true);
  //запускаем передачу данных на каналах, если она возможна, при этом проверяем состояния канала
  for(uint32_t n=0;n<CCAN527ProtectedPart::CAN_CHANNEL_AMOUNT;n++)
  {
   cCAN527ProtectedPart_Ptr.Get()->BussOffControl(n);
   cCAN527ProtectedPart_Ptr.Get()->TransmittProcessing(n);
  }
 }
 return(true); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//открытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//инициализировать плату
//----------------------------------------------------------------------------------------------------
bool CCAN527::Init(uintptr_t base_addr_ch1,uintptr_t base_addr_ch2,uint32_t irq_ch1,uint32_t irq_ch2)
{
 Release();
 //подключаемся к плате
 {
  CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
  {
   if (cCAN527ProtectedPart_Ptr.Get()->Init(base_addr_ch1,base_addr_ch2,irq_ch1,irq_ch2)==false)
   {
    cRAIICMutex.Unlock();
    Release();
    return(false);	
   }
  }
 }
 //запускаем поток
 StartThread();
 return(true);
}
//----------------------------------------------------------------------------------------------------
//освободить ресурсы
//----------------------------------------------------------------------------------------------------
void CCAN527::Release(void)
{
 StopThread();
 {
  CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
  {
   if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==true) cCAN527ProtectedPart_Ptr.Get()->Release();
  }
 }
}

//----------------------------------------------------------------------------------------------------
//получить все принятые пакеты
//----------------------------------------------------------------------------------------------------
void CCAN527::GetAllReceivedPackage(std::vector<CCAN527CANPackage> &vector_CCAN527CANPackage)
{
 vector_CCAN527CANPackage.clear();
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  while(1)
  {
   CCAN527CANPackage cCAN527CANPackage;
   if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
   if (cCAN527ProtectedPart_Ptr.Get()->GetReceivedPackage(cCAN527CANPackage)==false) break;
   vector_CCAN527CANPackage.push_back(cCAN527CANPackage);
  }
 }
}
//----------------------------------------------------------------------------------------------------
//получить не больше заданного количества принятых пакетов
//----------------------------------------------------------------------------------------------------
void CCAN527::GetPartReceivedPackage(std::vector<CCAN527CANPackage> &vector_CCAN527CANPackage,size_t counter)
{
 vector_CCAN527CANPackage.clear();
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  while(counter>0)
  {
   CCAN527CANPackage cCAN527CANPackage;
   if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
   if (cCAN527ProtectedPart_Ptr.Get()->GetReceivedPackage(cCAN527CANPackage)==false) break;
   vector_CCAN527CANPackage.push_back(cCAN527CANPackage);
   counter--;
  }
 }
}
//----------------------------------------------------------------------------------------------------
//отправить пакет
//----------------------------------------------------------------------------------------------------
bool CCAN527::TransmittPackage(const CCAN527CANPackage &cCAN527CANPackage)
{
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);
  if (cCAN527CANPackage.MessageIndex>=CCAN527Channel::MESSAGE_AMOUNT) return(false);
  if (IsChannelValid(cCAN527CANPackage.ChannelIndex)==false) return(false);
  if (cCAN527CANPackage.Length>8) return(false);  
  
  return(cCAN527ProtectedPart_Ptr.Get()->TransmittPackage(cCAN527CANPackage));
 }
}
//----------------------------------------------------------------------------------------------------
//задать настройки сообщения
//----------------------------------------------------------------------------------------------------
bool CCAN527::SetChannelMessage(uint32_t channel,uint32_t message_index,const CCAN527CANMessage &cCAN527CANMessage)
{
 if (IsChannelValid(channel)==false) return(false);
 if (message_index>=CCAN527Channel::MESSAGE_AMOUNT) return(false);
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);  
  return(cCAN527ProtectedPart_Ptr.Get()->SetChannelMessage(channel,message_index,cCAN527CANMessage));
 }
}
//----------------------------------------------------------------------------------------------------
//задать скорость обмена на канале
//----------------------------------------------------------------------------------------------------
bool CCAN527::SetChannelSpeed(uint32_t channel,CCAN527Channel::CAN_SPEED speed)
{
 if (IsChannelValid(channel)==false) return(false);
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);  
  return(cCAN527ProtectedPart_Ptr.Get()->SetChannelSpeed(channel,speed));
 }
}
//----------------------------------------------------------------------------------------------------
//задать маски на канале
//----------------------------------------------------------------------------------------------------
bool CCAN527::SetChannelMask(uint32_t channel,uint32_t global_message_mask,uint32_t global_extended_message_mask,uint32_t message_15_mask)
{
 if (IsChannelValid(channel)==false) return(false);
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);  
  return(cCAN527ProtectedPart_Ptr.Get()->SetChannelMask(channel,global_message_mask,global_extended_message_mask,message_15_mask));
 }
}
//----------------------------------------------------------------------------------------------------
//применить заданные настройки к каналу
//----------------------------------------------------------------------------------------------------
bool CCAN527::ApplyChannelSettings(uint32_t channel)
{
 if (IsChannelValid(channel)==false) return(false);
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return(false);  
  return(cCAN527ProtectedPart_Ptr.Get()->ApplyChannelSettings(channel));
 }
}
//----------------------------------------------------------------------------------------------------
//очистить буфер приёма
//----------------------------------------------------------------------------------------------------
void CCAN527::ClearReceiverBuffer(uint32_t channel)
{
 if (IsChannelValid(channel)==false) return;
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  cCAN527ProtectedPart_Ptr.Get()->ClearReceiverBuffer(channel);
 }
}
//----------------------------------------------------------------------------------------------------
//очистить буфер передачи
//----------------------------------------------------------------------------------------------------
void CCAN527::ClearTransmitterBuffer(uint32_t channel)
{
 if (IsChannelValid(channel)==false) return;
 CRAIICMutex cRAIICMutex(&cCAN527ProtectedPart_Ptr.Get()->cMutex);
 {
  if (cCAN527ProtectedPart_Ptr.Get()->IsEnabled()==false) return;
  cCAN527ProtectedPart_Ptr.Get()->ClearTransmitterBuffer(channel);
 }
}

//****************************************************************************************************
//прототипы функций
//****************************************************************************************************

static bool WaitInterrupt(void);//ожидание прерывания

//****************************************************************************************************
//функции
//****************************************************************************************************

//----------------------------------------------------------------------------------------------------
//функция потока
//----------------------------------------------------------------------------------------------------
static void* ThreadFunction(void *param)
{
 //разрешаем доступ потоку к ресурсам аппаратуры
 ThreadCtl(_NTO_TCTL_IO,NULL);
 if (param==NULL) return(NULL);	
 CCAN527 *cCAN527_Ptr=reinterpret_cast<CCAN527*>(param); 
 //подключаем прерывание
 cCAN527_Ptr->CANInterruptAttach();
 while(1)
 {
  if (cCAN527_Ptr->Processing()==false) break;//выходим из потока
  if (WaitInterrupt()==false) continue;//ждём прерывания
  cCAN527_Ptr->OnInterrupt();//выполняем обработчик прерывания  
 }
 cCAN527_Ptr->CANInterruptDetach(); 
 return(NULL);	
}

//----------------------------------------------------------------------------------------------------
//ожидание прерывания
//----------------------------------------------------------------------------------------------------
bool WaitInterrupt(void)
{
 //задаём таймер ожидания прерывания
 sigevent event_timeout;//событие "время вышло"
 uint64_t timeout=10000;
 SIGEV_UNBLOCK_INIT(&event_timeout);
 TimerTimeout(CLOCK_REALTIME,_NTO_TIMEOUT_INTR,&event_timeout,&timeout,NULL);//тайм-аут ядра на ожидание прерывания
 //ждём прерывания
 if (InterruptWait(0,NULL)<0) return(false);//прерывание не поступало
 return(true);	
}
