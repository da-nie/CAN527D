#ifndef C_UNIQUE_ARRAY_PTR_H
#define C_UNIQUE_ARRAY_PTR_H

//****************************************************************************************************
//Класс умного указателя для массивов объектов
//****************************************************************************************************

//****************************************************************************************************
//подключаемые библиотеки
//****************************************************************************************************
#include <stdio.h>

//****************************************************************************************************
//Класс умного указателя для массивов объектов
//****************************************************************************************************

template<class C>
class CUniqueArrayPtr
{
 private:
  C *c_Ptr;//указатель на массив объектов
 public:
  CUniqueArrayPtr()//конструктор
  {
   c_Ptr=NULL;  	
  }
 ~CUniqueArrayPtr()//деструктор
  {
   if (c_Ptr!=NULL) delete[](c_Ptr); 	
   c_Ptr=NULL;
  }
 public: 
  C* Get(void)//получить указатель на объекты
  {
   return(c_Ptr);
  }
  void Set(C *c_Ptr_Set)//задать указатель на объекты
  {
   if (c_Ptr!=NULL) delete[](c_Ptr);  	
   c_Ptr=c_Ptr_Set;
  }
  void Release(void)//освободить объекты
  {
   if (c_Ptr!=NULL) delete[](c_Ptr);  	
   c_Ptr=NULL;
  }
 //запрещаем операции копирования и присваивания 
 private:
  CUniqueArrayPtr(const CUniquePtr<C> &cUniqueArrayPtr){};//конструктор копирования запрещён
  CUniqueArrayPtr<C>& operator=(const CUniqueArrayPtr<C>& cUniqueArrayPtr){return(*this);};//оператор "="
};
#endif
