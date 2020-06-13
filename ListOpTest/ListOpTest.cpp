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

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>						//�������� ���������� ���� Windows
#include <vector>
#include <utility>							//��� swap
#include <tchar.h>							//��������� �������� Unicode
#include <stdlib.h>							//��� ��������� ����� - ������� rand()
#include "..\List\ListDataExample.h"		//����� ������ � �������
#include "..\List\SearchContainerElement.h"	//��������� ������ ��������
#include "ListList.h"						//����� ������, ��������� ������ ���� ������ � �������

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������� ������� �������
//������ ��������������� ���� ��� ��������
using ListElementData_1L = ListElementData_OneLinked_CP;				//��� ���� ������������ ������
//using ListElementData_1L = ListElementData_OneLinked2_CP<>;			//��� ���� ������������ ������ (������������ ������������ ������� ���������)
using ListElementData_2L = ListElementData_TwoLinked_CP;				//��� ���� ����������� ������
//using ListElementData_2L = ListElementData_TwoLinked2_CP<>;			//��� ���� ����������� ������ (������������ ������������ ������� ���������)

using ListElement = ListElementData_2L;
//using ListBase = List_OneLinked<ListElement, ThreadLockingWin_SRWLock>;
//using ListElement = ListElementData_2L;
//using ListBase = List_TwoLinked<ListElement>;
using List = ListData<List_TwoLinked<ListElement, ThreadLockingWin_SRWLock, SearchByIndex_BitArray, true>, true>;

using List1 = ListData<List_TwoLinked<ListElement, ThreadLockingWin_SRWLock, SearchByIndex_BitArray, false>, false>;

//���������� ���������/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned int c_uiFirstListNumNumbers = 3;					//����� ������� ������
const unsigned int c_uiSecondListNumNumbers = 2;				//����� ������� ������
const unsigned int c_uiThirdListNumNumbers = 3;					//����� �������� ������
const unsigned int c_uiForthListNumNumbers = 4;					//����� ��������� ������

//���������� ����������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

List list1, list2, list3;

//ListData<List_OneLinked<ListElementData_1L>, ListElementData_1L> list1, list2, list3;		//����������� ������
//ListData<List_TwoLinked<ListElementData_2L>, ListElementData_2L> listTwoLinked;		//���������� ������


//�������//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//� ���������� ����������� �������: ���� List list, ���� ListBase& list, ���� List&& list (�� ����� ���� ��� ������ � ������� ����������-�������,
//- PrintList(list), � �� ����������, ������������ std::move)
//��� ���� ���-�� �� ��� �� �������� ��� ������������� ������������� ������� (����� �������� ListBase& list):
//PrintList(list);
//PrintList(list1 + list2);
//������, �������� ������� � �������� ���� ������ ����� ������� ������ ������� ������� ListBase& list.

void PrintList(List&& list)
{
	//������� ���������� ������ �� ����� (���� �������� � ������� ������� � ��������� � ����)
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
	//��������� ������ ��� ����������
	List list;
	_tprintf_s(TEXT("Result list (initialization):\n"));
	for (unsigned int i = 0; i < c_uiFirstListNumNumbers; i++)
	{
		list.Add(0);
		_tprintf_s(TEXT("%u\n"), 0);
	}

	//��������� ������ ������
	_tprintf_s(TEXT("\nFirst list (List1):\n"));
	for (unsigned int i = 0; i < c_uiFirstListNumNumbers; i++)
	{
		list1.Add(i + 1);
		_tprintf_s(TEXT("%u\n"), i + 1);
	}
	//��������� ������ ������
	_tprintf_s(TEXT("\nSecond list (List2):\n"));
	for (unsigned int i = 0; i < c_uiSecondListNumNumbers; i++)
	{
		list2.Add(i + c_uiFirstListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + 1);
	}
	//��������� ������ ������
	_tprintf_s(TEXT("\nThird list (List3):\n"));
	for (unsigned int i = 0; i < c_uiThirdListNumNumbers; i++)
	{
		list3.Add(i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
	}

	List1 list4;
	//��������� �������� ������ ������� ����
	_tprintf_s(TEXT("\nForth list (List4):\n"));
	for (unsigned int i = 0; i < c_uiForthListNumNumbers; i++)
	{
		list4.Add(i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + c_uiThirdListNumNumbers + 1);
		_tprintf_s(TEXT("%u\n"), i + c_uiFirstListNumNumbers + c_uiSecondListNumNumbers + 1);
	}

	_tprintf_s(TEXT("\nList1 and list2 swap:\n"));
	std::swap(list1, list2);			//������ ������� ������
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

	//�������� � �������
	//PrintList(list);
	PrintList(std::move(list));
	PrintList(list1 + list2);

	//��������� ������ ����� ��������� ��������
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

	const List c_list = list;	//������ ����������� ������
	//������� ���
	_tprintf_s(TEXT("\n\nConst list:\n"));
	//listElement = c_list.GetFirst();
	//while (listElement)
	//{
	//	_tprintf_s(TEXT("%I64u\n"), c_list.GetValue(listElement));
	//	listElement = c_list.GetNext(listElement);
	//}
	for(auto& leValue : list)
		_tprintf_s(TEXT("%I64u\n"), leValue->u64Value);
	//c_list.Add(12345, false);		//������ - ���������� �������� � ������������ ������

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

	//������ �������
	const unsigned int c_uiListArraySize = 10;			//����� ��������� �������
	List lstArray[c_uiListArraySize];
	for (List& lst : lstArray)							//C++11: for-���� �� ���������
		lst = list;

	//������ ����������� �������
	const List c_lstArray[c_uiListArraySize] = {list1, list2, list3};	//������ ��� ���������������� ����, ���������, ������, ������������� �� ��������� (������ ������)

	//������ �������
	//������ ��-�������� � ��-��������, ��������� ������ � ����������� ������ ���������� � ����� ������ �������� ������
	//� ���������������� ����� ���������� ���������� �������� ������ �� ����� ������ � �����-���� �� ����������
	//�� � ����� ������ �� ����, �.�. ������ ��� ������ � �������� ����������� �������� ������ ������ ������, � �� � ��������� ���������������
	_tprintf_s(TEXT("\n"));
	ListList<List_TwoLinked<ListElementList_TwoLinked, ThreadLockingWin_SRWLock, DirectSearch, true>> listlist;
	const unsigned char c_ucNumLists = 3;
	unsigned char ucTotalIndex = 0;			//�������� �������� ������ � �������
	srand(GetTickCount());
	for (unsigned char i = 0; i < c_ucNumLists; i++)
	{
		_tprintf_s(TEXT("List %u: "), i);
		auto ptrListInternal = listlist.Add();
		//�� ����� ���� ��� ������� ������� ����� ���������� ������ ������� �������� ������ � ���������� � ���� ��������, ����� �������: �� �����
		//����� ��� ���������� ��� �� ������
		auto& listInternal = ptrListInternal->GetList();
		unsigned char ucListLength = 3 + rand() % 15;
		_tprintf_s(TEXT("%u elements\n"), ucListLength);
		for (unsigned char j = 0; j < ucListLength; j++)
			listInternal.Add(ucTotalIndex++);
	}

	//������� ������
	_tprintf_s(TEXT("\n"));
	auto ptrList = listlist.GetFirst();		//�������� ��������� �� ������ ������� ������
	unsigned char ucListIndex = 0;			//����� ������
	while(ptrList != nullptr)
	{
		_tprintf_s(TEXT("List %u: "), ucListIndex++);
		auto& listInternal = ptrList->GetList();			//�������� ������ � ����������� ������
		auto ptrElementData = listInternal.GetFirst();		//�������� ������ ������� ������
		while (ptrElementData != nullptr)
		{
			_tprintf_s(TEXT("%u "), listInternal.GetValue(ptrElementData));
			ptrElementData = listInternal.GetNext(ptrElementData);		//��������� � ���������� ��������
		}
		_tprintf_s(TEXT("\n"));
		ptrList = listlist.GetNext(ptrList);		//��������� � ���������� ������
	}
}