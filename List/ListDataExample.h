#pragma once

//Copyright (C) 2020 Andrey Khishchenko

//This file is part of ListMT library.
//
//ListMT is free library software : you can redistribute it and /or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//any later version.
//
//ListMT library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public License
//along with ListMT library. If not, see < https://www.gnu.org/licenses/>.

//Test list elements with data and list supporting data classes header.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "List.h"				//линейный список
#include "ListE.h"				//линейный список (с исключениями)

//используем пространство имён многопоточного списка (для тестового примера можно использовать всё пространство using-директивой)
using namespace ListMT;

//ОБЪЯВЛЕНИЯ КЛАССОВ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class List, bool bExceptions> class ListData;					//список с данными

//ОБЪЯВЛЕНИЯ ТИПОВ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//выбираем стратегию работы с памятью
//стратегия работы с памятью через внутренние указатели
//template<class Type> using MemoryPolicy = InternalPointer<Type>;
//частичная специализация, но с явным указанием распределителей и удалителей памяти (для примера совпадают с дефолтными)
//template<class Type> using MemoryPolicy = InternalPointer<Type, DefaultDeleter_InternalPointer<Type>, DefaultAllocator_InternalPointer<Type>>;

template<class Type> using MemoryPolicy = SmartSharedPointer<Type>;		//выбираем стратегию работы с памятью

//СТРУКТУРЫ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//узлы списка

//однократное наследование (больше нельзя наследовать от этой структуры, иначе в ListElement_OneLinked/ListElement_TwoLinked будет использоваться
//неверный аргумент шаблона!!!)

class ListElementData_OneLinked : public ListElement_OneLinked<ListElementData_OneLinked, MemoryPolicy>		//узел односвязного списка
{
	unsigned __int64 u64Value = 0;						//некое значение узла
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_OneLinked(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

class ListElementData_TwoLinked : public ListElement_TwoLinked<ListElementData_TwoLinked, MemoryPolicy>		//узел двусвязного списка
{
	unsigned __int64 u64Value = 0;						//некое значение узла
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_TwoLinked(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

class ListElementData_OneLinked_CP : public ListElement_OneLinked_CP<ListElementData_OneLinked_CP, MemoryPolicy>		//узел односвязного списка
{
	unsigned __int64 u64Value = 0;						//некое значение узла
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_OneLinked_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

class ListElementData_TwoLinked_CP : public ListElement_TwoLinked_CP<ListElementData_TwoLinked_CP, MemoryPolicy>		//узел двусвязного списка
{	
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	unsigned __int64 u64Value = 0;						//некое значение узла

	ListElementData_TwoLinked_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

//многократное наследование (допускает передачу последней структуры или класса в самый базовый)

class thisclass {};			//класс-пустышка, используемый для аргумента по умолчанию в классе, производном от ListElementData_OneLinked/ListElementData_TwoLinked

//элементы односвязного списка

template<class DerivedListElement = thisclass> class ListElementData_OneLinked1 : public ListElement_OneLinked<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked1<>, DerivedListElement>, MemoryPolicy>
{
	unsigned __int64 u64Value = 0;						//некое значение узла

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_OneLinked1(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked2 : public ListElementData_OneLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_OneLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_OneLinked2(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked1_CP : public ListElement_OneLinked_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked1_CP<>, DerivedListElement>, MemoryPolicy>
{
	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	unsigned __int64 u64Value = 0;						//некое значение узла

	ListElementData_OneLinked1_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked2_CP : public ListElementData_OneLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2_CP<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_OneLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2_CP<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_OneLinked2_CP(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//конструктор
};

//элементы двусвязного списка

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked1 : public ListElement_TwoLinked<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked1<>, DerivedListElement>, MemoryPolicy>
{
	unsigned __int64 u64Value = 0;						//некое значение узла

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_TwoLinked1(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked2 : public ListElementData_TwoLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_TwoLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_TwoLinked2(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked1_CP : public ListElement_TwoLinked_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked1_CP<>, DerivedListElement>, MemoryPolicy>
{
	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	unsigned __int64 u64Value = 0;						//некое значение узла

	ListElementData_TwoLinked1_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//конструктор
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked2_CP : public ListElementData_TwoLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2_CP<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_TwoLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2_CP<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//имитируем некоторые данные для заполнения памяти

	template<class List, bool bExceptions> friend class ListData;			//объявляем шаблонный класс списка с данными привилегированным для обеспечения доступа из него

public:

	ListElementData_TwoLinked2_CP(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//конструктор
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class _List> class ListData<_List, false> : public _List		//список с данными (без исключений)
{
public:

	//объявления типов
	using List = _List;
	using ListElement = typename List::ListElement;				//излекаем тип элемента из переданного списку параметра
	using ptrListElement = typename List::ptrListElement;		//извлекаем тип указателя из переданного списку параметра

	//конструкторы

	ListData() {}		//конструктор без аргументов (нужен при определении конструкторов создания из объектов базового класса ниже)

	//конструкторы для создания из объектов базового класса
	ListData(List& list) : List(list) {}
	ListData(List&& list) : List(std::move(list)) {}		//noexcept почему-то не допускается при указании конструктора базового класса

	//функции

	ListErrorCodes::eListErrorCode Add(const unsigned __int64 u64Value, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept					//добавляет новый элемент в конец списка, используя заданное значение для инициализации
	{
		//на вход: u64Value - значение, которое надо записать в конец списка; pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что
		//операция над элементом списка защищена, bUsePreviousElementIndex - использовать индекс предыдущего элемента для ускорения операции добавления в
		//некоторых стратегиях поиска элемента извне функции

		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ptrListElement pCurr = nullptr;
		pCurr = List::AddLast(&eError, true, bUsePreviousElementIndex, u64Value);			//добавляем новый элемент в конец списка
		if (pCurr == nullptr && eError == ListErrorCodes::eListErrorCode::FailElemCreation)
		{
			if (!bProtected)
				List::UnlockListExclusive();						//разблокируем список
			return eError;
		}

		if (!bProtected)
			List::UnlockListExclusive();						//разблокируем список

		return eError;
	}

	ListErrorCodes::eListErrorCode Add(ptrListElement pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept
	{
		//добавляет новый элемент списка, используя заданное значение для инициализации
		//на вход: pElem - указатель на элемент списка, после которого необходимо вставить новый узел, u64Value - значение, которое надо записать
		//в конец списка; bProtected - флаг того, что операция над элементом списка защищена извне функции

		if (!pElem)					//передан нулевой указатель
			return ListErrorCodes::eListErrorCode::Nullptr;

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//флаг повторной попытки создания элемента
		while (true)						//повторяем попытку, если не удалось создать элемент
		{
			pCurr = List::AddAfter(pElem, &eError, true, false, u64Value);			//добавляем новый элемент списка
			if (!pCurr)
			{
				if(eError == ListErrorCodes::eListErrorCode::NotPartOfList)	//переданный элемент уже не является частью списка - повторяем попытку
					bTryAgain = true;
				else if (eError != ListErrorCodes::eListErrorCode::Success)	//произошла другая ошибка - не разбираясь в деталях, выходим
				{
					if (!bProtected)
						List::UnlockListExclusive();				//разблокируем список
					return eError;
				}
			}			

			if (bTryAgain)
			{
				pElem = List::GetFirst(true);		//принудительно сбрасываем переданный элемент до первого, если переданный уже не является частью списка
				bTryAgain = false;
				continue;					//повторяем попытку для первого элемента
			}
			else
				break;									//выходим из цикла, если элемент создан удачно
		}
		//pCurr->u64Value = u64Value;						//сохраняем переданное значение в узле списка

		if (!bProtected)
			List::UnlockListExclusive();				//разблокируем список

		return eError;
	}

	ptrListElement FindElementByValue(const unsigned __int64 u64Value) const noexcept
	{
		//ищет в списке первый элемент, значение которого совпадает с заданным
		//на вход: u64Value - значение искомого элемента
		//на выход: указатель на элемент либо nullptr, если элемент не найден (либо список пуст)

		List::LockListShared();					//блокируем список на чтение

		ptrListElement pCurr = List::GetFirst(true);
		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;	//код ошибки
		while (pCurr)
		{
			if (pCurr->u64Value == u64Value)
				break;

			pCurr = List::GetNext(pCurr, &eError, true, true);
		}

		List::UnlockListShared();					//разблокируем список на чтение

		return pCurr;
	}

	unsigned __int64 ComputeAverageValue(void) const noexcept
	{
		//вычисляет среднее значение элементов всего списка

		List::LockListShared();					//блокируем список на чтение

		ptrListElement pCurr = List::GetFirst(true);
		unsigned __int64 u64AverageValue = 0;		//вычисляемое среднее значение
		unsigned int uiNumElements = 0;				//количество узлов
		while (pCurr)
		{
			u64AverageValue += pCurr->u64Value;
			uiNumElements++;
			pCurr = GetNext(pCurr, nullptr, true, true);
		}
		u64AverageValue /= uiNumElements;

		List::UnlockListShared();					//разблокируем список на чтение
		return u64AverageValue;
	}

	unsigned __int64 GetValue(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//возвращает значение переданного по указателю элемента
		//на вход: pElem - указатель на элемент списка, значение которого надо получить; pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг
		//того, что операция над элементом списка защищена извне функции
		//на выход: значение элемента

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return 0;
		}

		if (!bProtected)
			List::LockListShared();					//блокируем список на чтение
		//проверяем, является ли переданный элемент частью списка
		if (!List::FindElement(pElem, pErrorCode, true))
		{			
			if (!bProtected)
				List::UnlockListShared();
			return 0;
		}
		unsigned __int64 u64Value = pElem->u64Value;
		if (!bProtected)
			List::UnlockListShared();					//разблокируем список на чтение

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return u64Value;
	}

	ListErrorCodes::eListErrorCode SetValue(ptrListElement const pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept
	{
		//возвращает значение переданного по указателю элемента
		//на вход: pElem - указатель на элемент списка, значение которого надо установить; u64Value - устанавливаемое значение; bProtected - флаг того, что
		//операция над элементом списка, защищена извне функции
		//на выход: значение элемента

		if (!pElem)
			return ListErrorCodes::eListErrorCode::Nullptr;

		if (!bProtected)
			List::LockListExclusive();					//блокируем список на запись
		//проверяем, является ли переданный элемент частью списка
		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
		if (!List::FindElement(pElem, &eError, true))
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return eError;
		}
		pElem->u64Value = u64Value;
		if (!bProtected)
			List::UnlockListExclusive();					//разблокируем список на запись

		return eError;
	}

	//операции

	ListData& operator=(List& list) noexcept
	{
		//операция копирующего присваивания
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		List::MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	ListData& operator=(List&& list) noexcept
	{
		//операция перемещающего присваивания
		//на вход: list - rvalue-ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		//используем операцию базового класса
		//применяем std::move, чтобы принудительно привести list к rvalue (хотя она уже стала lvalue, будучи параметром функции), и вызвать
		//правильную версию операции базового класса
		List::operator=(std::move(list));
		return *this;
	}

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class _List> class ListData<_List, true> : public _List		//список с данными (с исключениями)
{
public:

	//объявления типов
	using List = _List;
	using ListElement = typename List::ListElement;				//излекаем тип элемента из переданного списку параметра
	using ptrListElement = typename List::ptrListElement;		//извлекаем тип указателя из переданного списку параметра

	//конструкторы

	ListData() {}		//конструктор без аргументов (нужен при определении конструкторов создания из объектов базового класса ниже)

	//конструкторы для создания из объектов базового класса
	ListData(List& list) : List(list) {}
	ListData(List&& list) : List(std::move(list)) {}		//noexcept почему-то не допускается при указании конструктора базового класса

	//функции

	bool Add(const unsigned __int64 u64Value, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept					//добавляет новый элемент в конец списка, используя заданное значение для инициализации
	{
		//на вход: u64Value - значение, которое надо записать в конец списка; bProtected - флаг того, что операция над элементом списка защищена,
		//bUsePreviousElementIndex - использовать индекс предыдущего элемента для ускорения операции добавления в некоторых стратегиях поиска элемента
		//извне функции критической секций

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ptrListElement pCurr = nullptr;
		try
		{
			pCurr = List::AddLast(true, bUsePreviousElementIndex, u64Value);			//добавляем новый элемент в конец списка
		}
		catch (typename ListErrors::FailElemCreation)					//не удалось выделить память для узла
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return false;
		}
		//pCurr->u64Value = u64Value;					//сохраняем переданное значение в узле списка

		if (!bProtected)
			List::UnlockListExclusive();						//разблокируем список

		return true;
	}

	bool Add(ptrListElement pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept		//добавляет новый элемент списка, используя заданное значение для инициализации
	{
		//на вход: pElem - указатель на элемент списка, после которого необходимо вставить новый узел, u64Value - значение, которое надо записать
		//в конец списка; bProtected - флаг того, что операция над элементом списка защищена извне функции критической секций

		if (!pElem)					//передан нулевой указатель
			return false;

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//флаг повторной попытки создания элемента
		while (true)						//повторяем попытку, если не удалось создать элемент
		{
			try
			{
				pCurr = List::AddAfter(pElem, true, false, u64Value);			//добавляем новый элемент списка
			}
			catch (typename ListErrors::NotPartOfList)						//переданный элемент уже не является частью списка - повторяем попытку
			{
				bTryAgain = true;
			}
			catch (typename ListErrors::ListError)						//произошла другая ошибка - не разбираясь в деталях, выходим
			{
				if (!bProtected)
					List::UnlockListExclusive();				//разблокируем список
				return false;
			}

			if (bTryAgain)
			{
				pElem = List::GetFirst(true);		//принудительно сбрасываем переданный элемент до первого, если переданный уже не является частью списка
				bTryAgain = false;
				continue;					//повторяем попытку для первого элемента
			}
			else
				break;									//выходим из цикла, если элемент создан удачно
		}
		//pCurr->u64Value = u64Value;						//сохраняем переданное значение в узле списка

		if (!bProtected)
			List::UnlockListExclusive();				//разблокируем список

		return true;
	}

	ptrListElement FindElementByValue(const unsigned __int64 u64Value) const noexcept
	{
		//ищет в списке первый элемент, значение которого совпадает с заданным
		//на вход: u64Value - значение искомого элемента
		//на выход: указатель на элемент либо nullptr, если элемент не найден (либо список пуст)

		List::LockListShared();					//блокируем список на чтение

		ptrListElement pCurr = List::GetFirst(true);
		while (pCurr)
		{
			if (pCurr->u64Value == u64Value)
				break;

			pCurr = List::GetNext(pCurr, true, true);
		}

		List::UnlockListShared();					//разблокируем список на чтение

		return pCurr;
	}

	unsigned __int64 ComputeAverageValue(void) const noexcept
	{
		//вычисляет среднее значение элементов всего списка

		List::LockListShared();					//блокируем список на чтение

		ptrListElement pCurr = List::GetFirst(true);
		unsigned __int64 u64AverageValue = 0;		//вычисляемое среднее значение
		unsigned int uiNumElements = 0;				//количество узлов
		while (pCurr)
		{
			u64AverageValue += pCurr->u64Value;
			uiNumElements++;
			pCurr = GetNext(pCurr, true, true);
		}
		u64AverageValue /= uiNumElements;

		List::UnlockListShared();					//разблокируем список на чтение
		return u64AverageValue;
	}

	unsigned __int64 GetValue(ptrListElement const pElem, bool bProtected = false) const
	{
		//возвращает значение переданного по указателю элемента
		//на вход: pElem - указатель на элемент списка, значение которого надо получить; bProtected - флаг того, что операция над элементом списка
		//защищена извне функции критической секций
		//на выход: значение элемента

		if (!pElem)
			throw ListErrors::Nullptr();	//генерация исключения об ошибочном параметре (нулевой указатель)

		if (!bProtected)
			List::LockListShared();					//блокируем список на чтение
		//проверяем, является ли переданный элемент частью списка
		if (!List::FindElement(pElem, true))
		{
			if (!bProtected)
				List::UnlockListShared();
			throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
		}
		unsigned __int64 u64Value = pElem->u64Value;
		if (!bProtected)
			List::UnlockListShared();					//разблокируем список на чтение

		return u64Value;
	}

	void SetValue(ptrListElement const pElem, const unsigned __int64 u64Value, bool bProtected = false)
	{
		//возвращает значение переданного по указателю элемента
		//на вход: pElem - указатель на элемент списка, значение которого надо установить; u64Value - устанавливаемое значение; bProtected - 
		//флаг того, что операция над элементом списка, защищена извне функции критической секций
		//на выход: значение элемента

		if (!pElem)
			throw ListErrors::Nullptr();	//генерация исключения об ошибочном параметре (нулевой указатель)

		if (!bProtected)
			List::LockListExclusive();					//блокируем список на запись
		//проверяем, является ли переданный элемент частью списка
		if (!List::FindElement(pElem, true))
		{
			if (!bProtected)
				List::UnlockListExclusive();
			throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
		}
		pElem->u64Value = u64Value;
		if (!bProtected)
			List::UnlockListExclusive();					//разблокируем список на запись
	}

	//операции

	ListData& operator=(List& list)
	{
		//операция копирующего присваивания
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		List::MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	ListData& operator=(List&& list) noexcept
	{
		//операция перемещающего присваивания
		//на вход: list - rvalue-ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		//используем операцию базового класса
		//применяем std::move, чтобы принудительно привести list к rvalue (хотя она уже стала lvalue, будучи параметром функции), и вызвать
		//правильную версию операции базового класса
		List::operator=(std::move(list));
		return *this;
	}

};