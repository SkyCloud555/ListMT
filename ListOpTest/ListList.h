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

//Test class header for the definition of a list inside list.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\List\ListDataExample.h"		//класс списка с данными
#include "..\List\SearchContainerElement.h"	//стратегии поиска элемента

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//для проверки списка списков

//однократное наследование (больше нельзя наследовать от этой структуры, иначе в ListElement_OneLinked/ListElement_TwoLinked будет использоваться
//неверный аргумент шаблона!!!)

class ListElementList_TwoLinked : public ListElement_TwoLinked_CP<ListElementList_TwoLinked, MemoryPolicy>		//узел двусвязного списка
{
public:

	using ListInternal = ListData<List_OneLinked<ListElementData_OneLinked2<>, ThreadLockingWin_SRWLock, DirectSearch, true>, true>;		//тип внутреннего списка

private:

	ListInternal list;			//односвязный список с данными
		
public:

	ListElementList_TwoLinked() = default;	//указываем, что нас устраивает конструктор по умолчанию
	ListElementList_TwoLinked(ListInternal& list) : list(list) {}		//конструктор
	
	ListInternal& GetList(void)
	{
		return list;
	}
	template<class List> friend class ListList;			//объявляем шаблонный класс списка списков привилегированным для обеспечения доступа из него
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class List> class ListList : public List		//список с данными
{
public:

	//объявления типов
	using ListElement = typename List::ListElement;				//излекаем тип элемента из переданного списку параметра
	using ptrListElement = typename List::ptrListElement;		//извлекаем тип указателя из переданного списку параметра

	//конструкторы

	ListList() {}		//конструктор без аргументов (нужен при определении конструкторов создания из объектов базового класса ниже)

	//конструкторы для создания из объектов базового класса
	ListList(List& list) : List(list) {}
	ListList(List&& list) : List(std::move(list)) {}		//noexcept почему-то не допускается при указании конструктора базового класса

	//функции

	ptrListElement Add(bool bProtected = false) noexcept					//добавляет новый пустой список с данными в конец списка
	{
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции критической секций

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ptrListElement pCurr = nullptr;
		try
		{
			pCurr = List::AddLast(true);			//добавляем новый элемент в конец списка
		}
		catch (typename ListErrors::FailElemCreation)					//не удалось выделить память для узла
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return nullptr;
		}

		if (!bProtected)
			List::UnlockListExclusive();						//разблокируем список

		return pCurr;
	}

	ptrListElement Add(ptrListElement pElem, bool bProtected = false) noexcept		//добавляет новый элемент списка, используя заданное значение для инициализации
	{
		//на вход: pElem - указатель на элемент списка, после которого необходимо вставить новый узел
		//в конец списка; bProtected - флаг того, что операция над элементом списка защищена извне функции критической секций

		if (!pElem)					//передан нулевой указатель
			return nullptr;

		if (!bProtected)
			List::LockListExclusive();				//блокируем список на время операции

		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//флаг повторной попытки создания элемента
		while (true)						//повторяем попытку, если не удалось создать элемент
		{
			try
			{
				pCurr = List::AddAfter(pElem, true, u64Value);			//добавляем новый элемент списка
			}
			catch (typename ListErrors::NotPartOfList)						//переданный элемент уже не является частью списка - повторяем попытку
			{
				bTryAgain = true;
			}
			catch (typename ListErrors::ListError)						//произошла другая ошибка - не разбираясь в деталях, выходим
			{
				if (!bProtected)
					List::UnlockListExclusive();				//разблокируем список
				return nullptr;
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

		if (!bProtected)
			List::UnlockListExclusive();				//разблокируем список

		return pCurr;
	}

	//операции

	ListList& operator=(List& list)
	{
		//операция копирующего присваивания
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		List::MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	ListList& operator=(List&& list) noexcept
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
