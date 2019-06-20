//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include "cmemorycontrol.h"

#include <hw/inout.h>
#include <sys/mman.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//конструктор и деструктор класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//конструктор
//----------------------------------------------------------------------------------------------------
CMemoryControl::CMemoryControl(void)
{
 PhisicalAddr=0;
 Size=0;
 VirtualAddr=MAP_FAILED;
}

CMemoryControl::CMemoryControl(uint64_t phisical_addr,uint64_t size)
{
 PhisicalAddr=0;
 Size=0;
 VirtualAddr=MAP_FAILED;
 SetAddr(phisical_addr,size);   	
}

CMemoryControl::CMemoryControl(const CMemoryControl &cMemoryControl)
{
 if ((&cMemoryControl)==this) return;
 PhisicalAddr=0;
 Size=0;
 VirtualAddr=MAP_FAILED;
 SetAddr(cMemoryControl.PhisicalAddr,cMemoryControl.Size);
}   
//----------------------------------------------------------------------------------------------------
//деструктор
//----------------------------------------------------------------------------------------------------
CMemoryControl::~CMemoryControl()
{
 Release();	
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//закрытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//открытые функции класса
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------------
//оператор =
//----------------------------------------------------------------------------------------------------
CMemoryControl& CMemoryControl::operator=(const CMemoryControl &cMemoryControl)
{
 if (this!=(&cMemoryControl)) SetAddr(cMemoryControl.PhisicalAddr,cMemoryControl.Size);   	
 return(*this);
}  
//----------------------------------------------------------------------------------------------------
//освободить ресурсы
//----------------------------------------------------------------------------------------------------
void CMemoryControl::Release(void)
{
 if (const_cast<void*>(VirtualAddr)!=MAP_FAILED) munmap_device_memory(const_cast<void*>(VirtualAddr),Size);  
 VirtualAddr=MAP_FAILED;    
}
//----------------------------------------------------------------------------------------------------
//задать адрес
//----------------------------------------------------------------------------------------------------
bool CMemoryControl::SetAddr(uint64_t phisical_addr,uint64_t size)
{
 Release();   	
 PhisicalAddr=phisical_addr;
 Size=size;
 VirtualAddr=mmap_device_memory(0,Size,PROT_READ|PROT_WRITE|PROT_NOCACHE,0,PhisicalAddr);
 if (const_cast<void*>(VirtualAddr)==MAP_FAILED) return(false);
 return(true);
}
//----------------------------------------------------------------------------------------------------
//получить, есть ли данный адрес
//----------------------------------------------------------------------------------------------------
bool CMemoryControl::IsAddr(uint64_t offset)
{
 if (const_cast<void*>(VirtualAddr)==MAP_FAILED) return(false);
 if (offset>=Size) return(false);
 return(true);	
}

//----------------------------------------------------------------------------------------------------
//получить адрес
//----------------------------------------------------------------------------------------------------
volatile void* CMemoryControl::GetAddr(void)
{
 return(VirtualAddr);
}
//----------------------------------------------------------------------------------------------------
//получить размер области памяти
//----------------------------------------------------------------------------------------------------
uint64_t CMemoryControl::GetSize(void)
{
 return(Size); 	
}   
//----------------------------------------------------------------------------------------------------
//записать в память
//----------------------------------------------------------------------------------------------------
bool CMemoryControl::Write(uint64_t offset,uint64_t size,volatile uint8_t *data)
{
 if (IsAddr(offset)==false) return(false);	
 if (IsAddr(offset+size-1+offset)==false) return(false);
 volatile uint8_t* ptr=reinterpret_cast<volatile uint8_t*>(VirtualAddr);
 for(uint64_t n=0;n<size;n++,ptr++,data++)
 {
  asm volatile ("": : :"memory");
  *ptr=*data;
 }
 return(true);	
}
//----------------------------------------------------------------------------------------------------
//считать из памяти
//----------------------------------------------------------------------------------------------------
bool CMemoryControl::Read(uint64_t offset,uint64_t size,volatile uint8_t *data)
{
 if (IsAddr(offset)==false) return(false);	
 if (IsAddr(offset+size-1+offset)==false) return(false);	
 volatile uint8_t* ptr=reinterpret_cast<volatile uint8_t*>(VirtualAddr);
 for(uint64_t n=0;n<size;n++,ptr++,data++) 
 {
  asm volatile ("": : :"memory");  
  *data=*ptr;
 }
 return(true);
}