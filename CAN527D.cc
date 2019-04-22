 //****************************************************************************************************
//Программа-эмулятор комплекса ИПК
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************

#include <stdint.h>
#include <sys/neutrino.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccan527.h"

//****************************************************************************************************
//сигналы
//****************************************************************************************************
void Signal_Exit(int32_t s_code);//уничтожение процесса

//****************************************************************************************************
//глобальные переменные
//****************************************************************************************************

//****************************************************************************************************
//прототипы функций
//****************************************************************************************************

//****************************************************************************************************
//функции
//****************************************************************************************************

//----------------------------------------------------------------------------------------------------
//основная функция программы
//----------------------------------------------------------------------------------------------------
int main(int32_t argc, char *argv[]) 
{
 //разрешаем доступ потоку к ресурсам аппаратуры
 ThreadCtl(_NTO_TCTL_IO,NULL);
 //подключим сигналы
 printf("Connecting signals.\r\n");
 signal(SIGINT,Signal_Exit);
 signal(SIGTERM,Signal_Exit);
 signal(SIGKILL,Signal_Exit);
 signal(SIGCLD,Signal_Exit);
 signal(SIGQUIT,Signal_Exit);

 CCAN527 cCAN527;
 
 uintptr_t base_addr_ch1=0xD0000;
 uintptr_t base_addr_ch2=base_addr_ch1+0x100;
 uint32_t irq_ch1=7;
 uint32_t irq_ch2=7;
 cCAN527.Init(base_addr_ch1,base_addr_ch2,irq_ch1,irq_ch2); 
 
 uint32_t arbitration=0x01;
 uint32_t arbitration_mask=0x0F;
 
 for(uint32_t channel=0;channel<2;channel++)
 {
  //настраиваем скорость каналов
  cCAN527.SetChannelSpeed(channel,CCAN527Channel::CAN_SPEED_1MBS);
  //настраиваем маски каналов
  cCAN527.SetChannelMask(channel,arbitration_mask,arbitration_mask,arbitration_mask);
  //настраиваем 15 слотов
  for(uint32_t message_index=0;message_index<15;message_index++)
  {
   uint32_t arb=message_index;
   bool received=true;
   if (message_index==0) received=false;//нулевой слот оставим для передачи 	
   cCAN527.SetChannelMessage(channel,message_index,CCAN527CANMessage(true,arb,received));
  }
 }
 
 CCAN527CANPackage cCAN527CANPackage(arbitration,8,0,0);
 for(uint8_t n=0;n<cCAN527CANPackage.Length;n++) cCAN527CANPackage.Data[n]=n; 

 int32_t index=0;
 while(1)
 {  
  cCAN527CANPackage.Data[0]=index&0xff;
  cCAN527CANPackage.ChannelIndex=index%2;
  cCAN527.TransmittPackage(cCAN527CANPackage);
  index++;
  delay(1000);
  std::vector<CCAN527CANPackage> vector_CCAN527CANPackage;
  cCAN527.GetAllReceivedPackage(vector_CCAN527CANPackage);
  size_t size=vector_CCAN527CANPackage.size();
  if (size>0) 
  {
   printf("Received %i \r\n",size);
   for(size_t v=0;v<size;v++)
   {
    printf("Channel %i ",vector_CCAN527CANPackage[v].ChannelIndex);
    printf("Arb:%08x ",vector_CCAN527CANPackage[v].Arbitration);
    printf("Len:%02x Data:",vector_CCAN527CANPackage[v].Length);
    for(uint8_t n=0;n<vector_CCAN527CANPackage[v].Length;n++) printf("%02x ",vector_CCAN527CANPackage[v].Data[n]);
    printf("\r\n");
   }
  }
 } 
 return(EXIT_SUCCESS);
}

//****************************************************************************************************
//сигналы
//****************************************************************************************************

//----------------------------------------------------------------------------------------------------
//уничтожение подчинённого процесса
//----------------------------------------------------------------------------------------------------
void Signal_Exit(int32_t s_code)
{
 printf("Receive signal TERM.\r\n");
 exit(EXIT_SUCCESS);
}