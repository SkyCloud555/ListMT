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

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "..\List\ListDataExample.h"		//����� ������ � �������
#include "..\List\SearchContainerElement.h"	//��������� ������ ��������

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��� �������� ������ �������

//����������� ������������ (������ ������ ����������� �� ���� ���������, ����� � ListElement_OneLinked/ListElement_TwoLinked ����� ��������������
//�������� �������� �������!!!)

class ListElementList_TwoLinked : public ListElement_TwoLinked_CP<ListElementList_TwoLinked, MemoryPolicy>		//���� ����������� ������
{
public:

	using ListInternal = ListData<List_OneLinked<ListElementData_OneLinked2<>, ThreadLockingWin_SRWLock, DirectSearch, true>, true>;		//��� ����������� ������

private:

	ListInternal list;			//����������� ������ � �������
		
public:

	ListElementList_TwoLinked() = default;	//���������, ��� ��� ���������� ����������� �� ���������
	ListElementList_TwoLinked(ListInternal& list) : list(list) {}		//�����������
	
	ListInternal& GetList(void)
	{
		return list;
	}
	template<class List> friend class ListList;			//��������� ��������� ����� ������ ������� ����������������� ��� ����������� ������� �� ����
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class List> class ListList : public List		//������ � �������
{
public:

	//���������� �����
	using ListElement = typename List::ListElement;				//�������� ��� �������� �� ����������� ������ ���������
	using ptrListElement = typename List::ptrListElement;		//��������� ��� ��������� �� ����������� ������ ���������

	//������������

	ListList() {}		//����������� ��� ���������� (����� ��� ����������� ������������� �������� �� �������� �������� ������ ����)

	//������������ ��� �������� �� �������� �������� ������
	ListList(List& list) : List(list) {}
	ListList(List&& list) : List(std::move(list)) {}		//noexcept ������-�� �� ����������� ��� �������� ������������ �������� ������

	//�������

	ptrListElement Add(bool bProtected = false) noexcept					//��������� ����� ������ ������ � ������� � ����� ������
	{
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� ������� ����������� ������

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ptrListElement pCurr = nullptr;
		try
		{
			pCurr = List::AddLast(true);			//��������� ����� ������� � ����� ������
		}
		catch (typename ListErrors::FailElemCreation)					//�� ������� �������� ������ ��� ����
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return nullptr;
		}

		if (!bProtected)
			List::UnlockListExclusive();						//������������ ������

		return pCurr;
	}

	ptrListElement Add(ptrListElement pElem, bool bProtected = false) noexcept		//��������� ����� ������� ������, ��������� �������� �������� ��� �������������
	{
		//�� ����: pElem - ��������� �� ������� ������, ����� �������� ���������� �������� ����� ����
		//� ����� ������; bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� ������� ����������� ������

		if (!pElem)					//������� ������� ���������
			return nullptr;

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//���� ��������� ������� �������� ��������
		while (true)						//��������� �������, ���� �� ������� ������� �������
		{
			try
			{
				pCurr = List::AddAfter(pElem, true, u64Value);			//��������� ����� ������� ������
			}
			catch (typename ListErrors::NotPartOfList)						//���������� ������� ��� �� �������� ������ ������ - ��������� �������
			{
				bTryAgain = true;
			}
			catch (typename ListErrors::ListError)						//��������� ������ ������ - �� ���������� � �������, �������
			{
				if (!bProtected)
					List::UnlockListExclusive();				//������������ ������
				return nullptr;
			}

			if (bTryAgain)
			{
				pElem = List::GetFirst(true);		//������������� ���������� ���������� ������� �� �������, ���� ���������� ��� �� �������� ������ ������
				bTryAgain = false;
				continue;					//��������� ������� ��� ������� ��������
			}
			else
				break;									//������� �� �����, ���� ������� ������ ������
		}

		if (!bProtected)
			List::UnlockListExclusive();				//������������ ������

		return pCurr;
	}

	//��������

	ListList& operator=(List& list)
	{
		//�������� ����������� ������������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		List::MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	ListList& operator=(List&& list) noexcept
	{
		//�������� ������������� ������������
		//�� ����: list - rvalue-������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		//���������� �������� �������� ������
		//��������� std::move, ����� ������������� �������� list � rvalue (���� ��� ��� ����� lvalue, ������ ���������� �������), � �������
		//���������� ������ �������� �������� ������
		List::operator=(std::move(list));
		return *this;
	}
};
