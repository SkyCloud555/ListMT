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

//Main ListMT header: list classes definitions without exception support.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <type_traits>					//������ � ������ - ����������� ����� ��� ��������� �����
#include <algorithm>					//���������� �������� � ��������� (std::min � std::max)
#include "..\COMMON POLICIES\CommonPolicies.h"				//������������� ���������
#include "ListElement.h"				//�������� ������

#ifdef min
#undef min
#endif

namespace ListMT						//������������ ��� �������������� ������ (������������ � �����������)
{

//���������� ���������/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//� ������ ������� � �������� ���� ������ ������ ��������� ��������� �� ������ ������ � ����������� ���������� �� ���� - ���� true, ����� - � ������������
//���������� (���� false)
constexpr inline bool ce_bGetMinLinksList = true;

//���������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������� ����� ������

template<class ListElement> class ListBase
{
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ������ � ���������� ��� ��������� ������ � �������

protected:

	//��������� �� ���� ������
	ptrListElement pFirst{ nullptr };			//��������� �� ������ ���� ������ (C++ 11: ������������� ����� ������)
	ptrListElement pLast{ nullptr };			//��������� �� ��������� ���� ������ (C++ 11: ������������� ����� ������)
};

//������ �������

//�������� ���������� ������� ������: _ListElement - ��� �������� ������, LockingPolicy - ��������� ���������� ������� (��. LockingPolicies.h),
//CheckingPresenceElementPolicy - ��������� �������� ������� �������� � ������, bExceptions - ���� ��������� ����������
//����� ����������� ��������� ������� ������
template<class ListElement, class LockingPolicy = ThreadLocking_STDMutex, template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_OneLinked;
template<class ListElement, class LockingPolicy = ThreadLocking_STDMutex, template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_TwoLinked;

//����������� ������
template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_OneLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, false> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, false>
{
public:

	//���������� �����
	using ListElement = _ListElement;								//��������� ���������� � ��������� ������� ��� �������� ��� ������� � ���� �� ����������� �������
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ������ � ���������� ��� ��������� ������ � �������
	
	//��������, ��� ���������� ��������� �++ (Type *) ����� �������������� ������ �� ���������� �������� �������� ������� ������ (DirectSearch)
	static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
		std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>> == false),
		"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

private:

	//�������� � ���������

	bool bTemporary = false;	//���� ���������� ������, �����������, ��� ����������� ������ �������� ���������, � ������, ��� �������� � ���
								//������ ����������� ��-������� (C++ 11: ������������� ����� ������)

	static constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;		//������������ ���������� ���������, ����������� � �������� ������ ������ (��� ������������� ����������� ��������� �������� ������� ��������)

	//��������������� �������
	ptrListElement FindPreviousElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//��������� ����� ��������, ������� ��������� �� ������������ �������
		//�� ����: pElem - ��������� �� ������������ �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ���
		//��������� ������ �������� ����� �������
		//�� �����: ��������� �� ���������� ������� ������������ �������� ���� nullptr, ���� ����� ������� �� ������ ���� ������ ����

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}
		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		if (pElem == ListBase<ListElement>::pFirst)
		{
			if (!bProtected)
				UnlockListShared();		//������������ ������
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return nullptr;
		}
		while (pCurr != nullptr)
		{
			if (pCurr->pNext == pElem)
				break;
			pCurr = pCurr->pNext;
		}
		if (!bProtected)
			UnlockListShared();		//������������ ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return pCurr;
	}

	void SetEmpty(bool bProtected = false) noexcept
	{
		//���������� ������, ������������ ��������� �� ������ � ��������� �������� � ������� ��������; ���� �������� �� ������ �� ���������,
		//�.�. �������� ������ ��������� �������� � ������
		//������� ������������ � �������� �������� ����������� �������� ������, � ����� ��� ������ �� ���������� �������� � ����������
		//����������/����������� �������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������		

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = nullptr;
		CheckingPresenceElementPolicy<ListElement, false>::Clear();

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected, ListErrorCodes::eListErrorCode* pErrorCode = nullptr) noexcept
	{
		//��������������� �������, ����������� ������ ���� ������� Delete � Remove ����������� �������� ��������������� �����
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; bDelete - ����, �����������, ������� ������� �� ������ ��� ������
		//��������� ��� �� ������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ������� (��������� ���
		//���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ���������
		//������ �������� ����� �������, pErrorCode - ��������� �� ������������ ��� ������
		//�� �����: true - �������, false - ������ ���� ���� ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			if (!bProtected)
				UnlockListExclusive();	//������������ ������
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return false;			//������ ����
		}

		//���������� ���� bSerial = true, ��������� �������� ������������ ������� �������� � ������ ����� ����������� ����, ������ �� �����
		//������ �������� ��������� �� ��������� �������
		ptrListElement pNext = GetNext(pElem, pErrorCode, true, true);
		if (pElem == ListBase<ListElement>::pFirst)		//���������� ������� �������� ������ ���������
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
			if (bDelete)
				ListElement::MemoryPolicy::Delete(pElem);
			ListBase<ListElement>::pFirst = pNext;
			if (!bProtected)
				UnlockListExclusive();		//������������ ������
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return true;
		}

		ptrListElement pPrev = FindPreviousElement(pElem, pErrorCode, true);		//���� �������, ����������� �� ����������
		if (pPrev)
		{
			pPrev->pNext = pNext;
			if (bCheckPresence == true && pPrev == pElem)
			{
				if (pElem == ListBase<ListElement>::pLast)
					ListBase<ListElement>::pLast = pPrev;
				CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
				if (bDelete)
					ListElement::MemoryPolicy::Delete(pElem);
			}
		}

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return true;
	}

public:

	//�������

	List_OneLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ce_ullNumElementsMax)		//�����������
	{
		//�� ����: bTemporary - ���� ����, ��� ������ �������� ��� ���������; ���� ���� ��������� � ����� ��������� ���������� ������

		List_OneLinked::bTemporary = bTemporary;
	}

	List_OneLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ullNumElementsMax)	//����������� ��� ������������� ��������� ������ ���������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������� ����� ���� ������� � ������ �� ����� ��� ������; ��� �������� ��������� � �����
		//��������� ������ ��������

		List_OneLinked::bTemporary = false;
	}

	List_OneLinked(const List_OneLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, false>(list)	//����������� �����������
	{
		MakeCopy(list);				//������ ������ ����� ����������� ������
	}

	List_OneLinked(List_OneLinked&& list) : CheckingPresenceElementPolicy<ListElement, false>(0)		//����������� �����������: ������ ��� �������� ��������� �������� � ����������
	{
		//����������� ��������� �� �������� ���������� ������������� � ��������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		CheckingPresenceElementPolicy<ListElement, false>::AssignAnotherCES(std::move(list), this);
		list.SetEmpty();
	}

	~List_OneLinked() noexcept					//����������
	{
		if (!bTemporary)
			Empty();							//������� ��� �������� �� ������, ���� ������ �� ���������
	}

	//��������������� �������

	static constexpr unsigned int GetLinksNumber(void) { return 1; }		//����� ������ ������

	bool FindElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ���
		//��������� ������ �������� ����� �������
		//�� �����: true - ������� ������ � ������������ � ������, false - ����� ������� �� ������ ���� ������ ����

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������

		bool bResult = CheckingPresenceElementPolicy<ListElement, false>::FindElement(pElem, static_cast<const List_OneLinked *>(this));

		if (!bProtected)
			UnlockListShared();		//������������ ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return bResult;
	}

	//������� ������ �� �������

	template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ����� ����������� � ���������� ����
		//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� ������� ����� ����, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� �����������
		//������������ ������������ �������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� ��
		//������ ������� ���, ������������ ������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ ��
		//�����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
		//���� ������ ����, �� �������� pElem ������������
		//�� �����: ��������� �� ��������� ������� ������

		if (pElem == nullptr && pErrorCode != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode != nullptr)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pElem, pErrorCode, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� ������ �� ������ ��� ���������
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pCurr;
			pCurr->pNext = pNext;
			//���������, �������� �� ������� ������� ���������
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;			

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ����� ���������� � ���������� �����
		//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� ������� ����� ����, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� �������������
		//������� ����������� ������������ ������������ �������� (��� ������� �� ���������� ������ ��������
		//SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ���
		//� ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������ ������� ���������� ���������� �������� - ���
		//���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args - ��������� ��������� �����,
		//������������ ������������ ListElement ���� ������ ����, �� �������� pElem ������������
		//�� �����: ��������� �� ��������� ������� ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode != nullptr)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		ptrListElement pPrev = FindPreviousElement(pElem, true);
		if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		//���������, �������� �� ��� ������ ��������� ������ (� ������, ������ �� ���� �������� ���������� �������� ��� ����)
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ���������� �������� ������ �� ������ ��� ���������
			pPrev->pNext = pCurr;
			pCurr->pNext = pElem;
			//���������, �������� �� ������� ������� ������
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
			{
				if (pPrev)
					ullElementIndexStart = pPrev->ullElementIndex;
			}
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddLast(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� ����� ���������� �������� ������
		//�� ����: pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������ �������� (��� ������� �� ����������
		//������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ���������� ������ �������� � ������ ����
		//��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������ ������� ����������
		//���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args -
		//��������� ��������� �����, ������������ ������������ ListElement
		//�� �����: ��������� �� ��������� ������� ������

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������ ��� ���������
			ListBase<ListElement>::pLast->pNext = pCurr;
			pCurr->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			ListBase<ListElement>::pLast = pCurr;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
				*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddFirst(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ������
		//�� ����: pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//Args - ��������� ��������� �����, ������������ ������������ ListElement
		//�� �����: ��������� �� ��������� ������� ������

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������� ������ �������
			ptrListElement pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pCurr;
			pCurr->pNext = pNext;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	bool InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� � ������ ����� ����������� � ���������� ����
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
		//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
		//�������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		//���� ������ ����, �� �������� pElem ������������

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� ������ �� ������ ��� ���������
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pElemToInsert;
			pElemToInsert->pNext = pNext;
			//���������, �������� �� ������� ������� ���������
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� � ������ ����� ���������� � ���������� �����
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
		//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
		//�������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		//���� ������ ����, �� �������� pElem ������������

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		ptrListElement pPrev = FindPreviousElement(pElem, true);
		if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//���������, �������� �� ��� ������ ��������� ������ (� ������, ������ �� ���� �������� ���������� �������� ��� ����)
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ���������� �������� ������ �� ������ ��� ���������
			pPrev->pNext = pElemToInsert;
			pElemToInsert->pNext = pElem;
			//���������, �������� �� ������� ������� ������
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pPrev->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return true;
	}

	bool InsertLast(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� ����� ���������� �������� ������
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ����� ������
		//�� ����: pElemToInsert - ��������� �� ����������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������ ��� ���������
			ListBase<ListElement>::pLast->pNext = pElemToInsert;
			pElemToInsert->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			ListBase<ListElement>::pLast = pElemToInsert;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return true;
	}

	bool InsertFirst(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ����� ������� � ������ ������
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ ������
		//�� ����: pElemToInsert - ��������� �� ����������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - �������

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������� ������ �������
			ptrListElement pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pElemToInsert;
			pElemToInsert->pNext = pNext;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool Delete(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
	{
		//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; pErrorCode - ��������� �� ������������ ��� ������, bCheckPresence - ���� ��������
		//������� �������� � ������ ������, ��� ��� ������� (��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������),
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - ������ ���� ���� ������

		//�������� ���������� ��������������� �������, ������ �� ������� ������� ������ �� ������
		return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected, pErrorCode);
	}

	bool Remove(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//��������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
	{
		//��������� ��������� ������� �� ������, ��� ������� ��������� �� ��������� �� ������
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; pErrorCode - ��������� �� ������������ ��� ������, bCheckPresence - ���� ��������
		//������� �������� � ������ ������, ��� ��� ������� (��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������),
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - ������ ���� ���� ������

		//�������� ���������� ��������������� �������, ������ �� ��������� ������� �� ������ ��� �������� �� ������
		return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected, pErrorCode);
	}

	ptrListElement Find(const ListElement& liElem, bool bProtected = false) const noexcept
	{
		//��������� ����� �������� � ���������� ��������� �� ����, ���� �� ������; ������� ������ ������ ������������ �������� ==
		//�� ����: liElem - ������ �� ������������ �������; ����������� ����� �� �����������, � �� �������� ����������� �������� � ������ (liElem � �����
		//������ �� ����������� ������), bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������� ������, ���� ������� � �������� ���������� ������, ��� nullptr � ��������� ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (*pCurr == liElem)
			{
				if (!bProtected)
					UnlockListShared();			//������������ ������ �� ������
				return pCurr;
			}
			pCurr = pCurr->pNext;
		}

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		return ptrListElement(nullptr);
	}

	unsigned long long CalculateElementsNumber(bool bProtected = false) const noexcept
	{
		//������������ ���������� ����� � ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ���������� ��������� � ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, false>::GetNumElements();			//����� ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullNumElements;
	}

	unsigned long long FindElementNumber(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//���� ���������� ����� �������� � ������
		//�� ����: pElem - ������� ������, ����� �������� ���� �����; pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������
		//�� �����: ����� �������� � ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullElementNumber = 0;			//����� ��������
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (pCurr == pElem)
				break;
			ullElementNumber++;
			pCurr = pCurr->pNext;
		}
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return ullElementNumber;
	}

	void MakeCopy(const List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
		//�� ����: list - ������ �� ���������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ���������� ������ �� ����� ��������
		list.LockListShared();

		Empty(true);		//������� ������
		ptrListElement pCurrToCopy = list.GetFirst(true);							//�������� ������ ������� ����������� ������
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//��������� �� ������� � ���������� �������� (������� ������)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);	//������ ����� �������, ������������� ��� ��������� ����������� ������
			if (pCurr == nullptr)										//������� ��� ��������� ������ - ���������� ����������
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//������������ ���������� ������
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//��������� � ���������� �������� ����������� ������
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ���������� ������
		list.UnlockListShared();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		
	}

	template<class List> void MakeCopy(const List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ���������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ���������� ������ �� ����� ��������
		list.LockListShared();

		Empty(true);		//������� ������
		ptrListElement pCurrToCopy = list.GetFirst(true);							//�������� ������ ������� ����������� ������
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//��������� �� ������� � ���������� �������� (������� ������)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);	//������ ����� �������, ������������� ��� ��������� ����������� ������
			if (pCurr == nullptr)										//������� ��� ��������� ������ - ���������� ����������
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//������������ ���������� ������
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//��������� � ���������� �������� ����������� ������
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ���������� ������
		list.UnlockListShared();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		
	}

	void Empty(bool bProtected = false) noexcept
	{
		//���������� ������, ��������� ������ ��� ��� �������� �� ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//������� ��� �������� ������
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		ptrListElement pNext = nullptr;
		while (pCurr != nullptr)
		{
			pNext = pCurr->pNext;
			ListElement::MemoryPolicy::Delete(pCurr);
			pCurr = pNext;
		}
		SetEmpty(true);					//������������� ��������� � ������� ��������

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListAfter(List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)			
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListAfter(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)			
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListBefore(List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListBefore(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	List_OneLinked Split(ptrListElement const pSplitElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ������� ������, ������� ��� ����� ���������� ��������
		//�� ����: pSplitElem - ��������� �� �������, ����� �������� ����������� ��������� ������; pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ����� ������, ���������� ���������� �����

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		if (!pSplitElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pSplitElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		List_OneLinked list(CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax(), true);				//������ ��������� ������, � ������� ����� ��������� ���������
		//���������, �������� �� ���������� ������� ���������; ���� ���, �� ��������� ������; ���� ��, �� ������ �� ������ � ����������
		//������ ��������� list
		if (pSplitElem != ListBase<ListElement>::pLast)
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveContainer(iterator(pSplitElem->pNext, this));

			list.pFirst = pSplitElem->pNext;
			pSplitElem->pNext = nullptr;
			list.pLast = ListBase<ListElement>::pLast;
			ListBase<ListElement>::pLast = pSplitElem;

			bool bRegisterContainer = list.CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(&list, List_OneLinked::iterator{ list.pFirst, &list });
			if (!bRegisterContainer && pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		}

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return list;
	}

	bool IsEmpty(bool bProtected = false) const noexcept
	{
		//���������, �������� �� ������ ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - ������ ����, false - ������ �� ����

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		bool bResult = false;
		if (ListBase<ListElement>::pFirst == nullptr && ListBase<ListElement>::pLast == nullptr)
			bResult = true;

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		return bResult;
	}

	unsigned long long GetNumElementsMax_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//���������� ������������ ���������� ���������, ������������� ���������� ������ ���������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ���������� ��������� � ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax();			//������������ ���������� ���������, ������������� ���������� ������ ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullNumElementsMax;
	}

	unsigned long long GetCurrentElementIndex_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//���������� ������� ������, ������������ ��� ������ ������������ �������� ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ������� ������, ������������ ��� ������ ������������ �������� ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, false>::GetCurrentElementIndex();			//������������ ���������� ���������, ������������� ���������� ������ ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullCurrentElementIndex;
	}

	//���������� � ������������� ������

	void LockListExclusive(void) const noexcept				//��������� ������ ��� ���������
	{
		LockingPolicy::LockExclusive();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
	}

	void UnlockListExclusive(void) const noexcept			//������������ ������ ����� ���������
	{
		LockingPolicy::UnlockExclusive();					//���������� � ���������� ����� �������� ������� ��������� ���������� �������
	}

	void LockListShared(void) const noexcept				//��������� ������ ��� ������
	{
		LockingPolicy::LockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
	}

	void UnlockListShared(void) const noexcept				//������������ ������ ����� ������
	{
		LockingPolicy::UnlockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
	}

	//����������� �� ����� ������

	ptrListElement GetFirst(bool bProtected = false) const noexcept
	{
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ ������� ������

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetFirstAndRemove(bool bProtected = false) const noexcept
	{
		//���������� ������ ������� ������ � ������� ��� �� ������ �� ���� ��������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ �������, ��� �� ���������� ������ ������

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;
		Remove(pFirst, false, true);

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetNext(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const noexcept		//���������� ��������� �� ��������� �������, �� ������� ��������� ���������� ����
	{
		//�� ����: pCurr - ��������� �� ������� ������, ������������ �������� ����� ������� � ����������, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bSerial - ���� ����������������� �������� - � ���� ������ ��
		//���������� ������� FindElement ��� ��������� ������ (������, ���� �� ���-�� ��������������� �������� �� ������ �� �������� � ��������, �� ���
		//������ ������ ������ ������� ������)
		//�� �����: ��������� �� ������� ������, ������������� ����� ����������� � ���������� �������

		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		if (!bProtected)
			LockListShared();

		ptrListElement pNext = nullptr;
		bool bPresent = true;		//���� ����������� � ������ ����������� ��������
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
		if (bPresent)
			pNext = pCurr->pNext;
		else
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
		}

		if (!bProtected)
			UnlockListShared();

		return pNext;
	}

	ptrListElement GetPrev(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept		//���������� ��������� �� ���������� �������, �� ������� ��������� ���������� ����
	{
		//�� ����: pCurr - ��������� �� ������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ���
		//��������� ������ �������� ����� �������
		//�� �����: ��������� �� �������, ������������� ����� ����������� � ���������� �������

		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (!bProtected)
			LockListShared();

		ptrListElement pPrev = FindPreviousElement(pCurr, pErrorCode, true);
		if (pPrev == nullptr && pCurr != ListBase<ListElement>::pFirst)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListShared();
			return nullptr;
		}

		if (!bProtected)
			UnlockListShared();

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return pPrev;
	}

	ptrListElement GetLast(bool bProtected = false) const noexcept
	{
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ��������� ������� ������

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	ptrListElement GetLastAndRemove(bool bProtected = false) const noexcept
	{
		//���������� ��������� ������� ������ � ������� ��� �� ������ �� ���� ��������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ �������, ��� �� ���������� ������ ������

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;
		Remove(pLast, false, true);

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	//��������

	List_OneLinked& operator=(List_OneLinked& list) noexcept
	{
		//�������� ����������� ������������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	template<class List> List_OneLinked& operator=(List& list) noexcept
	{
		//�������� ����������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	List_OneLinked& operator=(List_OneLinked&& list) noexcept
	{
		//�������� ������������� ������������
		//�� ����: list - rvalue-������ �� ���������� ������ (���������������, ��� ������ ���������)
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		LockListExclusive();		//��������� ������

		Empty(true);				//������� ������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		list.SetEmpty();

		UnlockListExclusive();		//������������ ������
		return *this;
	}

	template<class List> List_OneLinked& operator=(List&& list) noexcept
	{
		//�������� ������������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - rvalue-������ �� ���������� ������ (���������������, ��� ������ ���������)
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		LockListExclusive();		//��������� ������

		Empty(true);				//������� ������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();

		UnlockListExclusive();		//������������ ������
		return *this;
	}

	List_OneLinked& operator+=(List_OneLinked& list) noexcept
	{
		//�������� ���������� ������ � ����� �������� � ��������� ������������
		//�� ����: list - ������ �� ����������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		AddListAfter(list);		//��������� ������ � ����� ��������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	template<class List> List_OneLinked& operator+=(List& list) noexcept
	{
		//�������� ���������� ������ � ����� �������� � ��������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		AddListAfter(list);		//��������� ������ � ����� ��������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	//��������� ���������� - ������������ ������ �� ��������������� ���������� ���������� ���� ��������� �������� ������� ��������; ������ ������� �������
	//������ � ���� ��������� ��� protected

	protected:

	class ListIterator					//����� ��������� ��� �������� ���������
	{
		ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
		const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
		bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

	public:

		ListIterator() {}
		ListIterator(ptrListElement pElem, const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			ptrListElement& pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//���������� ���������: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		ListIterator operator++(int) noexcept									//����������� ���������: it++
		{
			ListIterator itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itPrev;
		}

		bool operator==(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = (pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}

		bool operator!=(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = false;
			if (pList == nullptr || li.pList == nullptr)
				bResult = !(pCurrElement == li.pCurrElement);
			else
				bResult = !(pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}
	};

	class ListIteratorConst					//����� ������������ ��������� ��� �������� ���������
	{
		ptrListElement pCurrElement{ nullptr };					//������� �������, �� ������� ��������� ��������
		const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
		bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

	public:

		ListIteratorConst() {}
		ListIteratorConst(ptrListElement pElem, const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIteratorConst(const ListIteratorConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
		ListIteratorConst(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		const ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			const ptrListElement& c_pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return c_pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator const ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//���������� ���������: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		ListIteratorConst operator++(int)			//����������� ���������: it++
		{
			ListIteratorConst itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itPrev;
		}

		bool operator==(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}

		bool operator!=(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = false;
			if (pList == nullptr || lic.pList == nullptr)
				bResult = !(pCurrElement == lic.pCurrElement);
			else
				bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}
	};

	using iterator = ListIterator;
	using const_iterator = ListIteratorConst;

	ListIterator begin(bool bProtected = false) noexcept
	{
		if(!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIterator lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return lit;
	}

	ListIterator end() noexcept
	{
		return ListIterator();
	}

	ListIteratorConst begin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return c_lit;
	}

	ListIteratorConst end() const noexcept
	{
		return ListIteratorConst();
	}

	ListIteratorConst cbegin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return c_lit;
	}

	ListIteratorConst cend() const noexcept
	{
		return ListIteratorConst();
	}

	//����������������� ������
	template<class ElementType, bool bExceptions> friend class DirectSearch;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

	//����������������� �������

	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//�������� ���������� ������� ������: _ListElement - ��� �������� ������, LockingPolicy - ��������� ���������� ������� (��. LockingPolicies.h),
//CheckingPresenceElementPolicy - ��������� �������� ������� �������� � ������

//���������� ������
template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_TwoLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, false> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, false>
{
public:

	//���������� �����
	using ListElement = _ListElement;								//��������� ���������� � ��������� ������� ��� �������� ��� ������� � ���� �� ����������� �������
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;					//��������� ��� ��������� �� ������ �������� ������ � ���������� ��� ��������� ������ � �������
	using weak_ptrListElement = typename ListElement::MemoryPolicy::weak_ptrType;		//��������� ��� ��� ������� ��������� (��������� ���������������� ����������)

	//��������, ��� ���������� ��������� �++ (Type *) ����� �������������� ������ �� ���������� �������� �������� ������� ������ (DirectSearch)
	static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
		std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>> == false),
		"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

private:

	//�������� � ���������

	bool bTemporary = false;		//���� ���������� ������, �����������, ��� ����������� ������ �������� ���������, � ������, ��� �������� � ���
									//������ ����������� ��-������� (C++ 11: ������������� ����� ������)

	static constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;		//������������ ���������� ���������, ����������� � �������� ������ ������ (��� ������������� ����������� ��������� �������� ������� ��������)

	//��������������� �������
	void SetEmpty(bool bProtected = false) noexcept
	{
		//���������� ������, ������������ ��������� �� ������ � ��������� �������� � ������� ��������; ���� �������� �� ������ �� ���������,
		//�.�. �������� ������ ��������� �������� � ������
		//������� ������������ � �������� �������� ����������� �������� ������, � ����� ��� ������ �� ���������� �������� � ����������
		//����������/����������� �������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = nullptr;
		CheckingPresenceElementPolicy<ListElement, false>::Clear();

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

public:

	//�������

	List_TwoLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ce_ullNumElementsMax)					//�����������
	{
		//�� ����: bTemporary - ���� ����, ��� ������ �������� ��� ���������

		List_TwoLinked::bTemporary = bTemporary;
	}

	List_TwoLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ullNumElementsMax)	//����������� ��� ������������� ��������� ������ ���������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������� ����� ���� ������� � ������ �� ����� ��� ������; ��� �������� ��������� � �����
		//��������� ������ ��������

		List_TwoLinked::bTemporary = false;
	}

	List_TwoLinked(const List_TwoLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, false>(list)		//����������� �����������
	{
		MakeCopy(list);				//������ ������ ����� ����������� ������
	}

	List_TwoLinked(List_TwoLinked&& list) : CheckingPresenceElementPolicy<ListElement, false>(0)				//����������� �����������: ������ ��� �������� ��������� �������� � ����������
	{
		//����������� ��������� �� �������� ���������� ������������� � ��������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		CheckingPresenceElementPolicy<ListElement, false>::AssignAnotherCES(std::move(list), this);
		list.SetEmpty();
	}

	~List_TwoLinked() noexcept					//����������
	{
		if (!bTemporary)
			Empty();							//������� ��� �������� �� ������, ���� ������ �� ���������
	}

	//��������������� �������

	static constexpr unsigned int GetLinksNumber(void) { return 2; }		//����� ������ ������

	bool FindElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ���
		//��������� ������ �������� ����� �������
		//�� �����: true - ������� ������ � ������������ � ������, false - ����� ������� �� ������ ���� ������ ����

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������

		bool bResult = CheckingPresenceElementPolicy<ListElement, false>::FindElement(pElem, static_cast<const List_TwoLinked *>(this));

		if (!bProtected)
			UnlockListShared();		//������������ ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return bResult;
	}

	inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected, ListErrorCodes::eListErrorCode* pErrorCode = nullptr) noexcept
	{
		//��������������� �������, ����������� ������ ���� ������� Delete � Remove ����������� �������� ��������������� �����
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; bDelete - ����, �����������, ������� ������� �� ������ ��� ������
		//��������� ��� �� ������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ������� (��������� ���
		//���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ���������
		//������ �������� ����� �������, pErrorCode - ��������� �� ������������ ��� ������
		//�� �����: true - �������, false - ������ ���� ���� ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			if (!bProtected)
				UnlockListExclusive();	//������������ ������
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return false;				//������ ����
		}

		//���������� ���� bSerial = true, ��������� �������� ������������ ������� �������� � ������ ����� ����������� ����, ������ �� �����
		//������ �������� ��������� �� ��������� �������
		ptrListElement pNext = GetNext(pElem, pErrorCode, true, true);
		if (pElem == ListBase<ListElement>::pFirst)		//���������� ������� �������� ������ ���������
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
			if (bDelete)
				ListElement::MemoryPolicy::Delete(pElem);
			ListBase<ListElement>::pFirst = pNext;
			if (ListBase<ListElement>::pFirst)
			{
				//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					ListBase<ListElement>::pFirst->pPrev.lock() = nullptr;
				else
					ListBase<ListElement>::pFirst->pPrev = nullptr;
			}
			if (!bProtected)
				UnlockListExclusive();		//������������ ������
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return true;
		}

		if (!(bCheckPresence == true && FindElement(pElem, pErrorCode, true) == true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();		//������������ ������
			return true;
		}

		ptrListElement pPrev = nullptr;
		//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
		if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			pPrev = pElem->pPrev.lock();
		else
			pPrev = pElem->pPrev;
		if (pPrev)
		{
			pPrev->pNext = pNext;
			if (pNext)
			{
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					pNext->pPrev.lock() = pPrev;
				else
					pNext->pPrev = pPrev;
			}
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pPrev;
			CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
			if (bDelete)
				ListElement::MemoryPolicy::Delete(pElem);
		}

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return true;
	}

	//������� ������ �� �������

	template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ����� ����������� � ���������� ����
		//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� ������� ����� ����, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� �����������
		//������������ ������������ �������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� ��
		//������ ������� ���, ������������ ������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ ��
		//�����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
		//���� ������ ����, �� �������� pElem ������������
		//�� �����: ��������� �� ��������� ������� ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pElem, pErrorCode, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� � �������� � ���
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pCurr;
			pCurr->pNext = pNext;
			//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				pCurr->pPrev.lock() = pElem;
				if (pNext)
					pNext->pPrev.lock() = pCurr;
			}
			else
			{
				pCurr->pPrev = pElem;
				if (pNext)
					pNext->pPrev = pCurr;
			}
			//���������, �������� �� ������� ������� ���������
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ����� ���������� � ���������� �����
		//�� ����: pElem - ��������� �� ������� ����, �� �������� ���������� ������� ����� ����, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� �����������
		//������������ ������������ �������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� ��
		//������ ������� ���, ������������ ������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ ��
		//�����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
		//���� ������ ����, �� �������� pElem ������������
		//�� �����: ��������� �� ��������� ������� ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			if (!bProtected)
				UnlockListExclusive();
			return nullptr;
		}

		//���������, �������� �� ��� ������ ��������� ������ (� ������, ������ �� ���� �������� ���������� �������� ��� ����)
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)//C++17: if constexpr
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� � �������� � ���
			//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				ptrListElement pPrev = pElem->pPrev.lock();
				if (bUsePreviousElementIndex)
				{
					if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
						ullElementIndexStart = pPrev->ullElementIndex;
				}
				if (pPrev)
					pPrev->pNext = pCurr;
				pCurr->pPrev.lock() = pPrev;
				pPrev = pCurr;
			}
			else
			{
				if (pElem->pPrev)
				{
					pElem->pPrev->pNext = pCurr;
					if (bUsePreviousElementIndex)
					{
						if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
							ullElementIndexStart = pElem->pPrev->ullElementIndex;
					}
				}
				pCurr->pPrev = pElem->pPrev;
				pElem->pPrev = pCurr;
			}
			pCurr->pNext = pElem;
			//���������, �������� �� ������� ������� ������
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pCurr;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddLast(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� ����� ���������� �������� ������
		//�� ����: pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������ �������� (��� ������� �� ����������
		//������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ���������� ������ �������� � ������ ����
		//��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������ ������� ����������
		//���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args -
		//��������� ��������� �����, ������������ ������������ ListElement
		//�� �����: ��������� �� ��������� ������� ������

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			//���� ��� pPrev �������� weak_ptr, �� ���������� � ���� ����� ������ ����� �������������� � shared_ptr � ������� ������� lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������ ��� ���������
			ListBase<ListElement>::pLast->pNext = pCurr;
			pCurr->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				pCurr->pPrev = ListBase<ListElement>::pLast;
			ListBase<ListElement>::pLast = pCurr;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddFirst(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, ArgTypes... Args) noexcept
	{
		//������ ����� ������� � ������ ������
		//�� ����: pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//Args - ��������� ��������� �����, ������������ ������������ ListElement
		//�� �����: ��������� �� ��������� ������� ������

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������� ������ �������
			ListElement* pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pCurr;
			pCurr->pNext = pNext;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				pCurr->pPrev.lock() = nullptr;
				pNext->pPrev.lock() = pCurr;
			}
			else
			{
				pCurr->pPrev = nullptr;
				pNext->pPrev = pCurr;
			}
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	bool InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� � ������ ����� ����������� � ���������� ����
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
		//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
		//�������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		//���� ������ ����, �� �������� pElem ������������

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pElemToInsert->pNext = pElemToInsert->pPrev.lock() = nullptr;
			else
				pElemToInsert->pNext = pElemToInsert->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� � �������� � ���
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pElemToInsert;
			pElemToInsert->pNext = pNext;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				pElemToInsert->pPrev.lock() = pElem;
				if (pNext)
					pNext->pPrev.lock() = pElemToInsert;
			}
			else
			{
				pElemToInsert->pPrev = pElem;
				if (pNext)
					pNext->pPrev = pElemToInsert;
			}
			//���������, �������� �� ������� ������� ���������
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� � ������ ����� ���������� � ���������� �����
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
		//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
		//�������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
		//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		//���� ������ ����, �� �������� pElem ������������

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ���������� ������� ������ ������
		ptrListElement pPrev = FindPreviousElement(pElem, true);
		if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//���������, �������� �� ��� ������ ��������� ������ (� ������, ������ �� ���� �������� ���������� �������� ��� ����)
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pElemToInsert->pNext = pElemToInsert->pPrev.lock() = nullptr;
			else
				pElemToInsert->pNext = pElemToInsert->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ������� �������� � �������� � ���
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				pPrev = pElem->pPrev.lock();
				if (pPrev)
				{
					pPrev->pNext = pElemToInsert;
					if (bUsePreviousElementIndex)
					{
						if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
							ullElementIndexStart = pPrev->ullElementIndex;
					}
				}
				pElemToInsert->pPrev.lock() = pPrev;
				pPrev = pElemToInsert;
			}
			else
			{
				if (pElem->pPrev)
				{
					pElem->pPrev->pNext = pElemToInsert;
					if (bUsePreviousElementIndex)
					{
						if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
							ullElementIndexStart = pElem->pPrev->ullElementIndex;
					}
				}
				pElemToInsert->pPrev = pElem->pPrev;
				pElem->pPrev = pElemToInsert;
			}
			pElemToInsert->pNext = pElem;
			//���������, �������� �� ������� ������� ������
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pElemToInsert;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertLast(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//��������� ����� ������� ����� ���������� �������� ������
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ����� ������
		//�� ����: pElemToInsert - ��������� �� ����������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
		//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
		//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
		//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
		//�� �����: true - �������, false - �������

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		unsigned long long ullElementIndexStart = 0;		//������ ��������, � �������� ����� ����������� ����� ���������� ���� � ��������� ������ ��������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pElemToInsert->pNext = pElemToInsert->pPrev.lock() = nullptr;
			else
				pElemToInsert->pNext = pElemToInsert->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������ ��� ���������
			ListBase<ListElement>::pLast->pNext = pElemToInsert;
			pElemToInsert->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pElemToInsert->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				pElemToInsert->pPrev = ListBase<ListElement>::pLast;
			ListBase<ListElement>::pLast = pElemToInsert;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this, ullElementIndexStart) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertFirst(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ����� ������� � ������ ������
		//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ ������
		//�� ����: pElemToInsert - ��������� �� ����������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - �������

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//���������, �������� �� ��� ������ ��������� ������
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pElemToInsert->pNext = pElemToInsert->pPrev.lock() = nullptr;
			else
				pElemToInsert->pNext = pElemToInsert->pPrev = nullptr;
		}
		else
		{
			//������������� ��������� � ��������� �������� ������ �� ������� ������ �������
			ptrListElement pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pElemToInsert;
			pElemToInsert->pNext = pNext;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			{
				pElemToInsert->pPrev.lock() = nullptr;
				pNext->pPrev.lock() = pElemToInsert;
			}
			else
			{
				pElemToInsert->pPrev = nullptr;
				pNext->pPrev = pElemToInsert;
			}
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this) && pErrorCode)	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool Delete(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
	{
		//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; pErrorCode - ��������� �� ������������ ��� ������, bCheckPresence - ���� ��������
		//������� �������� � ������ ������, ��� ��� ������� (��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������),
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - ������ ���� ���� ������

		//�������� ���������� ��������������� �������, ������ �� ������� ������� ������ �� ������
		return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected, pErrorCode);
	}

	bool Remove(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//��������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
	{
		//��������� ��������� ������� �� ������, ��� ������� ��������� �� ��������� �� ������
		//�� ����: pElem - ������� ������, ��������������� ��� ��������; pErrorCode - ��������� �� ������������ ��� ������, bCheckPresence - ���� ��������
		//������� �������� � ������ ������, ��� ��� ������� (��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������),
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - �������, false - ������ ���� ���� ������

		//�������� ���������� ��������������� �������, ������ �� ��������� ������� �� ������ ��� �������� �� ������
		return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected, pErrorCode);
	}

	ptrListElement Find(const ListElement& liElem, bool bProtected = false) const noexcept
	{
		//��������� ����� �������� � ���������� ��������� �� ����, ���� �� ������; ������� ������ ������ ������������ �������� ==
		//�� ����: liElem - ������ �� ������������ �������; ����������� ����� �� �����������, � �� �������� ����������� �������� � ������ (liElem � �����
		//������ �� ����������� ������), bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������� ������, ���� ������� � �������� ���������� ������, ��� nullptr � ��������� ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (*pCurr == liElem)
			{
				if (!bProtected)
					UnlockListShared();			//������������ ������ �� ������
				return pCurr;
			}
			pCurr = pCurr->pNext;
		}

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		return ptrListElement(nullptr);
	}

	unsigned long long CalculateElementsNumber(bool bProtected = false) const noexcept
	{
		//������������ ���������� ����� � ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ���������� ��������� � ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, false>::GetNumElements();			//����� ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullNumElements;
	}

	unsigned long long FindElementNumber(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//���� ���������� ����� �������� � ������
		//�� ����: pElem - ������� ������, ����� �������� ���� �����; pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� ��������
		//��� ��������� ������ �������� ����� �������
		//�� �����: ����� �������� � ������

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullElementNumber = 0;			//����� ��������
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (pCurr == pElem)
				break;
			ullElementNumber++;
			pCurr = pCurr->pNext;
		}
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return ullElementNumber;
	}

	void MakeCopy(const List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
		//�� ����: list - ������ �� ���������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ���������� ������ �� ����� ��������
		list.LockListShared();

		Empty(true);		//������� ������
		ptrListElement pCurrToCopy = list.GetFirst(true);							//�������� ������ ������� ����������� ������
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//��������� �� ������� � ���������� �������� (������� ������)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);				//������ ����� �������, ������������� ��� ��������� ����������� ������
			if (pCurr == nullptr)									//������� ��� ��������� ������ - ���������� ����������
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//������������ ���������� ������
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pPrev.lock() = pPrev;
			else
				pCurr->pPrev = pPrev;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//��������� � ���������� �������� ����������� ������
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ���������� ������
		list.UnlockListShared();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void MakeCopy(const List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ���������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ���������� ������ �� ����� ��������
		list.LockListShared();

		Empty(true);		//������� ������
		ptrListElement pCurrToCopy = list.GetFirst(true);							//�������� ������ ������� ����������� ������
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//��������� �� ������� � ���������� �������� (������� ������)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);				//������ ����� �������, ������������� ��� ��������� ����������� ������
			if (pCurr == nullptr)									//������� ��� ��������� ������ - ���������� ����������
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//������������ ���������� ������
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pPrev.lock() = pPrev;
			else
				pCurr->pPrev = pPrev;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//��������� � ���������� �������� ����������� ������
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//������������ ���������� ������
		list.UnlockListShared();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	void Empty(bool bProtected = false) noexcept
	{
		//���������� ������, ��������� ������ ��� ��� �������� �� ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		//������� ��� �������� ������
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		ptrListElement pNext = nullptr;
		while (pCurr != nullptr)
		{
			pNext = pCurr->pNext;
			ListElement::MemoryPolicy::Delete(pCurr);
			pCurr = pNext;
		}
		SetEmpty(true);					//������������� ��������� � ������� ��������

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListAfter(List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
		if (list.GetFirst(true))
		{
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				list.GetFirst(true)->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				list.GetFirst(true)->pPrev = ListBase<ListElement>::pLast;
		}
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListAfter(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
		if (list.GetFirst(true))
		{
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				list.GetFirst(true)->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				list.GetFirst(true)->pPrev = ListBase<ListElement>::pLast;
		}
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListBefore(List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
		if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			ListBase<ListElement>::pFirst->pPrev.lock() = list.GetLast(true);
		else
			ListBase<ListElement>::pFirst->pPrev = list.GetLast(true);
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListBefore(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ��� ���������
		//������, �������� ����� �������

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();
		//��������� ����������� ������ �� ����� ��������
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
		if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			ListBase<ListElement>::pFirst->pPrev.lock() = list.GetLast(true);
		else
			ListBase<ListElement>::pFirst->pPrev = list.GetLast(true);
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
		list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

		//������������ ����������� ������
		list.UnlockListExclusive();
		//������������ ������
		if (!bProtected)
			UnlockListExclusive();
	}

	List_TwoLinked Split(ptrListElement const pSplitElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//��������� ������� ������, ������� ��� ����� ���������� ��������
		//�� ����: pSplitElem - ��������� �� �������, ����� �������� ����������� ��������� ������; pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ����� ������, ���������� ���������� �����

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//��������� ������ �� ����� ��������
		if (!bProtected)
			LockListExclusive();

		if (!pSplitElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		//���������, �������� �� ���������� ������� ������ ������
		if (!FindElement(pSplitElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		List_TwoLinked list(CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax(), true);				//������ ��������� ������, � ������� ����� ��������� ���������
		//���������, �������� �� ���������� ������� ���������; ���� ���, �� ��������� ������; ���� ��, �� ������ �� ������ � ����������
		//������ ��������� list
		if (pSplitElem != ListBase<ListElement>::pLast)
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveContainer(iterator(pSplitElem->pNext, this));

			list.ListBase<ListElement>::pFirst = pSplitElem->pNext;
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				list.ListBase<ListElement>::pFirst->pPrev.lock() = nullptr;
			else
				list.ListBase<ListElement>::pFirst->pPrev = nullptr;
			pSplitElem->pNext = nullptr;
			list.ListBase<ListElement>::pLast = ListBase<ListElement>::pLast;
			ListBase<ListElement>::pLast = pSplitElem;

			bool bRegisterContainer = list.CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(&list, iterator{ list.pFirst, &list });
			if (!bRegisterContainer && pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		}

		//������������ ������
		if (!bProtected)
			UnlockListExclusive();		

		return list;
	}

	bool IsEmpty(bool bProtected = false) const noexcept
	{
		//���������, �������� �� ������ ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: true - ������ ����, false - ������ �� ����

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		bool bResult = false;
		if (ListBase<ListElement>::pFirst == nullptr && ListBase<ListElement>::pLast == nullptr)
			bResult = true;

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������

		return bResult;
	}

	unsigned long long GetNumElementsMax_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//���������� ������������ ���������� ���������, ������������� ���������� ������ ���������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ���������� ��������� � ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax();			//������������ ���������� ���������, ������������� ���������� ������ ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullNumElementsMax;
	}

	unsigned long long GetCurrentElementIndex_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//���������� ������� ������, ������������ ��� ������ ������������ �������� ������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ���������� ��������� � ������

		if (!bProtected)
			LockListShared();			//��������� ������ �� ������

		unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, false>::GetCurrentElementIndex();			//������������ ���������� ���������, ������������� ���������� ������ ���������

		if (!bProtected)
			UnlockListShared();			//������������ ������ �� ������
		return ullCurrentElementIndex;
	}

	//���������� � ������������� ������

	void LockListExclusive(void) const noexcept				//��������� ������ ��� ���������
	{
		LockingPolicy::LockExclusive();
	}

	void UnlockListExclusive(void) const noexcept			//������������ ������ ����� ���������
	{
		LockingPolicy::UnlockExclusive();
	}

	void LockListShared(void) const noexcept				//��������� ������ ��� ������
	{
		LockingPolicy::LockShared();
	}

	void UnlockListShared(void) const noexcept				//������������ ������ ����� ������
	{
		LockingPolicy::UnlockShared();
	}

	//����������� �� ����� ������

	ptrListElement GetFirst(bool bProtected = false) const noexcept
	{
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ ������� ������

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetFirstAndRemove(bool bProtected = false) const noexcept
	{
		//���������� ������ ������� ������ � ������� ��� �� ������ �� ���� ��������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ �������, ��� �� ���������� ������ ������

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;
		Remove(pFirst, false, true);

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetNext(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const	noexcept	//���������� ��������� �� ��������� �������, �� ������� ��������� ���������� ����
	{
		//�� ����: pCurr - ��������� �� ������� ������, ������������ �������� ����� ������� � ����������, pErrorCode - ��������� �� ������������ ��� ������,
		//bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bSerial - ���� ����������������� �������� - � ���� ������ ��
		//���������� ������� FindElement ��� ��������� ������ (������, ���� �� ���-�� ��������������� �������� �� ������ �� �������� � ��������, �� ���
		//������ ������ ������ ������� ������)
		//�� �����: ��������� �� ������� ������, ������������� ����� ����������� � ���������� �������

		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		if (!bProtected)
			LockListShared();

		ptrListElement pNext = nullptr;
		bool bPresent = true;		//���� ����������� � ������ ����������� ��������
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
		if (bPresent)
			pNext = pCurr->pNext;
		else
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
		}

		if (!bProtected)
			UnlockListShared();

		return pNext;
	}

	ptrListElement GetPrev(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const	noexcept	//���������� ��������� �� ���������� �������, �� ������� ��������� ���������� ����
	{
		//�� ����: pCurr - ��������� �� ������� �������, pErrorCode - ��������� �� ������������ ��� ������, bProtected - ���� ����, ��� �������� ���
		//��������� ������ �������� ����� �������
		//�� �����: ��������� �� �������, ������������� ����� ����������� � ���������� �������

		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		if (!bProtected)
			LockListShared();

		ptrListElement pPrev = nullptr;
		bool bPresent = true;		//���� ����������� � ������ ����������� ��������
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
		if (bPresent)
		{
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pPrev = pCurr->pPrev.lock();
			else
				pPrev = pCurr->pPrev;
		}
		else
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
		}

		if (!bProtected)
			UnlockListShared();

		return pPrev;
	}

	ptrListElement GetLast(bool bProtected = false) const noexcept
	{
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ��������� ������� ������

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	ptrListElement GetLastAndRemove(bool bProtected = false) const noexcept
	{
		//���������� ��������� ������� ������ � ������� ��� �� ������ �� ���� ��������
		//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
		//�� �����: ��������� �� ������ �������, ��� �� ���������� ������ ������

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;
		Remove(pLast, false, true);

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	//��������

	List_TwoLinked& operator=(List_TwoLinked& list) noexcept
	{
		//�������� ����������� ������������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	template<class List> List_TwoLinked& operator=(List& list) noexcept
	{
		//�������� ����������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ���������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	List_TwoLinked& operator=(List_TwoLinked&& list) noexcept
	{
		//�������� ������������� ������������
		//�� ����: list - rvalue-������ �� ���������� ������ (���������������, ��� ������ ���������)
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		LockListExclusive();		//��������� ������

		Empty(true);				//������� ������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		list.SetEmpty();

		UnlockListExclusive();		//������������ ������
		return *this;
	}

	template<class List> List_TwoLinked& operator=(List&& list) noexcept
	{
		//�������� ������������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - rvalue-������ �� ���������� ������ (���������������, ��� ������ ���������)
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		LockListExclusive();		//��������� ������

		Empty(true);				//������� ������� ������
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();

		UnlockListExclusive();		//������������ ������
		return *this;
	}

	List_TwoLinked& operator+=(List_TwoLinked& list) noexcept
	{
		//�������� ���������� ������ � ����� �������� � ��������� ������������
		//�� ����: list - ������ �� ����������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		AddListAfter(list);		//��������� ������ � ����� ��������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	template<class List> List_TwoLinked& operator+=(List& list) noexcept
	{
		//�������� ���������� ������ � ����� �������� � ��������� ������������
		//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
		//��������� ������ � �� ���������
		//�� ����: list - ������ �� ����������� ������
		//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

		AddListAfter(list);		//��������� ������ � ����� ��������, ���������� �� ������������: ��� ����� �������������� �����
		return *this;
	}

	//��������� ���������� - ������������ ������ �� ��������������� ���������� ���������� ���� ��������� �������� ������� ��������; ������ ������� �������
	//������ � ���� ��������� ��� protected

protected:

	class ListIterator					//����� ��������� ��� �������� ���������
	{
		ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
		const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
		bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

	public:

		ListIterator() {}
		ListIterator(ptrListElement pElem, const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			ptrListElement& pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//���������� ���������: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		void operator--()		//���������� ���������: --it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			pCurrElement = pCurrElement->pPrev;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		ListIterator operator++(int) noexcept									//����������� ���������: it++
		{
			ListIterator itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itPrev;
		}

		ListIterator operator--(int) noexcept									//����������� ���������: it--
		{
			ListIterator itCurr = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}
			pCurrElement = pCurrElement->pPrev;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itCurr;
		}

		bool operator==(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = (pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}

		bool operator!=(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = false;
			if (pList == nullptr || li.pList == nullptr)
				bResult = !(pCurrElement == li.pCurrElement);
			else
				bResult = !(pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}
	};

	class ListIteratorConst					//����� ������������ ��������� ��� �������� ���������
	{
		ptrListElement pCurrElement{ nullptr };					//������� �������, �� ������� ��������� ��������
		const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
		bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

	public:

		ListIteratorConst() {}
		ListIteratorConst(ptrListElement pElem, const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIteratorConst(const ListIteratorConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
		ListIteratorConst(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		const ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			const ptrListElement& c_pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return c_pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator const ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//���������� ���������: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		void operator--()		//���������� ���������: --it
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			if (!pList->FindElement(pCurrElement, nullptr, true))
			{
				if (!bProtected)
					pList->UnlockListShared();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			pCurrElement = pCurrElement->pPrev;

			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
		}

		ListIteratorConst operator++(int)			//����������� ���������: it++
		{
			ListIteratorConst itPrev = *this;

			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			if (!pList->FindElement(pCurrElement, nullptr, true))
			{
				if (!bProtected)
					pList->UnlockListShared();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itPrev;
		}

		ListIteratorConst operator--(int)			//����������� ���������: it--
		{
			ListIteratorConst itCurr = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//��������� ������ �� ����� ��������
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
				}
			}
			pCurrElement = pCurrElement->pPrev;
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������

			return itCurr;
		}

		bool operator==(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}

		bool operator!=(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//��������� ������ �� ����� ��������
			bool bResult = false;
			if (pList == nullptr || lic.pList == nullptr)
				bResult = !(pCurrElement == lic.pCurrElement);
			else
				bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//������������ ������ ����� ��������
			return bResult;
		}
	};

	using iterator = ListIterator;
	using const_iterator = ListIteratorConst;

	ListIterator begin(bool bProtected = false) noexcept
	{
		if(!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIterator lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return lit;
	}

	ListIterator end() noexcept
	{
		return ListIterator();
	}

	ListIteratorConst begin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return c_lit;
	}

	ListIteratorConst end() const noexcept
	{
		return ListIteratorConst();
	}

	ListIteratorConst cbegin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//��������� ������ �� ����� ��������
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//������������ ������

		return c_lit;
	}

	ListIteratorConst cend() const noexcept
	{
		return ListIteratorConst();
	}

	//����������������� ������
	template<class ElementType, bool bExceptions> friend class DirectSearch;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

	//������������������� �������

	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);
};

//�������//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2)
{
	//�������� ����������� �������: ��� ���������� ������, ���������� � ����������, �� ��������� ������, ����� ���� ������� �������� ������������ ������
	//�� ����: list1 � list2 - ������������ ������

	constexpr unsigned int ce_uiMinLinksNumber = std::min(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber(), 
		ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>::GetLinksNumber());
	constexpr unsigned int ce_uiMaxLinksNumber = std::max(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber(),
		ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>::GetLinksNumber());

	using ListTypeResultMinLinks = std::conditional_t<ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMinLinksNumber, ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>>;
	using ListTypeResultMaxLinks = std::conditional_t<ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMaxLinksNumber, ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>>;
	using ListTypeResult = std::conditional_t<ce_bGetMinLinksList, ListTypeResultMinLinks, ListTypeResultMaxLinks>;

	using CheckingPresenceElementPolicyResultMinLinks = std::conditional_t<ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMinLinksNumber, CheckingPresenceElementPolicy1<ListElement, bExceptions1>, CheckingPresenceElementPolicy2<ListElement, bExceptions2>>;
	using CheckingPresenceElementPolicyResultMaxLinks = std::conditional_t<ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMaxLinksNumber, CheckingPresenceElementPolicy1<ListElement, bExceptions1>, CheckingPresenceElementPolicy2<ListElement, bExceptions2>>;
	using CheckingPresenceElementPolicyResult = std::conditional_t<ce_bGetMinLinksList, CheckingPresenceElementPolicyResultMinLinks, CheckingPresenceElementPolicyResultMaxLinks>;

	constexpr bool bExceptionsResultMinLinks = ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMinLinksNumber ? bExceptions1 : bExceptions2;
	constexpr bool bExceptionsResultMaxLinks = ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>::GetLinksNumber() == ce_uiMaxLinksNumber ? bExceptions1 : bExceptions2;
	constexpr bool bExceptionsResult = ce_bGetMinLinksList ? bExceptionsResultMinLinks : bExceptionsResultMaxLinks;


	//��������� ������: ����� - ������������� �������� ����������; ����� ������ ������������� ��� ������ ������������ �� ������ ��������� �������� ���� std::lock(mutex1, mutex2)
	list1.LockListExclusive();
	list2.LockListExclusive();

	//������ ��������� ������, � ������� ����� ��������� ���������
	ListTypeResult list(list1.GetNumElementsMax_SearchElementPolicy(true) + list2.GetNumElementsMax_SearchElementPolicy(true), false);

	//������������� ���������, ������������ ������ (���� ��� ��� �� ��� ����� ��������� �������, ������� ����� ��������)
	if (list1.GetFirst(true))
		list.ListBase<ListElement>::pFirst = list1.GetFirst(true);
	else
		list.ListBase<ListElement>::pFirst = list2.GetFirst(true);
	if (list1.GetLast(true))
		list1.GetLast(true)->pNext = list2.GetFirst(true);
	if (list2.GetLast(true))
		list.ListBase<ListElement>::pLast = list2.GetLast(true);
	else
		list.ListBase<ListElement>::pLast = list1.GetLast(true);

	//������������ �������� ������ � ��������������� ��������� ������ ��������
	list.CheckingPresenceElementPolicyResult::RegisterContainer(&list, typename ListTypeResult::iterator{ list.ListBase<ListElement>::pFirst, &list });

	//������� ������������ ������, ��������� ���������
	list1.SetEmpty(true);
	list2.SetEmpty(true);

	//������������ ������
	list2.UnlockListExclusive();
	list1.UnlockListExclusive();

	return list;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy = SmartSharedPointer, class LockingPolicy = ThreadLocking_STDMutex,
	template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_OneLinked_DataAdapter;

template<class ElementData, template<class> class _MemoryPolicy = SmartSharedPointer, class LockingPolicy = ThreadLocking_STDMutex,
	template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_TwoLinked_DataAdapter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData,	template<class> class _MemoryPolicy, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy>
	class List_OneLinked_DataAdapter<ElementData, _MemoryPolicy, LockingPolicy, CheckingPresenceElementPolicy, false> : 
	public List_OneLinked<std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_OneLinked<ElementData, _MemoryPolicy>, ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>>, LockingPolicy, CheckingPresenceElementPolicy, false>
{
public:

	//�������� ��� �������� ListElementCompound: ���� ��������� ������ �������� = DirectSearch, �� ��������� ListElementCompound_OneLinked, ��� ���� ���������
	//��������� - ListElementCompound_OneLinked_CP
	//��������� � ������������ � ��������� CheckingPresenceElementPolicy, � DirectSearch ����� �������������� ���-�� ���������� ��� ���������� ���������,
	//�� ��������� ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy> � �������� ��������� ��������� ������ ��������
	using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_OneLinked<ElementData, _MemoryPolicy>, ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>>;
	using List = List_OneLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, false>;

	//�������

	List_OneLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//�����������

	List_OneLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //����������� ��� ������������� ��������� ������ ���������

	List_OneLinked_DataAdapter(const List_OneLinked_DataAdapter& list) : List(list) {}	//����������� �����������

	List_OneLinked_DataAdapter(List_OneLinked_DataAdapter&& list) : List(list) {}		//����������� �����������: ������ ��� �������� ��������� �������� � ����������
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy>
	class List_TwoLinked_DataAdapter<ElementData, _MemoryPolicy, LockingPolicy, CheckingPresenceElementPolicy, false> : 
	public List_TwoLinked<std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>, LockingPolicy, CheckingPresenceElementPolicy, false>
{
public:

	//�������� ��� �������� ListElementCompound: ���� ��������� ������ �������� = DirectSearch, �� ��������� ListElementCompound_TwoLinked, ��� ���� ���������
	//��������� - ListElementCompound_TwoLinked_CP
	//��������� � ������������ � ��������� CheckingPresenceElementPolicy, � DirectSearch ����� �������������� ���-�� ���������� ��� ���������� ���������,
	//�� ��������� ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy> � �������� ��������� ��������� ������ ��������
	using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>;
	using List = List_TwoLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, false>;

	//�������

	List_TwoLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//�����������

	List_TwoLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //����������� ��� ������������� ��������� ������ ���������

	List_TwoLinked_DataAdapter(const List_TwoLinked_DataAdapter& list) : List(list) {}	//����������� �����������

	List_TwoLinked_DataAdapter(List_TwoLinked_DataAdapter&& list) : List(list) {}		//����������� �����������: ������ ��� �������� ��������� �������� � ����������
};

}		//����� ����������� ������������ ��� ListMT