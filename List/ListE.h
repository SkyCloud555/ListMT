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

//Main ListMT header: list classes definitions WITH exception support.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <atomic>						//��������� ��������
#include "List.h"						//�������� ������������ ���� �������������� ������

namespace ListMT						//������������ ��� �������������� ������ (������������ � �����������) ��� ����������� ����������
{

	//���������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//������ �������

//�������� ���������� ������� ������: _ListElement - ��� �������� ������, LockingPolicy - ��������� ���������� ������� (��. LockingPolicies.h),
//CheckingPresenceElementPolicy - ��������� �������� ������� �������� � ������

//����������� ������
	template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_OneLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, true> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, true>
	{	
	public:

		//���������� �����
		using ListElement = _ListElement;								//��������� ���������� � ��������� ������� ��� �������� ��� ������� � ���� �� ����������� �������
		using ptrListElement = typename ListElement::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ������ � ���������� ��� ��������� ������ � �������		

		//��������, ��� ���������� ��������� �++ (Type *) ����� �������������� ������ �� ���������� �������� �������� ������� ������ (DirectSearch)
		static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
			std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>> == false),
			"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

	private:
		//�������� � ���������

		bool bTemporary = false;	//���� ���������� ������, �����������, ��� ����������� ������ �������� ���������, � ������, ��� �������� � ���
									//������ ����������� ��-������� (C++ 11: ������������� ����� ������)

		static constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;		//������������ ���������� ���������, ����������� � �������� ������ ������ (��� ������������� ����������� ��������� �������� ������� ��������)		

		//��������������� �������
		ptrListElement FindPreviousElement(ptrListElement const pElem, bool bProtected = false) const
		{
			//��������� ����� ��������, ������� ��������� �� ������������ �������
			//�� ����: pElem - ��������� �� ������������ �������, bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������
			//�� �����: ��������� �� ���������� ������� ������������ �������� ���� nullptr, ���� ����� ������� �� ������ ���� ������ ����

			if (!pElem)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)
			if (!bProtected)
				LockListShared();		//��������� ������ �� ����� ��������
			ptrListElement pCurr = ListBase<ListElement>::pFirst;
			if (pElem == ListBase<ListElement>::pFirst)
			{
				if (!bProtected)
					UnlockListShared();		//������������ ������
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
			CheckingPresenceElementPolicy<ListElement, true>::Clear();

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected)
		{
			//��������������� �������, ����������� ������ ���� ������� Delete � Remove ����������� �������� ��������������� �����
			//�� ����: pElem - ������� ������, ��������������� ��� ��������; bDelete - ����, �����������, ������� ������� �� ������ ��� ������
			//��������� ��� �� ������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ������� (��������� ���
			//���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ���������
			//������ �������� ����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			if (!pElem)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			if (ListBase<ListElement>::pFirst == nullptr)
			{
				if (!bProtected)
					UnlockListExclusive();	//������������ ������
				return false;			//������ ����
			}

			try
			{
				//���������� ���� bSerial = true, ��������� �������� ������������ ������� �������� � ������ ����� ����������� ����, ������ �� �����
				//������ �������� ��������� �� ��������� �������
				ptrListElement pNext = GetNext(pElem, true, true);
				if (pElem == ListBase<ListElement>::pFirst)		//���������� ������� �������� ������ ���������
				{
					CheckingPresenceElementPolicy<ListElement, true>::RemoveElement(pElem->ullElementIndex);
					if (bDelete)
						ListElement::MemoryPolicy::Delete(pElem);
					ListBase<ListElement>::pFirst = pNext;
					if (!bProtected)
						UnlockListExclusive();		//������������ ������
					return true;
				}

				ptrListElement pPrev = FindPreviousElement(pElem, true);		//���� �������, ����������� �� ����������
				if (pPrev)
				{
					pPrev->pNext = pNext;
					if (bCheckPresence == true && pPrev == pElem)
					{
						if (pElem == ListBase<ListElement>::pLast)
							ListBase<ListElement>::pLast = pPrev;
						CheckingPresenceElementPolicy<ListElement, true>::RemoveElement(pElem->ullElementIndex);
						if (bDelete)
							ListElement::MemoryPolicy::Delete(pElem);
					}
				}
			}
			catch (typename ListErrors::NotPartOfList)			//���������� �� ���������� ��������� �������� � ������ - ������� ������ �� ����, �������
			{
				//������������ ������
				if (!bProtected)
					UnlockListExclusive();
				return true;
			}

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return true;
		}

	protected:

		mutable std::atomic<bool> bLocked = false;								//���� ����, ��� ������ ������������

	public:

		//�������

		List_OneLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, true>(ce_ullNumElementsMax)		//�����������
		{
			//�� ����: bTemporary - ���� ����, ��� ������ �������� ��� ���������; ���� ���� ��������� � ����� ��������� ���������� ������

			List_OneLinked::bTemporary = bTemporary;
		}

		List_OneLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, true>(ullNumElementsMax)	//����������� ��� ������������� ��������� ������ ���������
		{
			//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������� ����� ���� ������� � ������ �� ����� ��� ������; ��� �������� ��������� � �����
			//��������� ������ ��������

			List_OneLinked::bTemporary = false;
		}

		List_OneLinked(const List_OneLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, true>(list)	//����������� �����������
		{
			MakeCopy(list);				//������ ������ ����� ����������� ������
		}

		List_OneLinked(List_OneLinked&& list) : CheckingPresenceElementPolicy<ListElement, true>(0)		//����������� �����������: ������ ��� �������� ��������� �������� � ����������
		{
			//����������� ��������� �� �������� ���������� ������������� � ��������� ������
			ListBase<ListElement>::pFirst = list.GetFirst();
			ListBase<ListElement>::pLast = list.GetLast();
			CheckingPresenceElementPolicy<ListElement, true>::AssignAnotherCES(std::move(list), this);
			list.SetEmpty();
		}

		~List_OneLinked() noexcept					//����������
		{
			if (!bTemporary)
				Empty();							//������� ��� �������� �� ������, ���� ������ �� ���������
		}

		//��������������� �������

		static constexpr unsigned int GetLinksNumber(void) { return 1; }		//����� ������ ������

		bool FindElement(ptrListElement const pElem, bool bProtected = false) const
		{
			//��������� ����� ��������, �� ������� ��������� ���������� ��������
			//�� ����: pElem - ��������� �� ������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������
			//�������� ����� �������
			//�� �����: true - ������� ������ � ������������ � ������, false - ����� ������� �� ������ ���� ������ ����

			if (!pElem)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();		//��������� ������ �� ����� ��������

			bool bResult = CheckingPresenceElementPolicy<ListElement, true>::FindElement(pElem, static_cast<const List_OneLinked *>(this));

			if (!bProtected)
				UnlockListShared();		//������������ ������
			return bResult;
		}

		//������� ������ �� �������

		template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ����� ����������� � ���������� ����
			//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� ������� ����� ����, bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������);
			//Args - ��������� ��������� �����, ������������ ������������ ListElement
			//���� ������ ����, �� �������� pElem ������������
			//�� �����: ��������� �� ��������� ������� ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
					ullElementIndexStart = pElem->ullElementIndex;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ����� ���������� � ���������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� ������� ����� ����, bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
			//���� ������ ����, �� �������� pElem ������������
			//�� �����: ��������� �� ��������� ������� ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			ptrListElement pPrev = FindPreviousElement(pElem, true);
			if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
				{
					if (pPrev)
						ullElementIndexStart = pPrev->ullElementIndex;
				}
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddLast(bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� ����� ���������� �������� ������
			//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ����
			//������������� ������� ����������� ������������ ������������ �������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/
			//SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ��� � ��������������� �������
			//�������; � ������ ������ �� ��������� �� ������ ������� ���, ������������ ������� ���������� ���������� �������� - ��� ���������������� �������� ������
			//(��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
			//�� �����: ��������� �� ��������� ������� ������

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������

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
					if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
						ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
				}
				ListBase<ListElement>::pLast = pCurr;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddFirst(bool bProtected = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ������
			//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//Args - ��������� ��������� �����, ������������ ������������ ListElement
			//�� �����: ��������� �� ��������� ������� ������

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		void InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� � ������ ����� ����������� � ���������� ����
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
			//�������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)

			//���� ������ ����, �� �������� pElem ������������

			if (pElem == nullptr || pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
					ullElementIndexStart = pElem->ullElementIndex;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElem, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� � ������ ����� ���������� � ���������� �����
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
			//�������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)

			//���� ������ ����, �� �������� pElem ������������

			if (pElem == nullptr || pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			ptrListElement pPrev = FindPreviousElement(pElem, true);
			if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
					ullElementIndexStart = pPrev->ullElementIndex;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElem, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertLast(ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� ����� ���������� �������� ������
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ����� ������
			//�� ����: pElemToInsert - ��������� �� ����������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����,
			//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
			//������� 

			if (pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
					if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
						ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
				}
				ListBase<ListElement>::pLast = pElemToInsert;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElemToInsert, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertFirst(ptrListElement const pElemToInsert, bool bProtected = false)
		{
			//��������� ����� ������� � ������ ������
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ ������
			//�� ����: pElemToInsert - ��������� �� ����������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����
			//������� 

			if (pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElemToInsert, this);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		bool Delete(ptrListElement const pElem, bool bCheckPresence = false, bool bProtected = false)	//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		{
			//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
			//�� ����: pElem - ������� ������, ��������������� ��� ��������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� �������
			//(��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ��������� ������ ��������
			//����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			//�������� ���������� ��������������� �������, ������ �� ������� ������� ������ �� ������
			return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected);
		}

		bool Remove(ptrListElement const pElem, bool bCheckPresence = false, bool bProtected = false)	//��������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		{
			//��������� ��������� ������� �� ������, ��� ������� ��������� �� ��������� �� ������
			//�� ����: pElem - ������� ������, ��������������� ��� ����������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ���������
			//(��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ��������� ������ ��������
			//����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			//�������� ���������� ��������������� �������, ������ �� ��������� ������� �� ������ ��� �������� �� ������
			return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected);
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

			unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, true>::GetNumElements();			//����� ���������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ullNumElements;
		}

		unsigned long long FindElementNumber(ptrListElement const pElem, bool bProtected = false) const
		{
			//���� ���������� ����� �������� � ������
			//�� ����: pElem - ������� ������, ����� �������� ���� �����; bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����
			//������� 
			//�� �����: ����� �������� � ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
				throw ListErrors::NotPartOfList();		//���������� ������� �� �������� ������ ������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������

			return ullElementNumber;
		}

		void MakeCopy(const List_OneLinked& list, bool bProtected = false)
		{
			//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
			//�� ����: list - ������ �� ���������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
					if (!bProtected)
						UnlockListExclusive();
					//������������ ���������� ������
					list.UnlockListShared();
					throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });

			//������������ ���������� ������
			list.UnlockListShared();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void MakeCopy(const List& list, bool bProtected = false)
		{
			//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ���������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
					if (!bProtected)
						UnlockListExclusive();
					//������������ ���������� ������
					list.UnlockListShared();
					throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });

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

		void AddListAfter(List_OneLinked& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			//��������� ����������� ������ �� ����� ��������
			list.LockListExclusive();

			ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
			if (list.GetLast(true))
				ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)				
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void AddListAfter(List& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			//��������� ����������� ������ �� ����� ��������
			list.LockListExclusive();

			ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//���������� ������� ������, ������ �� ������ ������������
			if (list.GetLast(true))
				ListBase<ListElement>::pLast = list.GetLast(true);				//������������� ��������� ������� �������� ������ (���� ����������� ������ �� ����)				
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void AddListBefore(List_OneLinked& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			//��������� ����������� ������ �� ����� ��������
			list.LockListExclusive();

			if (list.GetLast(true))
				list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
			if (list.GetFirst(true))
				ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void AddListBefore(List& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			//��������� ����������� ������ �� ����� ��������
			list.LockListExclusive();

			if (list.GetLast(true))
				list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//��������� ������� ������������ ������ ��������� �� ������ ��������
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
			if (list.GetFirst(true))
				ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		List_OneLinked Split(ptrListElement const pSplitElem, bool bProtected = false)
		{
			//��������� ������� ������, ������� ��� ����� ���������� ��������
			//�� ����: pSplitElem - ��������� �� �������, ����� �������� ����������� ��������� ������; bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������
			//�� �����: ����� ������, ���������� ���������� �����

			if (!pSplitElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pSplitElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			List_OneLinked list(CheckingPresenceElementPolicy<ListElement, true>::GetNumElementsMax(), true);				//������ ��������� ������, � ������� ����� ��������� ���������
			//���������, �������� �� ���������� ������� ���������; ���� ���, �� ��������� ������; ���� ��, �� ������ �� ������ � ����������
			//������ ��������� list
			if (pSplitElem != ListBase<ListElement>::pLast)
			{
				CheckingPresenceElementPolicy<ListElement, true>::RemoveContainer(iterator(pSplitElem->pNext, this));

				list.pFirst = pSplitElem->pNext;
				pSplitElem->pNext = nullptr;
				list.pLast = ListBase<ListElement>::pLast;
				ListBase<ListElement>::pLast = pSplitElem;

				list.CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(&list, List_OneLinked::iterator{ list.pFirst, &list });
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

			unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, true>::GetNumElementsMax();			//������������ ���������� ���������, ������������� ���������� ������ ���������

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

			unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, true>::GetCurrentElementIndex();			//������������ ���������� ���������, ������������� ���������� ������ ���������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ullCurrentElementIndex;
		}

		//���������� � ������������� ������

		void LockListExclusive(void) const noexcept				//��������� ������ ��� ���������
		{
			LockingPolicy::LockExclusive();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(true);
		}

		void UnlockListExclusive(void) const noexcept			//������������ ������ ����� ���������
		{
			LockingPolicy::UnlockExclusive();					//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(false);
		}

		void LockListShared(void) const noexcept				//��������� ������ ��� ������
		{
			LockingPolicy::LockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(true);
		}

		void UnlockListShared(void) const noexcept				//������������ ������ ����� ������
		{
			LockingPolicy::UnlockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(false);
		}

		bool GetLockStatus(void) const noexcept
		{
			return bLocked.load();
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

		ptrListElement GetNext(ptrListElement const pCurr, bool bProtected = false, bool bSerial = false) const		//���������� ��������� �� ��������� �������, �� ������� ��������� ���������� ����
		{
			//�� ����: pCurr - ��������� �� ������� ������, ������������ �������� ����� ������� � ����������, bProtected - ���� ����, ��� ��������
			//��� ��������� ������ �������� ����� �������, bSerial - ���� ����������������� �������� - � ���� ������ �� ����������
			//������� FindElement ��� ��������� ������ (������, ���� �� ���-�� ��������������� �������� �� ������ �� �������� � ��������, �� ���
			//������ ������ ������ ������� ������)
			//�� �����: ��������� �� ������� ������, ������������� ����� ����������� � ���������� �������

			if (!pCurr)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();

			ptrListElement pNext = nullptr;
			bool bPresent = true;		//���� ����������� � ������ ����������� ��������
			if (bProtected == false || bSerial == false)
				bPresent = FindElement(pCurr, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
			if (bPresent)
				pNext = pCurr->pNext;
			else
			{
				if (!bProtected)
					UnlockListShared();
				throw ListErrors::NotPartOfList();			//���������� ���������� �� ���������� �������� � ������
			}

			if (!bProtected)
				UnlockListShared();

			return pNext;
		}

		ptrListElement GetPrev(ptrListElement const pCurr, bool bProtected = false) const		//���������� ��������� �� ���������� �������, �� ������� ��������� ���������� ����
		{
			//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������
			//�� �����: ��������� �� �������, ������������� ����� ����������� � ���������� �������

			if (!pCurr)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();

			ptrListElement pPrev = FindPreviousElement(pCurr, true);
			if (pPrev == nullptr && pCurr != ListBase<ListElement>::pFirst)
			{
				if (!bProtected)
					UnlockListShared();
				throw ListErrors::NotPartOfList();			//���������� ���������� �� ���������� �������� � ������
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

		List_OneLinked& operator=(List_OneLinked& list)
		{
			//�������� ����������� ������������
			//�� ����: list - ������ �� ���������� ������
			//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

			MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
			return *this;
		}

		template<class List> List_OneLinked& operator=(List& list)
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

		//��������� ����������

	protected:

		class ListIterator					//����� ��������� ��� �������� ���������
		{
			ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
			const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
			bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

		public:

			ListIterator() {}
			ListIterator(ptrListElement pElem, const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList, bool bProtected = true)
				noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
			ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			ptrListElement& operator*()
			{
				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
			const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
			bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

		public:

			ListIteratorConst() {}
			ListIteratorConst(ptrListElement pElem, const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList, bool bProtected = true)
				noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
			ListIteratorConst(const ListIteratorConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
			ListIteratorConst(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			const ptrListElement& operator*()
			{
				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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

	public:

		using iterator = ListIterator;
		using const_iterator = ListIteratorConst;

		ListIterator begin(bool bProtected = false) noexcept
		{
			if(!bProtected)
				LockListShared();		//��������� ������ �� ����� ��������
			ListIterator lit(ListBase<ListElement>::pFirst, this);
			if(!bProtected)
				UnlockListShared();		//������������ ������

			return lit;
		}

		ListIterator end() noexcept
		{
			return ListIterator();
		}

		ListIteratorConst begin(bool bProtected = false) const noexcept
		{
			if(!bProtected)
				LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
			if(!bProtected)
				UnlockListShared();		//������������ ������

			return c_lit;
		}

		ListIteratorConst end() const noexcept
		{
			return ListIteratorConst();
		}

		ListIteratorConst cbegin(bool bProtected = false) const noexcept
		{
			if(!bProtected)
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
	template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_TwoLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, true> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, true>
	{
	public:

		//���������� �����
		using ListElement = _ListElement;								//��������� ���������� � ��������� ������� ��� �������� ��� ������� � ���� �� ����������� �������
		using ptrListElement = typename ListElement::MemoryPolicy::ptrType;					//��������� ��� ��������� �� ������ �������� ������ � ���������� ��� ��������� ������ � �������
		using weak_ptrListElement = typename ListElement::MemoryPolicy::weak_ptrType;		//��������� ��� ��� ������� ��������� (��������� ���������������� ����������)		

		//��������, ��� ���������� ��������� �++ (Type *) ����� �������������� ������ �� ���������� �������� �������� ������� ������ (DirectSearch)
		static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
			std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>> == false),
			"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

	private:

		//��������

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
			CheckingPresenceElementPolicy<ListElement, true>::Clear();

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

	protected:

		mutable std::atomic<bool> bLocked = false;								//���� ����, ��� ������ ������������

	public:

		//�������

		List_TwoLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, true>(ce_ullNumElementsMax)					//�����������
		{
			//�� ����: bTemporary - ���� ����, ��� ������ �������� ��� ���������

			List_TwoLinked::bTemporary = bTemporary;
		}

		List_TwoLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, true>(ullNumElementsMax)	//����������� ��� ������������� ��������� ������ ���������
		{
			//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������� ����� ���� ������� � ������ �� ����� ��� ������; ��� �������� ��������� � �����
			//��������� ������ ��������

			List_TwoLinked::bTemporary = false;
		}

		List_TwoLinked(const List_TwoLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, true>(list)		//����������� �����������
		{
			MakeCopy(list);				//������ ������ ����� ����������� ������
		}

		List_TwoLinked(List_TwoLinked&& list) : CheckingPresenceElementPolicy<ListElement, true>(0)	//����������� �����������: ������ ��� �������� ��������� �������� � ����������
		{
			//����������� ��������� �� �������� ���������� ������������� � ��������� ������
			ListBase<ListElement>::pFirst = list.GetFirst();
			ListBase<ListElement>::pLast = list.GetLast();
			CheckingPresenceElementPolicy<ListElement, true>::AssignAnotherCES(std::move(list), this);
			list.SetEmpty();
		}

		~List_TwoLinked() noexcept					//����������
		{
			if (!bTemporary)
				Empty();							//������� ��� �������� �� ������, ���� ������ �� ���������
		}

		//��������������� �������

		static constexpr unsigned int GetLinksNumber(void) { return 2; }		//����� ������ ������

		bool FindElement(ptrListElement const pElem, bool bProtected = false) const
		{
			//��������� ����� ��������, �� ������� ��������� ���������� ��������
			//�� ����: pElem - ��������� �� ������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������
			//�������� ����� �������
			//�� �����: true - ������� ������ � ������������ � ������, false - ����� ������� �� ������ ���� ������ ����

			if (!pElem)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();		//��������� ������ �� ����� ��������

			bool bResult = CheckingPresenceElementPolicy<ListElement, true>::FindElement(pElem, static_cast<const List_TwoLinked *>(this));

			if (!bProtected)
				UnlockListShared();		//������������ ������
			return bResult;
		}

		inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected)
		{
			//��������������� �������, ����������� ������ ���� ������� Delete � Remove ����������� �������� ��������������� �����
			//�� ����: pElem - ������� ������, ��������������� ��� ��������; bDelete - ����, �����������, ������� ������� �� ������ ��� ������
			//��������� ��� �� ������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ������� (��������� ���
			//���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ���������
			//������ �������� ����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			if (!pElem)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();
			if (ListBase<ListElement>::pFirst == nullptr)
			{
				if (!bProtected)
					UnlockListExclusive();	//������������ ������
				return false;				//������ ����
			}

			try
			{
				if (!(bCheckPresence == true && FindElement(pElem, true) == true))
					throw ListErrors::NotPartOfList();

				ptrListElement pNext = GetNext(pElem, true, true);
				if (pElem == ListBase<ListElement>::pFirst)		//���������� ������� �������� ������ ���������
				{
					CheckingPresenceElementPolicy<ListElement, true>::RemoveElement(pElem->ullElementIndex);
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
					CheckingPresenceElementPolicy<ListElement, true>::RemoveElement(pElem->ullElementIndex);
					if (bDelete)
						ListElement::MemoryPolicy::Delete(pElem);
				}
			}
			catch (typename ListErrors::NotPartOfList)			//���������� �� ���������� ��������� �������� � ������ - ������� ������ �� ����, �������
			{
				//������������ ������
				if (!bProtected)
					UnlockListExclusive();
				return true;
			}

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return true;
		}

		//������� ������ �� �������

		template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ����� ����������� � ���������� ����
			//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� ������� ����� ����, bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������);
			//Args - ��������� ��������� �����, ������������ ������������ ListElement
			//���� ������ ����, �� �������� pElem ������������
			//�� �����: ��������� �� ��������� ������� ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
					ullElementIndexStart = pElem->ullElementIndex;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ����� ���������� � ���������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� ������� ����� ����, bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������, bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������);
			//Args - ��������� ��������� �����, ������������ ������������ ListElement
			//���� ������ ����, �� �������� pElem ������������
			//�� �����: ��������� �� ��������� ������� ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
						if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
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
							if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddLast(bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args)
		{
			//������ ����� ������� ����� ���������� �������� ������
			//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������, bUsePreviousElementIndex - ����
			//������������� ������� ����������� ������������ ������������ �������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/
			//SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ���������� ������ �������� � ������ ���� ��������� (�������) ��� � ��������������� �������
			//�������; � ������ ������ �� ��������� �� ������ ������� ���, ������������ ������� ���������� ���������� �������� - ��� ���������������� �������� ������
			//(��������, ��� �������� ������ �� �����) ��� ����������� �������� ������); Args - ��������� ��������� �����, ������������ ������������ ListElement
			//�� �����: ��������� �� ��������� ������� ������

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������

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
					if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
						ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
				}
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					pCurr->pPrev.lock() = ListBase<ListElement>::pLast;
				else
					pCurr->pPrev = ListBase<ListElement>::pLast;
				ListBase<ListElement>::pLast = pCurr;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		template<class... ArgTypes> ptrListElement AddFirst(bool bProtected = false, ArgTypes... Args)
		{
			//������ ����� ������� � ������ ������
			//�� ����: bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//Args - ��������� ��������� �����, ������������ ������������ ListElement
			//�� �����: ��������� �� ��������� ������� ������

			ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
			if (!pCurr)
				throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pCurr, this);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();

			return pCurr;
		}

		void InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� � ������ ����� ����������� � ���������� ����
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� �������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
			//�������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)

			//���� ������ ����, �� �������� pElem ������������

			if (pElem == nullptr || pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
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
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
					ullElementIndexStart = pElem->ullElementIndex;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElem, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� � ������ ����� ���������� � ���������� �����
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ � �������� �����
			//�� ����: pElem - ��������� �� ������� ����, ����� ������� ���������� �������� ����� ����, pElemToInsert - ��������� �� �����������
			//�������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������,
			//bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)
			//���� ������ ����, �� �������� pElem ������������

			if (pElem == nullptr || pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			ptrListElement pPrev = FindPreviousElement(pElem, true);
			if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
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
							if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
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
							if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElem, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertLast(ptrListElement const pElemToInsert, bool bProtected = false, bool bUsePreviousElementIndex = false)
		{
			//��������� ����� ������� ����� ���������� �������� ������
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ����� ������
			//�� ����: pElemToInsert - ��������� �� ����������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����
			//������� , bUsePreviousElementIndex - ���� ������������� ������� ����������� ������������ ������������
			//�������� (��� ������� �� ���������� ������ �������� SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, ������� ��� ����������
			//������ �������� � ������ ���� ��������� (�������) ��� � ��������������� ������� �������; � ������ ������ �� ��������� �� ������ ������� ���, ������������
			//������� ���������� ���������� �������� - ��� ���������������� �������� ������ (��������, ��� �������� ������ �� �����) ��� ����������� �������� ������)

			if (pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
					if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, true>, DirectSearch<ListElement, true>>)
						ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
				}
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					pElemToInsert->pPrev.lock() = ListBase<ListElement>::pLast;
				else
					pElemToInsert->pPrev = ListBase<ListElement>::pLast;
				ListBase<ListElement>::pLast = pElemToInsert;
			}
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElemToInsert, this, ullElementIndexStart);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void InsertFirst(ptrListElement const pElemToInsert, bool bProtected = false)
		{
			//��������� ����� ������� � ������ ������
			//����������� ������� ��� ���� �������� �����, � ������ ������� - �������� ��� � ������ ������
			//�� ����: pElemToInsert - ��������� �� ����������� �������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����
			//������� 

			if (pElemToInsert == nullptr)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterElement(pElemToInsert, this);	//������������ ������� � ��������� ������ �������� ��� ���������� ��� ������ � ����������

			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		bool Delete(ptrListElement const pElem, bool bCheckPresence = false, bool bProtected = false)	//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		{
			//������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
			//�� ����: pElem - ������� ������, ��������������� ��� ��������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� �������
			//(��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ��������� ������ ��������
			//����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			//�������� ���������� ��������������� �������, ������ �� ������� ������� ������ �� ������
			return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected);
		}

		bool Remove(ptrListElement const pElem, bool bCheckPresence = false, bool bProtected = false)	//��������� ���������� ������� ������, �������� �������� ��������� ����������� ��������
		{
			//��������� ��������� ������� �� ������, ��� ������� ��������� �� ��������� �� ������
			//�� ����: pElem - ������� ������, ��������������� ��� ����������; bCheckPresence - ���� �������� ������� �������� � ������ ������, ��� ��� ���������
			//(��������� ��� ���������������, ����� �� ������� �������� �������, �������� ������ �������), bProtected - ���� ����, ��� �������� ��� ��������� ������ ��������
			//����� �������
			//�� �����: true - �������, false - ������ ���� ���� ������

			//�������� ���������� ��������������� �������, ������ �� ��������� ������� �� ������ ��� �������� �� ������
			return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected);
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

			unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, true>::GetNumElements();			//����� ���������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ullNumElements;
		}

		unsigned long long FindElementNumber(ptrListElement const pElem, bool bProtected = false) const
		{
			//���� ���������� ����� �������� � ������
			//�� ����: pElem - ������� ������, ����� �������� ���� �����; bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� �����
			//������� 
			//�� �����: ����� �������� � ������

			if (!pElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

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
				throw ListErrors::NotPartOfList();		//���������� ������� �� �������� ������ ������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������

			return ullElementNumber;
		}

		void MakeCopy(const List_TwoLinked& list, bool bProtected = false)
		{
			//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
			//�� ����: list - ������ �� ���������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
					if (!bProtected)
						UnlockListExclusive();
					//������������ ���������� ������
					list.UnlockListShared();
					throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });

			//������������ ���������� ������
			list.UnlockListShared();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void MakeCopy(const List& list, bool bProtected = false)
		{
			//��������� ��������� �������� ������ ����������, �.�. ���������� ������� ������ � ������ ����� �����������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ���������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
					if (!bProtected)
						UnlockListExclusive();
					//������������ ���������� ������
					list.UnlockListShared();
					throw ListErrors::FailElemCreation();		//��������� ���������� � ������� ��������� ������ ��� ���� ������
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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });

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

		void AddListAfter(List_TwoLinked& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void AddListAfter(List& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ����� ��������, ����� ���� ���������� (�������) ��������� ������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������, �������� ����� �������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		void AddListBefore(List_TwoLinked& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
			if (list.GetFirst(true))
				ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		template<class List> void AddListBefore(List& list, bool bProtected = false) noexcept
		{
			//��������� �������� ���������� ������ � ������ ��������, ����� ���� ���������� (�������) ��������� ������
			//��������� ������� - ����� ���� ������� ������ � ������� ����������� ���������� � �������� ������� ��������: �������, ����� ����� �������� �
			//��������� ������ � �� ���������
			//�� ����: list - ������ �� ����������� ������, bProtected - ���� ����, ��� �������� ��� ��������� ������ �������� ����� �������

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
			CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
			if (list.GetFirst(true))
				ListBase<ListElement>::pFirst = list.GetFirst(true);				//������������� ������ ������� �������� ������ (���� ����������� ������ �� ����)
			list.SetEmpty(true);						//������� ����������� ������, ��������� ��� ��������� � ������� ��������

			//������������ ����������� ������
			list.UnlockListExclusive();
			//������������ ������
			if (!bProtected)
				UnlockListExclusive();
		}

		List_TwoLinked Split(ptrListElement const pSplitElem, bool bProtected = false)
		{
			//��������� ������� ������, ������� ��� ����� ���������� ��������
			//�� ����: pSplitElem - ��������� �� �������, ����� �������� ����������� ��������� ������; bProtected - ���� ����, ��� �������� ���
			//��������� ������ �������� ����� �������
			//�� �����: ����� ������, ���������� ���������� �����

			if (!pSplitElem)
				throw ListErrors::Nullptr();			//��������� ���������� �� ��������� ��������� (������� ���������)

			//��������� ������ �� ����� ��������
			if (!bProtected)
				LockListExclusive();

			//���������, �������� �� ���������� ������� ������ ������
			if (!FindElement(pSplitElem, true))
			{
				if (!bProtected)
					UnlockListExclusive();
				throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
			}

			List_TwoLinked list(CheckingPresenceElementPolicy<ListElement, true>::GetNumElementsMax(), true);				//������ ��������� ������, � ������� ����� ��������� ���������
			//���������, �������� �� ���������� ������� ���������; ���� ���, �� ��������� ������; ���� ��, �� ������ �� ������ � ����������
			//������ ��������� list
			if (pSplitElem != ListBase<ListElement>::pLast)
			{
				CheckingPresenceElementPolicy<ListElement, true>::RemoveContainer(iterator{ pSplitElem->pNext, this });

				list.ListBase<ListElement>::pFirst = pSplitElem->pNext;
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					list.ListBase<ListElement>::pFirst->pPrev.lock() = nullptr;
				else
					list.ListBase<ListElement>::pFirst->pPrev = nullptr;
				pSplitElem->pNext = nullptr;
				list.ListBase<ListElement>::pLast = ListBase<ListElement>::pLast;
				ListBase<ListElement>::pLast = pSplitElem;

				list.CheckingPresenceElementPolicy<ListElement, true>::RegisterContainer(&list, iterator{ list.pFirst, &list });
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

			unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, true>::GetNumElementsMax();			//������������ ���������� ���������, ������������� ���������� ������ ���������

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

			unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, true>::GetCurrentElementIndex();			//������������ ���������� ���������, ������������� ���������� ������ ���������

			if (!bProtected)
				UnlockListShared();			//������������ ������ �� ������
			return ullCurrentElementIndex;
		}

		//���������� � ������������� ������

		void LockListExclusive(void) const noexcept				//��������� ������ ��� ���������
		{
			LockingPolicy::LockExclusive();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(true);
		}

		void UnlockListExclusive(void) const noexcept			//������������ ������ ����� ���������
		{
			LockingPolicy::UnlockExclusive();					//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(false);
		}

		void LockListShared(void) const noexcept				//��������� ������ ��� ������
		{
			LockingPolicy::LockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(true);
		}

		void UnlockListShared(void) const noexcept				//������������ ������ ����� ������
		{
			LockingPolicy::UnlockShared();						//���������� � ���������� ����� �������� ������� ��������� ���������� �������
			bLocked.store(false);
		}

		bool GetLockStatus(void) const noexcept
		{
			return bLocked.load();
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

		ptrListElement GetNext(ptrListElement const pCurr, bool bProtected = false, bool bSerial = false) const		//���������� ��������� �� ��������� �������, �� ������� ��������� ���������� ����
		{
			//�� ����: pCurr - ��������� �� ������� ������, ������������ �������� ����� ������� � ����������, bProtected - ���� ����, ��� ��������
			//��� ��������� ������ �������� ����� �������, bSerial - ���� ����������������� �������� - � ���� ������ �� ����������
			//������� FindElement ��� ��������� ������ (������, ���� �� ���-�� ��������������� �������� �� ������ �� �������� � ��������, �� ���
			//������ ������ ������ ������� ������)
			//�� �����: ��������� �� ������� ������, ������������� ����� ����������� � ���������� �������

			if (!pCurr)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();
			ptrListElement pNext = nullptr;
			bool bPresent = true;		//���� ����������� � ������ ����������� ��������
			if (bProtected == false || bSerial == false)
				bPresent = FindElement(pCurr, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
			if (bPresent)
				pNext = pCurr->pNext;
			else
			{
				if (!bProtected)
					UnlockListShared();
				throw ListErrors::NotPartOfList();			//���������� ���������� �� ���������� �������� � ������
			}

			if (!bProtected)
				UnlockListShared();

			return pNext;
		}

		ptrListElement GetPrev(ptrListElement const pCurr, bool bProtected = false, bool bSerial = false) const		//���������� ��������� �� ���������� �������, �� ������� ��������� ���������� ����
		{
			//�� ����: pCurr - ��������� �� ������� ������, ������������ �������� ����� ������� � �����������, bProtected - ���� ����, ��� ��������
			//��� ��������� ������ �������� ����� �������, bSerial - ���� ����������������� �������� - � ���� ������ �� ����������
			//������� FindElement ��� ��������� ������ (������, ���� �� ���-�� ��������������� �������� �� ������ �� �������� � ��������, �� ���
			//������ ������ ������ ������� ������)
			//�� �����: ��������� �� �������, ������������� ����� ����������� � ���������� �������

			if (!pCurr)
				throw ListErrors::Nullptr();	//��������� ���������� �� ��������� ��������� (������� ���������)

			if (!bProtected)
				LockListShared();
			ptrListElement pPrev = nullptr;
			bool bPresent = true;		//���� ����������� � ������ ����������� ��������
			if (bProtected == false || bSerial == false)
				bPresent = FindElement(pCurr, true);		//���� ������� ������ � ��� ������, ���� ������ ������� ����� �� �������� ��� �� ������ ���� ����������������� ��������
			if (bPresent)
			{
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					pPrev = pCurr->pPrev.lock();
				else
					pPrev = pCurr->pPrev;
			}
			else
			{
				if (!bProtected)
					UnlockListShared();
				throw ListErrors::NotPartOfList();			//���������� ���������� �� ���������� �������� � ������
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

		List_TwoLinked& operator=(List_TwoLinked& list)
		{
			//�������� ����������� ������������
			//�� ����: list - ������ �� ���������� ������
			//�� �����: ���������� ������ �� ������� ������, ����� ��������� �������� ������������ ����� ���� ������������ � ������ ��������

			MakeCopy(list);			//�������� ������, ���������� �� ������������: ��� ����� �������������� �����
			return *this;
		}

		template<class List> List_TwoLinked& operator=(List& list)
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
			CheckingPresenceElementPolicy<ListElement, true>::AssignAnotherCES(std::move(list), this);
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

		//��������� ����������

	protected:

		class ListIterator					//����� ��������� ��� �������� ���������
		{
			ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
			const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
			bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

		public:

			ListIterator() {}
			ListIterator(ptrListElement pElem, const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList, bool bProtected = true)
				noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
			ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			ptrListElement& operator*()
			{
				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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

			friend class ListIteratorConst;
		};

		class ListIteratorConst					//����� ������������ ��������� ��� �������� ���������
		{
			ptrListElement pCurrElement{ nullptr };					//������� �������, �� ������� ��������� ��������
			const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������
			bool bProtected = true;									//���� ����, ��� ������ ���������� ����� ���������

		public:

			ListIteratorConst() {}
			ListIteratorConst(ptrListElement pElem, const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, true>* pList, bool bProtected = true)
				noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
			ListIteratorConst(const ListIteratorConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
			ListIteratorConst(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			const ptrListElement& operator*()
			{
				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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
					if (!pList->FindElement(pCurrElement, true))
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

			ListIteratorConst operator++(int)			//����������� ���������: it++
			{
				ListIteratorConst itPrev = *this;

				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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

			ListIteratorConst operator--(int)			//����������� ���������: it--
			{
				ListIteratorConst itCurr = *this;

				if (!bProtected)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
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

			friend class ListIterator;
		};

	public:

		using iterator = ListIterator;
		using const_iterator = ListIteratorConst;

		ListIterator begin(bool bProtected = false) noexcept
		{
			if (!bProtected)
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
			if(!bProtected)
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

		//����������������� �������

		template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
			template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
			template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
		friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ElementData, template<class> class _MemoryPolicy, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy>
	class List_OneLinked_DataAdapter<ElementData, _MemoryPolicy, LockingPolicy, CheckingPresenceElementPolicy, true> : 
		public List_OneLinked<std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, true>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, true>>, ListElementCompound_OneLinked<ElementData, _MemoryPolicy>, ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>>, LockingPolicy, CheckingPresenceElementPolicy, true>
	{
	public:

		//�������� ��� �������� ListElementCompound: ���� ��������� ������ �������� = DirectSearch, �� ��������� ListElementCompound_OneLinked, ��� ���� ���������
		//��������� - ListElementCompound_OneLinked_CP
		//��������� � ������������ � ��������� CheckingPresenceElementPolicy, � DirectSearch ����� �������������� ���-�� ���������� ��� ���������� ���������,
		//�� ��������� ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy> � �������� ��������� ��������� ������ ��������
		using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, true>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, true>>, ListElementCompound_OneLinked<ElementData, _MemoryPolicy>, ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>>;
		using List = List_OneLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, true>;

		//�������

		List_OneLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//�����������

		List_OneLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //����������� ��� ������������� ��������� ������ ���������

		List_OneLinked_DataAdapter(const List_OneLinked_DataAdapter& list) : List(list) {}	//����������� �����������

		List_OneLinked_DataAdapter(List_OneLinked_DataAdapter&& list) : List(list) {}		//����������� �����������: ������ ��� �������� ��������� �������� � ����������

		//��������� ���������� � STL
		using value_type = ElementData;

	protected:

		class ListIteratorCompound					//����� ��������� ��� �������� ���������
		{
			typename List::ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
			const List* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������

		public:

			ListIteratorCompound() {}
			ListIteratorCompound(typename List::ptrListElement pElem, const List* pList) noexcept : pCurrElement(pElem), pList(pList) {}
			ListIteratorCompound(const ListIteratorCompound& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			//����������� ����� ��� STL
			using iterator_category = std::forward_iterator_tag;
			using value_type = ElementData;
			using difference_type = std::ptrdiff_t;
			using pointer = typename List::ptrListElement;
			using reference = ElementData&;

			ElementData& operator*()
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				ElementData& li = **pCurrElement;
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return li;
			}
			operator bool() noexcept { return pCurrElement != nullptr; }
			operator typename List::ptrListElement() noexcept { return pCurrElement; }

			void operator++()		//���������� ���������: ++it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pCurrElement->pNext;

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			ListIteratorCompound operator++(int) noexcept									//����������� ���������: it++
			{
				ListIteratorCompound itPrev = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}
				pCurrElement = pCurrElement->pNext;
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itPrev;
			}

			bool operator==(const ListIteratorCompound lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}

			bool operator!=(const ListIteratorCompound lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = false;
				if (pList == nullptr || lic.pList == nullptr)
					bResult = !(pCurrElement == lic.pCurrElement);
				else
					bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}
		};

		class ListIteratorCompoundConst					//����� ������������ ��������� ��� �������� ���������
		{
			typename List::ptrListElement pCurrElement{ nullptr };					//������� �������, �� ������� ��������� ��������
			const List* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������

		public:

			ListIteratorCompoundConst() {}
			ListIteratorCompoundConst(typename List::ptrListElement pElem, const List* pList) noexcept : pCurrElement(pElem), pList(pList) {}
			ListIteratorCompoundConst(const ListIteratorCompoundConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
			ListIteratorCompoundConst(const ListIteratorCompound& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			//����������� ����� ��� STL
			using iterator_category = std::forward_iterator_tag;
			using value_type = const ElementData;
			using difference_type = std::ptrdiff_t;
			using pointer = const typename List::ptrListElement;
			using reference = const ElementData&;

			const ElementData& operator*()
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				const ElementData& c_li = **pCurrElement;
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return c_li;
			}
			operator bool() noexcept { return pCurrElement != nullptr; }
			operator const typename List::ptrListElement() noexcept { return pCurrElement; }

			void operator++()		//���������� ���������: ++it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetNext(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			ListIteratorCompoundConst operator++(int)			//����������� ���������: it++
			{
				ListIteratorCompoundConst itPrev = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}
				pCurrElement = pList->GetNext(pCurrElement, true, true);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itPrev;
			}

			bool operator==(const ListIteratorCompoundConst lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}

			bool operator!=(const ListIteratorCompoundConst lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = false;
				if (pList == nullptr || lic.pList == nullptr)
					bResult = !(pCurrElement == lic.pCurrElement);
				else
					bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}
		};

	public:

		using iterator = ListIteratorCompound;
		using const_iterator = ListIteratorCompoundConst;

		ListIteratorCompound begin() noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompound lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return lit;
		}

		ListIteratorCompound end() noexcept
		{
			return ListIteratorCompound();
		}

		ListIteratorCompoundConst begin() const noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompoundConst c_lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return c_lit;
		}

		ListIteratorCompoundConst end() const noexcept
		{
			return ListIteratorCompoundConst();
		}

		ListIteratorCompoundConst cbegin() const noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompoundConst c_lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return c_lit;
		}

		ListIteratorCompoundConst cend() const noexcept
		{
			return ListIteratorCompoundConst();
		}

		void push_back(const ElementData& ed)
		{
			List::AddLast(false, false, ed);
		}

		void push_back(ElementData&& ed)
		{
			List::AddLast(false, false, std::move(ed));
		}

		void push_front(const ElementData& ed)
		{
			List::AddFirst(false, ed);
		}

		void push_front(ElementData&& ed)
		{
			List::AddFirst(false, std::move(ed));
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ElementData, template<class> class _MemoryPolicy, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy>
	class List_TwoLinked_DataAdapter<ElementData, _MemoryPolicy, LockingPolicy, CheckingPresenceElementPolicy, true> : 
		public List_TwoLinked<std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, true>, DirectSearch<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, true>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>, LockingPolicy, CheckingPresenceElementPolicy, true>
	{
	public:

		//�������� ��� �������� ListElementCompound: ���� ��������� ������ �������� = DirectSearch, �� ��������� ListElementCompound_TwoLinked, ��� ���� ���������
		//��������� - ListElementCompound_TwoLinked_CP
		//��������� � ������������ � ��������� CheckingPresenceElementPolicy, � DirectSearch ����� �������������� ���-�� ���������� ��� ���������� ���������,
		//�� ��������� ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy> � �������� ��������� ��������� ������ ��������
		using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, true>, DirectSearch<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, true>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>;
		using List = List_TwoLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, true>;

		//�������

		List_TwoLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//�����������

		List_TwoLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //����������� ��� ������������� ��������� ������ ���������

		List_TwoLinked_DataAdapter(const List_TwoLinked_DataAdapter& list) : List(list) {}	//����������� �����������

		List_TwoLinked_DataAdapter(List_TwoLinked_DataAdapter&& list) : List(list) {}		//����������� �����������: ������ ��� �������� ��������� �������� � ����������

		//��������� ���������� � STL
		using value_type = ElementData;		

	protected:

		class ListIteratorCompound					//����� ��������� ��� �������� ���������
		{
			typename List::ptrListElement pCurrElement{ nullptr };						//������� �������, �� ������� ��������� ��������
			const List* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������

		public:

			ListIteratorCompound() {}
			ListIteratorCompound(typename List::ptrListElement pElem, const List* pList) noexcept : pCurrElement(pElem), pList(pList) {}
			ListIteratorCompound(const ListIteratorCompound& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			//����������� ����� ��� STL
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = ElementData;
			using difference_type = std::ptrdiff_t;
			using pointer = typename List::ptrListElement;
			using reference = ElementData&;

			ElementData& operator*()
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				ElementData& li = **pCurrElement;

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return li;
			}
			operator bool() noexcept { return pCurrElement != nullptr; }
			operator typename List::ptrListElement() noexcept { return pCurrElement; }

			void operator++()		//���������� ���������: ++it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetNext(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			void operator--()		//���������� ���������: --it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetPrev(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			ListIteratorCompound operator++(int) noexcept									//����������� ���������: it++
			{
				ListIteratorCompound itPrev = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetNext(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itPrev;
			}

			ListIteratorCompound operator--(int) noexcept									//����������� ���������: it--
			{
				ListIteratorCompound itCurr = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetPrev(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itCurr;
			}

			bool operator==(const ListIteratorCompound lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}

			bool operator!=(const ListIteratorCompound lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = false;
				if (pList == nullptr || lic.pList == nullptr)
					bResult = !(pCurrElement == lic.pCurrElement);
				else
					bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}
		};

		class ListIteratorCompoundConst				//����� ������������ ��������� ��� �������� ���������
		{
			typename List::ptrListElement pCurrElement{ nullptr };					//������� �������, �� ������� ��������� ��������
			const List* pList = nullptr;		//��������� �� ������, �������� ����������� ������ �������

		public:

			ListIteratorCompoundConst() {}
			ListIteratorCompoundConst(typename List::ptrListElement pElem, const List* pList) noexcept : pCurrElement(pElem), pList(pList) {}
			ListIteratorCompoundConst(const ListIteratorCompoundConst& lic) noexcept : pCurrElement(lic.pCurrElement), pList(lic.pList) {}
			ListIteratorCompoundConst(const ListIteratorCompound& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

			//����������� ����� ��� STL
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = const ElementData;
			using difference_type = std::ptrdiff_t;
			using pointer = const typename List::ptrListElement;
			using reference = const ElementData&;

			const ElementData& operator*()
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				const ElementData& c_li = **pCurrElement;

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return c_li;
			}
			operator bool() noexcept { return pCurrElement != nullptr; }
			operator const typename List::ptrListElement() noexcept { return pCurrElement; }

			void operator++()		//���������� ���������: ++it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetNext(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			void operator--()		//���������� ���������: --it
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetPrev(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
			}

			ListIteratorCompoundConst operator++(int)			//����������� ���������: it++
			{
				ListIteratorCompoundConst itPrev = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}

				pCurrElement = pList->GetNext(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itPrev;
			}

			ListIteratorCompoundConst operator--(int)			//����������� ���������: it--
			{
				ListIteratorCompoundConst itCurr = *this;

				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
				{
					pList->LockListShared();		//��������� ������ �� ����� ��������
					if (!pList->FindElement(pCurrElement, true))
					{
						if (!bListLocked)
							pList->UnlockListShared();
						throw ListErrors::NotPartOfList();	//��������� ���������� - ���������� ������� �� �������� ������ ������
					}
				}
				
				pCurrElement = pList->GetPrev(pCurrElement, true, true);

				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������

				return itCurr;
			}

			bool operator==(const ListIteratorCompoundConst lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}

			bool operator!=(const ListIteratorCompoundConst lic)
			{
				bool bListLocked = pList->GetLockStatus();
				if (!bListLocked)
					pList->LockListShared();		//��������� ������ �� ����� ��������
				bool bResult = false;
				if (pList == nullptr || lic.pList == nullptr)
					bResult = !(pCurrElement == lic.pCurrElement);
				else
					bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
				if (!bListLocked)
					pList->UnlockListShared();		//������������ ������ ����� ��������
				return bResult;
			}
		};

	public:

		using iterator = ListIteratorCompound;
		using const_iterator = ListIteratorCompoundConst;

		ListIteratorCompound begin() noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompound lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return lit;
		}

		ListIteratorCompound end() noexcept
		{
			return ListIteratorCompound();
		}

		ListIteratorCompoundConst begin() const noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompoundConst c_lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return c_lit;
		}

		ListIteratorCompoundConst end() const noexcept
		{
			return ListIteratorCompoundConst();
		}

		ListIteratorCompoundConst cbegin() const noexcept
		{
			bool bListLocked = List::bLocked.load();
			if (!bListLocked)
				List::LockListShared();		//��������� ������ �� ����� ��������
			ListIteratorCompoundConst c_lit(ListBase<typename List::ListElement>::pFirst, this);
			if (!bListLocked)
				List::UnlockListShared();		//������������ ������

			return c_lit;
		}

		ListIteratorCompoundConst cend() const noexcept
		{
			return ListIteratorCompoundConst();
		}

		void push_back(const ElementData& ed)
		{
			List::AddLast(false, false, ed);
		}

		void push_back(ElementData&& ed)
		{
			List::AddLast(false, false, std::move(ed));
		}

		void push_front(const ElementData& ed)
		{
			List::AddFirst(false, ed);
		}

		void push_front(ElementData&& ed)
		{
			List::AddFirst(false, std::move(ed));
		}
	};
}		//����� ����������� ������������ ��� ListMT