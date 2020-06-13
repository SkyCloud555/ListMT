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

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "ListErrors.h"					//���������� � ���� ������ ��� ������ ������
#include "SearchContainerElement.h"		//��������� ������ �������� (��� � ���� ������������ ����� �� �����, �� ����� ������������� ���������� ��� ���� ������������� ������)

namespace ListMT						//������������ ��� �������������� ������ (������������ � �����������)
{

//���������� �������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement> class ListBase;				//������� ����� ������

template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> class List_OneLinked;					//����������� ������
template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> class List_TwoLinked;					//���������� ������


//��������� �������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//�������� ����������� �������
template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
	template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
	template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���� ������

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_OneLinked			//���� ������������ ������ (��� ������)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

private:

	typename MemoryPolicy::ptrType pNext;		//��������� �� ��������� ������� ������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ����� ������� ������ � �������� �������� ������ �����������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_TwoLinked			//���� ������������ ������ (��� ������)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

private:

	typename MemoryPolicy::ptrType		pNext;		//��������� �� ��������� ������� ������
	typename MemoryPolicy::weak_ptrType	pPrev;		//��������� �� ���������� ������� ������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ����� ������� ������ � �������� �������� ������ �����������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//�������� ������ � ����������� ��� ������� �������� ����������� �������� � ������

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_OneLinked_CP			//���� ������������ ������ (��� ������)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

private:

	typename MemoryPolicy::ptrType pNext;		//��������� �� ��������� ������� ������

	unsigned long long ullElementIndex;			//������ �������� ����� ���� ��������� � ������
	ListBase<ListElement> *pContainer;			//��������� �� ������-�������� ��������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ������ ��������� ������ � �������� �������� ������ ������������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;	
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, template<class> class _MemoryPolicy> class ListElement_TwoLinked_CP			//���� ������������ ������ (��� ������)
{
public:

	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

private:

	typename MemoryPolicy::ptrType		pNext;		//��������� �� ��������� ������� ������
	typename MemoryPolicy::weak_ptrType	pPrev;		//��������� �� ���������� ������� ������

	unsigned long long ullElementIndex;				//������ �������� ����� ���� ��������� � ������
	ListBase<ListElement> *pContainer;				//��������� �� ������-�������� ��������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ������ ��������� ������ � �������� �������� ������ ������������������
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
	ElementData ed;		//������

public:

	using ListElement = ListElementCompound_OneLinked<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

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

	typename MemoryPolicy::ptrType pNext = nullptr;		//��������� �� ��������� ������� ������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ����� ������� ������ � �������� �������� ������ �����������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_OneLinked_CP
{
	ElementData ed;		//������

public:

	using ListElement = ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

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

	typename MemoryPolicy::ptrType pNext = nullptr;		//��������� �� ��������� ������� ������

	unsigned long long ullElementIndex = 0;			//������ �������� ����� ���� ��������� � ������
	ListBase<ListElement>* pContainer = nullptr;	//��������� �� ������-�������� ��������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_OneLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ������ ��������� ������ � �������� �������� ������ ������������������
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
	ElementData ed;		//������

public:

	using ListElement = ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

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

	typename MemoryPolicy::ptrType		pNext = nullptr;		//��������� �� ��������� ������� ������
	typename MemoryPolicy::weak_ptrType	pPrev;		//��������� �� ���������� ������� ������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ����� ������� ������ � �������� �������� ������ �����������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy> class ListElementCompound_TwoLinked_CP
{
	ElementData ed;		//������

public:

	using ListElement = ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>;
	using MemoryPolicy = _MemoryPolicy<ListElement>;			//��������� ������������ ��������� ������ � ������� ������ ������ ��������

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

	typename MemoryPolicy::ptrType		pNext = nullptr;		//��������� �� ��������� ������� ������
	typename MemoryPolicy::weak_ptrType	pPrev;		//��������� �� ���������� ������� ������

	unsigned long long ullElementIndex = 0;				//������ �������� ����� ���� ��������� � ������
	ListBase<ListElement>* pContainer = nullptr;		//��������� �� ������-�������� ��������

	//��������� ��������� ����� ������������ ������ ����������������� ��� ����������� ������� �� ����; ����� ����������� ����������������� ����, ���������� �� ����������
	//�������� ���������� �������; ��������� ������������������ ��������� ������������� �����������: https://stackoverflow.com/questions/44213761/partial-template-specialization-for-friend-classes
	template<class ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy, bool bExceptions> friend class List_TwoLinked;

	//��������� ��������� ������� �������� "+" �����������������
	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);

	//��������� ������ ��������� ������ � �������� �������� ������ ������������������
	template<class ListElement, bool bExceptions> friend class DirectSearch;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ListElement, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif
};

}		//����� ����������� ������������ ��� ListMT