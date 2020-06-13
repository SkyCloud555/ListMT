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

//Program module to test ListMT operations.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>						//основной включаемый файл Windows
#include <vector>
#include <utility>							//для swap
#include <tchar.h>							//поддержка макросов Unicode
#include <stdlib.h>							//для случайных чисел - функция rand()
#include "..\List\ListDataExample.h"		//класс списка с данными
#include "..\List\SearchContainerElement.h"	//стратегии поиска элемента
#include "ListList.h"						//класс списка, хранящего внутри себя списки с данными

//ОБЪЯВЛЕНИЯ ТИПОВ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//объекты классов списков
//вводим вспомогательные типы для удобства
using ListElementData_1L = ListElementData_OneLinked_CP;				//тип узла односвязного списка
//using ListElementData_1L = ListElementData_OneLinked2_CP<>;			//тип узла односвязного списка (многократное наследование классов элементов)
using ListElementData_2L = ListElementData_TwoLinked_CP;				//тип узла двусвязного списка
//using ListElementData_2L = ListElementData_TwoLinked2_CP<>;			//тип узла двусвязного списка (многократное наследование классов элементов)

using ListElement = ListElementData_2L;
//using ListBase = List_OneLinked<ListElement, ThreadLockingWin_SRWLock>;
//using ListElement = ListElementData_2L;
//using ListBase = List_TwoLinked<ListElement>;
using List = ListData<List_TwoLinked<ListElement, ThreadLockingWin_SRWLock, SearchByIndex_BitArray, true>, true>;

using List1 = ListData<List_TwoLinked<ListElement, ThreadLockingWin_SRWLock, SearchByIndex_BitArray, false>, false>;

//ГЛОБАЛЬНЫЕ КОНСТАНТЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned int c_uiFirstListNumNumbers = 3;					//длина первого списка
const unsigned int c_uiSecondListNumNumbers = 2;				//длина второго списка
const unsigned int c_uiThirdListNumNumbers = 3;					//длина третьего списка
const unsigned int c_uiForthListNumNumbers = 4;					//длина четвёртого списка

//ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

List list1, list2, list3;

//ListData<List_OneLinked<ListElementData_1L>, ListElementData_1L> list1, list2, list3;		//односвязный список
//ListData<List_TwoLinked<ListElementData_2L>, ListElementData_2L> listTwoLinked;		//двусвязный список


//ФУНКЦИИ//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//в параметрах определения функции: либо List list, либо ListBase& list, либо List&& list (но тогда надо при вызове с обычной переменной-списком,
//- PrintList(list), а не выражением, использовать std::move)
//при этом что-то из них не работает при одновременном использовании вызовов (кроме варианта ListBase& list):
//PrintList(list);
//PrintList(list1 + list2);
//Правда, введение доступа к значению узла только через функцию списка закрыло вариант ListBase& list.

void PrintList(List&& list)
{
	//выводит содержимое списка на экран (тест передачи в функцию списков и выражений с ними)
	_tprintf_s(TEXT("\n\nList printing function:\n"));
	List::ptrListElement listElement = list.GetFirst();
	while (listElement)
	{
		_tprintf_s(TEXT("%I64u\n"), list.GetValue(listElement));
		listElement = list.GetNext(listElement);
	}
}

void main(void)
{
	//заполняем список для результата
	List list;
	_tprintf_s(TEXT("Result list (initialization):\n"));
	for (unsigned int i = 0; i < c_uiFirstListNumNumbers; i++)
	{
		list.Add(0);
		_tprintf_s(TEXT("%u\n"), 0);
	}

	//заполняем первый список
	_tprintf_s(TEXT("\nFirst list (List1):\n"));
	for (unsigned int i = 0; i < c_uiFirstListNumNumbers; i++)
	{
		list1.Add(i + 1);
		_tprintf_s(TEXT("%u\n"), i + 1);
	}
	//заполняем второй список
	_tprintf_s(TEXT("\nSecond list (List2):\n"));
	for (unsigned int i = 0; i < c_uiSecondListNumNumbers; i++)
	{
		list2.Add(i + c_uiFirstListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + 1);
	}
	//заполняем третий список
	_tprintf_s(TEXT("\nThird list (List3):\n"));
	for (unsigned int i = 0; i < c_uiThirdListNumNumbers; i++)
	{
		list3.Add(i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
	}

	List1 list4;
	//заполняем четвёртый список другого типа
	_tprintf_s(TEXT("\nForth list (List4):\n"));
	for (unsigned int i = 0; i < c_uiForthListNumNumbers; i++)
	{
		list4.Add(i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + c_uiThirdListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
	}

	_tprintf_s(TEXT("\nList1 and list2 swap:\n"));
	std::swap(list1, list2);			//меняем местами списки
	_tprintf_s(TEXT("List1:\n"));
	List::ptrListElement listElement = list1.GetFirst();
	while (listElement)
	{
		_tprintf_s(TEXT("%I64u\n"), list1.GetValue(listElement));
		listElement = list1.GetNext(listElement);
	}
	_tprintf_s(TEXT("List2:\n"));
	listElement = list2.GetFirst();
	while (listElement)
	{
		_tprintf_s(TEXT("%I64u\n"), list2.GetValue(listElement));
		listElement = list2.GetNext(listElement);
	}

	

	//list = list1;
	//list = list1 + list2;
	//list += list1 + list2;
	//list = list1 + list2 + list3;
	list += list1 + list2 + list3 + list4;
	_tprintf_s(TEXT("\nFinal list:\n"));
	listElement = list.GetFirst();
	while (listElement)
	{
		_tprintf_s(TEXT("%I64u\n"), list.GetValue(listElement));
		listElement = list.GetNext(listElement);
	}

	//передача в функцию
	//PrintList(list);
	PrintList(std::move(list));
	PrintList(list1 + list2);

	//разбиваем список после заданного элемента
	_tprintf_s(TEXT("\n\nList splitting:\n"));
	listElement = list.FindElementByValue(5);
	if (listElement)
	{
		List lstSplitPart = list.Split(listElement);

		//List lstSplitPart;
		//lstSplitPart = list.Split(listElement);

		_tprintf_s(TEXT("First part:\n"));
		listElement = list.GetFirst();
		while (listElement)
		{
			_tprintf_s(TEXT("%I64u\n"), list.GetValue(listElement));
			listElement = list.GetNext(listElement);
		}

		_tprintf_s(TEXT("Second part:\n"));
		listElement = lstSplitPart.GetFirst();
		while (listElement)
		{
			_tprintf_s(TEXT("%I64u\n"), lstSplitPart.GetValue(listElement));
			listElement = lstSplitPart.GetNext(listElement);
		}
	}

	const List c_list = list;	//создаём константный список
	//выводим его
	_tprintf_s(TEXT("\n\nConst list:\n"));
	//listElement = c_list.GetFirst();
	//while (listElement)
	//{
	//	_tprintf_s(TEXT("%I64u\n"), c_list.GetValue(listElement));
	//	listElement = c_list.GetNext(listElement);
	//}
	for(auto& leValue : list)
		_tprintf_s(TEXT("%I64u\n"), leValue->u64Value);
	//c_list.Add(12345, false);		//ошибка - добавление элемента к константному списку

	_tprintf_s(TEXT("\n\nStd vector test:\n"));
	std::vector<int> v = { 0, 1, 2, 3, 4, 5 };
	const auto cv = v;
	for (auto&& i : v)
		i = 1;
	for (auto&& i : v)
		_tprintf_s(TEXT("%d "), i);
	_tprintf_s(TEXT("\n"));
	for (auto&& i : cv) // access by f-d reference, the type of i is const int&
		_tprintf_s(TEXT("%d "), i);
	_tprintf_s(TEXT("\n"));

	//массив списков
	const unsigned int c_uiListArraySize = 10;			//число элементов массива
	List lstArray[c_uiListArraySize];
	for (List& lst : lstArray)							//C++11: for-цикл по коллекции
		lst = list;

	//массив константных списков
	const List c_lstArray[c_uiListArraySize] = {list1, list2, list3};	//первые три инициализируются явно, остальные, видимо, конструктором по умолчанию (пустые списки)

	//СПИСОК СПИСКОВ
	//сделал по-простому и по-быстрому, поскольку доступ к внутреннему списку получается в обход защиты внешнего списка
	//в действительности нужно обеспечить блокировку внешнего списка на время работы с каким-либо из внутренних
	//но я этого делать не стал, т.к. вопрос был именно в проверке возможности создания списка внутри списка, а не в поддержке многопоточности
	_tprintf_s(TEXT("\n"));
	ListList<List_TwoLinked<ListElementList_TwoLinked, ThreadLockingWin_SRWLock, DirectSearch, true>> listlist;
	const unsigned char c_ucNumLists = 3;
	unsigned char ucTotalIndex = 0;			//значение элемента списка с данными
	srand(GetTickCount());
	for (unsigned char i = 0; i < c_ucNumLists; i++)
	{
		_tprintf_s(TEXT("List %u: "), i);
		auto ptrListInternal = listlist.Add();
		//на самом деле для данного примера можно внутренний список сделать открытым членом и обращаться к нему напрямую, минуя функцию: всё равно
		//здесь она возвращает его по ссылке
		auto& listInternal = ptrListInternal->GetList();
		unsigned char ucListLength = 3 + rand() % 15;
		_tprintf_s(TEXT("%u elements\n"), ucListLength);
		for (unsigned char j = 0; j < ucListLength; j++)
			listInternal.Add(ucTotalIndex++);
	}

	//выводим списки
	_tprintf_s(TEXT("\n"));
	auto ptrList = listlist.GetFirst();		//получаем указатель на первый элемент списка
	unsigned char ucListIndex = 0;			//номер списка
	while(ptrList != nullptr)
	{
		_tprintf_s(TEXT("List %u: "), ucListIndex++);
		auto& listInternal = ptrList->GetList();			//получаем доступ к внутреннему списку
		auto ptrElementData = listInternal.GetFirst();		//получаем первый элемент списка
		while (ptrElementData != nullptr)
		{
			_tprintf_s(TEXT("%u "), listInternal.GetValue(ptrElementData));
			ptrElementData = listInternal.GetNext(ptrElementData);		//переходим к следующему элементу
		}
		_tprintf_s(TEXT("\n"));
		ptrList = listlist.GetNext(ptrList);		//переходим к следующему списку
	}
}