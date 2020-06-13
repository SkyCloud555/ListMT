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

//List element classes header.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "ListErrors.h"					//исключения и коды ошибок при работе списка
#include "SearchContainerElement.h"		//стратегии поиска элемента (они в этом заголовочном файле не нужны, но будут автоматически включаться для всех пользователей списка)

namespace ListMT						//пространство имён многопоточного списка (односвязного и двусвязного)
{

//ОБЪЯВЛЕНИЯ КЛАССОВ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement> class ListBase;				//базовый класс списка

template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> class List_OneLinked;					//односвязный список
template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> class List_TwoLinked;					//двусвязный список


//ПРОТОТИПЫ ФУНКЦИЙ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//операция объединения списков
template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
	template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
	template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//узлы списка

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_OneLinked			//узел односвязного списка (без данных)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

private:

	typename MemoryPolicy::ptrType pNext;		//указатель на следующий элемент списка

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем класс прямого поиска и проверки элемента списка привилегированным
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_TwoLinked			//узел двухсвязного списка (без данных)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

private:

	typename MemoryPolicy::ptrType		pNext;		//указатель на следующий элемент списка
	typename MemoryPolicy::weak_ptrType	pPrev;		//указатель на предыдущий элемент списка

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем класс прямого поиска и проверки элемента списка привилегированным
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//элементы списка с информацией для быстрой проверки присутствия элемента в списке

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_OneLinked_CP			//узел односвязного списка (без данных)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

private:

	typename MemoryPolicy::ptrType pNext;		//указатель на следующий элемент списка

	unsigned long long ullElementIndex;			//индекс элемента среди всех созданных в списке
	ListBase<ListElement> *pContainer;			//указатель на список-владелец элемента

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем классы стратегий поиска и проверки элемента списка привилегированными
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;	
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_TwoLinked_CP			//узел двухсвязного списка (без данных)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

private:

	typename MemoryPolicy::ptrType		pNext;		//указатель на следующий элемент списка
	typename MemoryPolicy::weak_ptrType	pPrev;		//указатель на предыдущий элемент списка

	unsigned long long ullElementIndex;				//индекс элемента среди всех созданных в списке
	ListBase<ListElement> *pContainer;				//указатель на список-владелец элемента

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем классы стратегий поиска и проверки элемента списка привилегированными
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_OneLinked
{
	ElementData ed;		//данные

public:

	using ListElement = ListElementCompound_OneLinked<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

	template<class... Args> ListElementCompound_OneLinked(Args... args) : ed(args...) {}
	ListElementCompound_OneLinked(const ElementData& ed) : ed(ed) {}

	ElementData& operator*()
	{
		return ed;
	}

	operator ElementData()
	{
		return ed;
	}

	//template<template<class, class> class ListElementCompound, template<class> class ListElement_MemoryPolicy> ListElement operator=(const ListElementCompound<ElementData, ListElement_MemoryPolicy>& le)
	//{
	//	ed = le.ed;
	//	return *this;
	//}

	//ListElement operator=(const ElementData& ed)
	//{
	//	ListElement::ed = ed;
	//}

private:

	typename MemoryPolicy::ptrType pNext = nullptr;		//указатель на следующий элемент списка

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем класс прямого поиска и проверки элемента списка привилегированным
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_OneLinked_CP
{
	ElementData ed;		//данные

public:

	using ListElement = ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

	template<class... Args> ListElementCompound_OneLinked_CP(Args... args) : ed(args...) {}
	ListElementCompound_OneLinked_CP(const ElementData& ed) : ed(ed) {}

	ElementData& operator*()
	{
		return ed;
	}

	operator ElementData()
	{
		return ed;
	}

	//template<template<class, class> class ListElementCompound, template<class> class ListElement_MemoryPolicy> ListElement operator=(const ListElementCompound<ElementData, ListElement_MemoryPolicy>& le)
	//{
	//	ed = le.ed;
	//	return *this;
	//}

	//ListElement operator=(const ElementData& ed)
	//{
	//	ListElement::ed = ed;
	//}

private:

	typename MemoryPolicy::ptrType pNext = nullptr;		//указатель на следующий элемент списка

	unsigned long long ullElementIndex = 0;			//индекс элемента среди всех созданных в списке
	ListBase<ListElement>* pContainer = nullptr;	//указатель на список-владелец элемента

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем классы стратегий поиска и проверки элемента списка привилегированными
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_TwoLinked
{
	ElementData ed;		//данные

public:

	using ListElement = ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

	template<class... Args> ListElementCompound_TwoLinked(Args... args) : ed(args...) {}
	ListElementCompound_TwoLinked(const ElementData& ed) : ed(ed) {}

	ElementData& operator*()
	{
		return ed;
	}

	operator ElementData()
	{
		return ed;
	}

	//template<template<class, class> class ListElementCompound, template<class> class ListElement_MemoryPolicy> ListElement operator=(const ListElementCompound<ElementData, ListElement_MemoryPolicy>& le)
	//{
	//	ed = le.ed;
	//	return *this;
	//}

	//ListElement operator=(const ElementData& ed)
	//{
	//	ListElement::ed = ed;
	//}

private:

	typename MemoryPolicy::ptrType		pNext = nullptr;		//указатель на следующий элемент списка
	typename MemoryPolicy::weak_ptrType	pPrev;		//указатель на предыдущий элемент списка

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем класс прямого поиска и проверки элемента списка привилегированным
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_TwoLinked_CP
{
	ElementData ed;		//данные

public:

	using ListElement = ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//сохраняем используемую стратегию работы с памятью внутри класса элемента

	template<class... Args> ListElementCompound_TwoLinked_CP(Args... args) : ed(args...) {}
	ListElementCompound_TwoLinked_CP(const ElementData& ed) : ed(ed) {}

	ElementData& operator*()
	{
		return ed;
	}

	operator ElementData()
	{
		return ed;
	}

	//template<template<class, class> class ListElementCompound, template<class> class ListElement_MemoryPolicy> ListElement operator=(const ListElementCompound<ElementData, ListElement_MemoryPolicy>& le)
	//{
	//	ed = le.ed;
	//	return *this;
	//}

	//ListElement operator=(const ElementData& ed)
	//{
	//	ListElement::ed = ed;
	//}

private:

	typename MemoryPolicy::ptrType		pNext = nullptr;		//указатель на следующий элемент списка
	typename MemoryPolicy::weak_ptrType	pPrev;		//указатель на предыдущий элемент списка

	unsigned long long ullElementIndex = 0;				//индекс элемента среди всех созданных в списке
	ListBase<ListElement>* pContainer = nullptr;		//указатель на список-владелец элемента

	//объявляем шаблонный класс односвязного списка привилегированным для обеспечения доступа из него; класс объявляется привилегированным ВЕСЬ, независимо от конкретных
	//значений параметров шаблона; указывать привилегированными частичные специализации недопустимо: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//объявляем шаблонную функцию операции "+" привилегированной
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//объявляем классы стратегий поиска и проверки элемента списка привилегированными
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

}		//конец определения пространства имён ListMT