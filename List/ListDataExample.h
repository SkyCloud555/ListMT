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

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "List.h"				//�������� ������
#include "ListE.h"				//�������� ������ (� ������������)

//���������� ������������ ��� �������������� ������ (��� ��������� ������� ����� ������������ �� ������������ using-����������)
using namespace ListMT;

//���������� �������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class List, bool bExceptions> class ListData;					//������ � �������

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//�������� ��������� ������ � �������
//��������� ������ � ������� ����� ���������� ���������
//template<class Type> using MemoryPolicy = InternalPointer<Type>;
//��������� �������������, �� � ����� ��������� ��������������� � ���������� ������ (��� ������� ��������� � ����������)
//template<class Type> using MemoryPolicy = InternalPointer<Type, DefaultDeleter_InternalPointer<Type>, DefaultAllocator_InternalPointer<Type>>;

template<class Type> using MemoryPolicy = SmartSharedPointer<Type>;		//�������� ��������� ������ � �������

//���������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���� ������

//����������� ������������ (������ ������ ����������� �� ���� ���������, ����� � ListElement_OneLinked/ListElement_TwoLinked ����� ��������������
//�������� �������� �������!!!)

class ListElementData_OneLinked : public ListElement_OneLinked<ListElementData_OneLinked, MemoryPolicy>		//���� ������������ ������
{
	unsigned __int64 u64Value = 0;						//����� �������� ����
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_OneLinked(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

class ListElementData_TwoLinked : public ListElement_TwoLinked<ListElementData_TwoLinked, MemoryPolicy>		//���� ����������� ������
{
	unsigned __int64 u64Value = 0;						//����� �������� ����
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_TwoLinked(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

class ListElementData_OneLinked_CP : public ListElement_OneLinked_CP<ListElementData_OneLinked_CP, MemoryPolicy>		//���� ������������ ������
{
	unsigned __int64 u64Value = 0;						//����� �������� ����
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_OneLinked_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

class ListElementData_TwoLinked_CP : public ListElement_TwoLinked_CP<ListElementData_TwoLinked_CP, MemoryPolicy>		//���� ����������� ������
{	
	//unsigned char ucSomeData[1024 - sizeof(decltype(u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	unsigned __int64 u64Value = 0;						//����� �������� ����

	ListElementData_TwoLinked_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

//������������ ������������ (��������� �������� ��������� ��������� ��� ������ � ����� �������)

class thisclass {};			//�����-��������, ������������ ��� ��������� �� ��������� � ������, ����������� �� ListElementData_OneLinked/ListElementData_TwoLinked

//�������� ������������ ������

template<class DerivedListElement = thisclass> class ListElementData_OneLinked1 : public ListElement_OneLinked<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked1<>, DerivedListElement>, MemoryPolicy>
{
	unsigned __int64 u64Value = 0;						//����� �������� ����

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_OneLinked1(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked2 : public ListElementData_OneLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_OneLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_OneLinked2(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked1_CP : public ListElement_OneLinked_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked1_CP<>, DerivedListElement>, MemoryPolicy>
{
	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	unsigned __int64 u64Value = 0;						//����� �������� ����

	ListElementData_OneLinked1_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_OneLinked2_CP : public ListElementData_OneLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2_CP<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_OneLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_OneLinked2_CP<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_OneLinked2_CP(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//�����������
};

//�������� ����������� ������

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked1 : public ListElement_TwoLinked<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked1<>, DerivedListElement>, MemoryPolicy>
{
	unsigned __int64 u64Value = 0;						//����� �������� ����

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_TwoLinked1(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked2 : public ListElementData_TwoLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_TwoLinked1<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_TwoLinked2(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked1_CP : public ListElement_TwoLinked_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked1_CP<>, DerivedListElement>, MemoryPolicy>
{
	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	unsigned __int64 u64Value = 0;						//����� �������� ����

	ListElementData_TwoLinked1_CP(unsigned __int64 u64Value = 0) : u64Value(u64Value) {}		//�����������
};

template<class DerivedListElement = thisclass> class ListElementData_TwoLinked2_CP : public ListElementData_TwoLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2_CP<>, DerivedListElement>>
{
	using ListElementBase = ListElementData_TwoLinked1_CP<std::conditional_t<std::is_same<DerivedListElement, thisclass>::value, ListElementData_TwoLinked2_CP<>, DerivedListElement>>;

	unsigned char ucSomeData[1024 - sizeof(decltype(ListElementBase::u64Value))];	//��������� ��������� ������ ��� ���������� ������

	template<class List, bool bExceptions> friend class ListData;			//��������� ��������� ����� ������ � ������� ����������������� ��� ����������� ������� �� ����

public:

	ListElementData_TwoLinked2_CP(unsigned __int64 u64Value = 0) : ListElementBase(u64Value) {}		//�����������
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class _List> class ListData<_List, false> : public _List		//������ � ������� (��� ����������)
{
public:

	//���������� �����
	using List = _List;
	using ListElement = typename List::ListElement;				//�������� ��� �������� �� ����������� ������ ���������
	using ptrListElement = typename List::ptrListElement;		//��������� ��� ��������� �� ����������� ������ ���������

	//������������

	ListData() {}		//����������� ��� ���������� (����� ��� ����������� ������������� �������� �� �������� �������� ������ ����)

	//������������ ��� �������� �� �������� �������� ������
	ListData(List& list) : List(list) {}
	ListData(List&& list) : List(std::move(list)) {}		//noexcept ������-�� �� ����������� ��� �������� ������������ �������� ������

	//�������

	ListErrorCodes::eListErrorCode Add(const unsigned __int64 u64Value, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept					//��������� ����� ������� � ����� ������, ��������� �������� �������� ��� �������������
	{
		//�� ����: u64Value - ��������, ������� ���� �������� � ����� ������; pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ���
		//�������� ��� ��������� ������ ��������, bUsePreviousElementIndex - ������������ ������ ����������� �������� ��� ��������� �������� ���������� �
		//��������� ���������� ������ �������� ����� �������

		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ptrListElement pCurr = nullptr;
		pCurr = List::AddLast(&eError, true, bUsePreviousElementIndex, u64Value);			//��������� ����� ������� � ����� ������
		if (pCurr == nullptr && eError == ListErrorCodes::eListErrorCode::FailElemCreation)
		{
			if (!bProtected)
				List::UnlockListExclusive();						//������������ ������
			return eError;
		}

		if (!bProtected)
			List::UnlockListExclusive();						//������������ ������

		return eError;
	}

	ListErrorCodes::eListErrorCode Add(ptrListElement pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept
	{
		//��������� ����� ������� ������, ��������� �������� �������� ��� �������������
		//�� ����: pElem - ��������� �� ������� ������, ����� �������� ���������� �������� ����� ����, u64Value - ��������, ������� ���� ��������
		//� ����� ������; bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������

		if (!pElem)					//������� ������� ���������
			return ListErrorCodes::eListErrorCode::Nullptr;

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//���� ��������� ������� �������� ��������
		while (true)						//��������� �������, ���� �� ������� ������� �������
		{
			pCurr = List::AddAfter(pElem, &eError, true, false, u64Value);			//��������� ����� ������� ������
			if (!pCurr)
			{
				if(eError == ListErrorCodes::eListErrorCode::NotPartOfList)	//���������� ������� ��� �� �������� ������ ������ - ��������� �������
					bTryAgain = true;
				else if (eError != ListErrorCodes::eListErrorCode::Success)	//��������� ������ ������ - �� ���������� � �������, �������
				{
					if (!bProtected)
						List::UnlockListExclusive();				//������������ ������
					return eError;
				}
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
		//pCurr->u64Value = u64Value;						//��������� ���������� �������� � ���� ������

		if (!bProtected)
			List::UnlockListExclusive();				//������������ ������

		return eError;
	}

	ptrListElement FindElementByValue(const unsigned __int64 u64Value) const noexcept
	{
		//���� � ������ ������ �������, �������� �������� ��������� � ��������
		//�� ����: u64Value - �������� �������� ��������
		//�� �����: ��������� �� ������� ���� nullptr, ���� ������� �� ������ (���� ������ ����)

		List::LockListShared();					//��������� ������ �� ������

		ptrListElement pCurr = List::GetFirst(true);
		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;	//��� ������
		while (pCurr)
		{
			if (pCurr->u64Value == u64Value)
				break;

			pCurr = List::GetNext(pCurr, &eError, true, true);
		}

		List::UnlockListShared();					//������������ ������ �� ������

		return pCurr;
	}

	unsigned __int64 ComputeAverageValue(void) const noexcept
	{
		//��������� ������� �������� ��������� ����� ������

		List::LockListShared();					//��������� ������ �� ������

		ptrListElement pCurr = List::GetFirst(true);
		unsigned __int64 u64AverageValue = 0;		//����������� ������� ��������
		unsigned int uiNumElements = 0;				//���������� �����
		while (pCurr)
		{
			u64AverageValue += pCurr->u64Value;
			uiNumElements++;
			pCurr = GetNext(pCurr, nullptr, true, true);
		}
		u64AverageValue /= uiNumElements;

		List::UnlockListShared();					//������������ ������ �� ������
		return u64AverageValue;
	}

	unsigned __int64 GetValue(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//���������� �������� ����������� �� ��������� ��������
		//�� ����: pElem - ��������� �� ������� ������, �������� �������� ���� ��������; pErrorCode - ��������� �� ������������ ��� ������, bProtected - ����
		//����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: �������� ��������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return 0;
		}

		if (!bProtected)
			List::LockListShared();					//��������� ������ �� ������
		//���������, �������� �� ���������� ������� ������ ������
		if (!List::FindElement(pElem, pErrorCode, true))
		{			
			if (!bProtected)
				List::UnlockListShared();
			return 0;
		}
		unsigned __int64 u64Value = pElem->u64Value;
		if (!bProtected)
			List::UnlockListShared();					//������������ ������ �� ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return u64Value;
	}

	ListErrorCodes::eListErrorCode SetValue(ptrListElement const pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept
	{
		//���������� �������� ����������� �� ��������� ��������
		//�� ����: pElem - ��������� �� ������� ������, �������� �������� ���� ����������; u64Value - ��������������� ��������; bProtected - ���� ����, ���
		//�������� ��� ��������� ������, �������� ����� �������
		//�� �����: �������� ��������

		if (!pElem)
			return ListErrorCodes::eListErrorCode::Nullptr;

		if (!bProtected)
			List::LockListExclusive();					//��������� ������ �� ������
		//���������, �������� �� ���������� ������� ������ ������
		ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
		if (!List::FindElement(pElem, &eError, true))
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return eError;
		}
		pElem->u64Value = u64Value;
		if (!bProtected)
			List::UnlockListExclusive();					//������������ ������ �� ������

		return eError;
	}

	//��������

	ListData& operator=(List& list) noexcept
	{
		//�������� ����������� ������������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		List::MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	ListData& operator=(List&& list) noexcept
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class _List> class ListData<_List, true> : public _List		//������ � ������� (� ������������)
{
public:

	//���������� �����
	using List = _List;
	using ListElement = typename List::ListElement;				//�������� ��� �������� �� ����������� ������ ���������
	using ptrListElement = typename List::ptrListElement;		//��������� ��� ��������� �� ����������� ������ ���������

	//������������

	ListData() {}		//����������� ��� ���������� (����� ��� ����������� ������������� �������� �� �������� �������� ������ ����)

	//������������ ��� �������� �� �������� �������� ������
	ListData(List& list) : List(list) {}
	ListData(List&& list) : List(std::move(list)) {}		//noexcept ������-�� �� ����������� ��� �������� ������������ �������� ������

	//�������

	bool Add(const unsigned __int64 u64Value, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept					//��������� ����� ������� � ����� ������, ��������� �������� �������� ��� �������������
	{
		//�� ����: u64Value - ��������, ������� ���� �������� � ����� ������; bProtected - ���� ����, ��� �������� ��� ��������� ������ ��������,
		//bUsePreviousElementIndex - ������������ ������ ����������� �������� ��� ��������� �������� ���������� � ��������� ���������� ������ ��������
		//����� ������� ����������� ������

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ptrListElement pCurr = nullptr;
		try
		{
			pCurr = List::AddLast(true, bUsePreviousElementIndex, u64Value);			//��������� ����� ������� � ����� ������
		}
		catch (typename ListErrors::FailElemCreation)					//�� ������� �������� ������ ��� ����
		{
			if (!bProtected)
				List::UnlockListExclusive();
			return false;
		}
		//pCurr->u64Value = u64Value;					//��������� ���������� �������� � ���� ������

		if (!bProtected)
			List::UnlockListExclusive();						//������������ ������

		return true;
	}

	bool Add(ptrListElement pElem, const unsigned __int64 u64Value, bool bProtected = false) noexcept		//��������� ����� ������� ������, ��������� �������� �������� ��� �������������
	{
		//�� ����: pElem - ��������� �� ������� ������, ����� �������� ���������� �������� ����� ����, u64Value - ��������, ������� ���� ��������
		//� ����� ������; bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� ������� ����������� ������

		if (!pElem)					//������� ������� ���������
			return false;

		if (!bProtected)
			List::LockListExclusive();				//��������� ������ �� ����� ��������

		ptrListElement pCurr = nullptr;
		bool bTryAgain = false;				//���� ��������� ������� �������� ��������
		while (true)						//��������� �������, ���� �� ������� ������� �������
		{
			try
			{
				pCurr = List::AddAfter(pElem, true, false, u64Value);			//��������� ����� ������� ������
			}
			catch (typename ListErrors::NotPartOfList)						//���������� ������� ��� �� �������� ������ ������ - ��������� �������
			{
				bTryAgain = true;
			}
			catch (typename ListErrors::ListError)						//��������� ������ ������ - �� ���������� � �������, �������
			{
				if (!bProtected)
					List::UnlockListExclusive();				//������������ ������
				return false;
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
		//pCurr->u64Value = u64Value;						//��������� ���������� �������� � ���� ������

		if (!bProtected)
			List::UnlockListExclusive();				//������������ ������

		return true;
	}

	ptrListElement FindElementByValue(const unsigned __int64 u64Value) const noexcept
	{
		//���� � ������ ������ �������, �������� �������� ��������� � ��������
		//�� ����: u64Value - �������� �������� ��������
		//�� �����: ��������� �� ������� ���� nullptr, ���� ������� �� ������ (���� ������ ����)

		List::LockListShared();					//��������� ������ �� ������

		ptrListElement pCurr = List::GetFirst(true);
		while (pCurr)
		{
			if (pCurr->u64Value == u64Value)
				break;

			pCurr = List::GetNext(pCurr, true, true);
		}

		List::UnlockListShared();					//������������ ������ �� ������

		return pCurr;
	}

	unsigned __int64 ComputeAverageValue(void) const noexcept
	{
		//��������� ������� �������� ��������� ����� ������

		List::LockListShared();					//��������� ������ �� ������

		ptrListElement pCurr = List::GetFirst(true);
		unsigned __int64 u64AverageValue = 0;		//����������� ������� ��������
		unsigned int uiNumElements = 0;				//���������� �����
		while (pCurr)
		{
			u64AverageValue += pCurr->u64Value;
			uiNumElements++;
			pCurr = GetNext(pCurr, true, true);
		}
		u64AverageValue /= uiNumElements;

		List::UnlockListShared();					//������������ ������ �� ������
		return u64AverageValue;
	}

	unsigned __int64 GetValue(ptrListElement const pElem, bool bProtected = false) const
	{
		//���������� �������� ����������� �� ��������� ��������
		//�� ����: pElem - ��������� �� ������� ������, �������� �������� ���� ��������; bProtected - ���� ����, ��� �������� ��� ��������� ������
		//�������� ����� ������� ����������� ������
		//�� �����: �������� ��������

		if (!pElem)
			throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

		if (!bProtected)
			List::LockListShared();					//��������� ������ �� ������
		//���������, �������� �� ���������� ������� ������ ������
		if (!List::FindElement(pElem, true))
		{
			if (!bProtected)
				List::UnlockListShared();
			throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
		}
		unsigned __int64 u64Value = pElem->u64Value;
		if (!bProtected)
			List::UnlockListShared();					//������������ ������ �� ������

		return u64Value;
	}

	void SetValue(ptrListElement const pElem, const unsigned __int64 u64Value, bool bProtected = false)
	{
		//���������� �������� ����������� �� ��������� ��������
		//�� ����: pElem - ��������� �� ������� ������, �������� �������� ���� ����������; u64Value - ��������������� ��������; bProtected - 
		//���� ����, ��� �������� ��� ��������� ������, �������� ����� ������� ����������� ������
		//�� �����: �������� ��������

		if (!pElem)
			throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

		if (!bProtected)
			List::LockListExclusive();					//��������� ������ �� ������
		//���������, �������� �� ���������� ������� ������ ������
		if (!List::FindElement(pElem, true))
		{
			if (!bProtected)
				List::UnlockListExclusive();
			throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
		}
		pElem->u64Value = u64Value;
		if (!bProtected)
			List::UnlockListExclusive();					//������������ ������ �� ������
	}

	//��������

	ListData& operator=(List& list)
	{
		//�������� ����������� ������������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		List::MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	ListData& operator=(List&& list) noexcept
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