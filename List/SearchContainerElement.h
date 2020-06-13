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

//Header for checking presence policy classes supporting abstract container through iterators.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER					//������������ ������� Windows
#include <Windows.h>			//�������� ���������� ���� Windows
#endif

#include <memory>				//���������������� ���������
#include <limits>				//������������ ���� ��� ������ �������� �������� numeric_limits
#include <string.h>				//��� ������� memset

#ifdef _MSC_VER					//������������ ������� Windows
#include "SystemCommon.h"		//��������� ������� ������ ����������
#endif

#ifdef max
#undef max						//������ ���������������� ����������� ������� max �� numeric_limits, ������� ��� ������� "�������������"
#endif

//������ ����������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SearchContainerElementError {};										//����� ������ ��������� ������ ��������
class FailCheckPresenceStructure : public SearchContainerElementError {};	//�� ������� ������� ��������� ������ ��� ��������� ������ ��������
class BitFlagArrayIsFull : public SearchContainerElementError {};			//��������� ������ ��������: �������� ������ ��� ��� ������, �� ������ ������� ����� �������

//���������� �������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementType, bool bExceptions> class DirectSearch;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray2;
#ifdef _MSC_VER
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray_MemoryOnRequestLocal;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������ ��������� ������ ��������

template<class ElementType> class DirectSearch<ElementType, true>			//��������� ������� ������ - ������� ������ � ������ ���������������
{

	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� � ���������� ��� ��������� ������ � �������
	unsigned long long ullNumElements = 0;									//������� ���������� ��������� ����������

public:

	DirectSearch(unsigned long long) {};			//����������� � ����������� ������������ ��������� - �� ������ ������

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� �������� (� ������ ���������
		//�� ������������)
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		bool bResult = false;
		for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
		{
			if (*it == pElem)
			{
				bResult = true;
				break;
			}
		}
		return bResult;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, const Container* const pContainer, unsigned long long ullIdxStart = 0) { ullNumElements++; }		//������������ ����� ������� � ���������� - � ������ ��������� �� ������ ������

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, �������������� � ������� - � ������ ��������� �� ������ ������
		
		//������������ ���������� ��������� � �������������� ����������
		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements++;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept { ullNumElements--; };		//������� ������� - � ������ ���������� ��������� �� ������ ������

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� ������� - � ������ ���������� ��������� �� ������ ������

		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements--;
	}

	void Clear(void) noexcept { ullNumElements = 0; }

	unsigned long long GetNumElementsMax(void) const noexcept { return 0; }

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ������� ������ - ������� ��� ����������
template<class ElementType> class DirectSearch<ElementType, false>
{

	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� � ���������� ��� ��������� ������ � �������
	unsigned long long ullNumElements = 0;									//������� ���������� ��������� ����������

public:

	DirectSearch(unsigned long long) {};			//����������� � ����������� ������������ ��������� - �� ������ ������

	//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ��������� - �� ������ ������
	template<class Container> void AssignAnotherCES(DirectSearch&& ces, Container* const pNewContainer) noexcept {};

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� �������� (� ������ ���������
		//�� ������������)
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		bool bResult = false;
		for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
		{
			if (*it == pElem)
			{
				bResult = true;
				break;
			}
		}
		return bResult;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, const Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//������������ ����� ������� � ���������� - � ������ ��������� �� ������ ������
		ullNumElements++;
		return true;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, �������������� � ������� - � ������ ��������� �� ������ ������

		//������������ ���������� ��������� � �������������� ����������
		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements++;
		return true;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept { ullNumElements--; };		//������� ������� - � ������ ���������� ��������� �� ������ ������

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� ������� - � ������ ���������� ��������� �� ������ ������

		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements--;
	}

	void Clear(void) noexcept { ullNumElements = 0; }

	unsigned long long GetNumElementsMax(void) const noexcept { return 0; }

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� �������� ������� ��������, ���������� �� ������� ������� �� ������ ����������������� ��������� unique_ptr
//��� �������� �������� ��� ������������� ���������� ����� �� ������ �������� �������� �������� ullCurrentElementIndex ������ ������ ���������
//��� �� ����� ������ ��������������� � �������; ��� �������� �������� ��� �� ����� �� ������ ����������
//������� ���� ����� �������� ��������� ����� �� ������������ - ��� �������� ����� ������������� ����� ���� �� ��������� ���������������� ��������
//��� ������� � ����, ��� ���� ��� ������ ������ ����������� � ������� ������� ������� ������, ������ ��� ������� ���� ������ ���� � �������� �������
//�� �������� ���������� ��������� � ����������

template<class ElementType> class SearchByIndex_BitArray<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//��������� �� ������ ������ ��� ������� �������� ����������
	unsigned long long ullCurrentElementIndex = 0;						//������ ��������, � ������� ����� ����������� ����� ������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	template<class Container> void UpdateFlagsArray(const Container* const pContainer)
	{
		//������� � ��������� ������ ������, ������ �� ���� ������� ����, ���������� ����� ������� ����� ��� ����� ���������
		//�� ����: pContainer - ��������� �� ���������, ��� �������� ����������� ������ ������

		//��������������� ������������ �������, ������� � ����, ������ ������� ��������
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//��������� ���������� �������� (�� ������ ������, ���� ����� ����� � �� ������)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			throw BitFlagArrayIsFull();

		memset(pElementPresentFlags.get(), 0, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//������������� ���
			ullNumBitsInLastQWord--;
		}
	}

public:

	//�������

	SearchByIndex_BitArray(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullNumElementsMax)
		{
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray(const SearchByIndex_BitArray& ces)
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray(SearchByIndex_BitArray&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ��� (� ������ ��������� �� ������������)

		if (pElementPresentFlags)
		{
			if (ullCurrentElementIndex == ullNumElementsMax)
				UpdateFlagsArray(pContainer);

			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			//�������� ��������� ������� ��������� ����� � ������� ������
			auto ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;					//������������� ���

			ullCurrentElementIndex++;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (ullCurrentElementIndex == ullNumElementsMax)
					UpdateFlagsArray(pContainer);

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
				ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;				//������ ���� ������ ������������ �����
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;				//������������� ���
				ullCurrentElementIndex++;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullCurrentElementIndex = 0;
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
				return true;
			else
				return false;
		};

		if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
			return true;
		else
			return false;
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ����������

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� �������� ������� ��������, ���������� �� ������� ������� �� ������ ����������������� ��������� unique_ptr
//������� ��� ����������

template<class ElementType> class SearchByIndex_BitArray<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//��������� �� ������ ������ ��� ������� �������� ����������
	unsigned long long ullCurrentElementIndex = 0;						//������ ��������, � ������� ����� ����������� ����� ������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	template<class Container> bool UpdateFlagsArray(const Container* const pContainer) noexcept
	{
		//������� � ��������� ������ ������, ������ �� ���� ������� ����, ���������� ����� ������� ����� ��� ����� ���������
		//�� ����: pContainer - ��������� �� ���������, ��� �������� ����������� ������ ������
		//�� �����: true - �������, false - ������ (�������� ������ ������)

		//��������������� ������������ �������, ������� � ����, ������ ������� ��������
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//��������� ���������� �������� (�� ������ ������, ���� ����� ����� � �� ������)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			return false;

		memset(pElementPresentFlags.get(), 0, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//������������� ���
			ullNumBitsInLastQWord--;
		}
		return true;
	}

public:

	//�������

	SearchByIndex_BitArray(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullNumElementsMax)
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
	}

	SearchByIndex_BitArray(const SearchByIndex_BitArray& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray(SearchByIndex_BitArray&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ��� (� ������ ��������� �� ������������)
		//�� �����: true - �������, false - ������

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			if (ullCurrentElementIndex == ullNumElementsMax)
			{
				bool bResult = UpdateFlagsArray(pContainer);
				if (!bResult)
				{
					pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();
					return false;
				}
			}			

			//�������� ��������� ������� ��������� ����� � ������� ������
			auto ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;					//������������� ���

			ullCurrentElementIndex++;
			return true;
		}
		else
			return false;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������
		//�� �����: true - �������, false - ������

		//���� �����: ��������� �� �� ����� ��������� ����������, ��, ���� ������� ������ ������ �� ��������, �������� �������������� ��� ������
		//���� �� �� �����������, �� �� � ���������� ��������� ���������� ����������� ������, ������ std::numeric_limits<unsigned long long>::max(), �����������
		//��� ������� �� ��� ���������� ��������������� ��-�� ����, ��� �� ������� ����� � �������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			bool bFlagArrayIsNotFull = false;		//���� ����, ��� ������ ������ �� ��������, � ����� �������������� �������� � ������� ������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					if (ullCurrentElementIndex == ullNumElementsMax)
					{
						bool bResult = UpdateFlagsArray(pContainer);
						if (!bResult)
						{
							bFlagArrayIsNotFull = true;

							pCurr->ullElementIndex = std::numeric_limits<unsigned long long>::max();
							pCurr->pContainer = pContainer;
							ullNumElements++;

							continue;
						}
					}

					pCurr->ullElementIndex = ullCurrentElementIndex;
					pCurr->pContainer = pContainer;

					ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
					ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;				//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;				//������������� ���
					ullCurrentElementIndex++;
				}
				else
				{
					pCurr->ullElementIndex = std::numeric_limits<unsigned long long>::max();
					pCurr->pContainer = pContainer;
				}

				ullNumElements++;
			}
			return !bFlagArrayIsNotFull;
		}
		else
			return false;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
			//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullCurrentElementIndex = 0;
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
				return true;
			else
				return false;
		};

		if (pElem->pContainer == pContainer)
		{
			if (pElem->ullElementIndex < ullNumElementsMax)
			{
				if (CheckElementFlag(pElem->ullElementIndex) == true)
					return true;
				else
					return false;
			}
			else
			{
				bool bResult = false;
				for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
				{
					if (*it == pElem)
					{
						bResult = true;
						break;
					}
				}
				return bResult;
			}
		}
		else
			return false;
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ����������

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� �������� ������� ��������, ���������� �� ������� ������� �� ������ ����������������� ��������� unique_ptr
//��������� ���������� �������������� ���� ��������� SearchByIndex_BitArray, �� ������ ������� ���������� �������������� �� �� �������, ������������ ���������
//�������������� ��������� � ����������, � ������ ������ ������� ���
//�� ���� ����� �������� ������ ��������� ����������� ��������� �� ��������� � ���������� (SearchByIndex_BitArray), �� ���� ����������� �������� �������������
//������

template<class ElementType> class SearchByIndex_BitArray2<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//��������� �� ������ ������ ��� ������� �������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	//�������

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//��������� ����� � ������� ������� �������� ����
		//�� ����: Word - ��������������� �����

		auto BitIndex = Word;	//������ ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		auto BitValue = Word;	//�������� ����������� ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		for (BitIndex = 0; BitIndex < sizeof(Word) * 8ULL; BitIndex++)
		{
			BitValue = (Word & (1ULL << BitIndex)) >> BitIndex;
			if (BitValue == 0)
				break;
		}

		return BitIndex;
	}

	unsigned long long FindAndSetFirstZeroBit(unsigned long long ullIdxStart) noexcept
	{
		//����� ������� �������� ���� � ������������� ��� ����������
		//�� ����: ullIdxStart - ������, ������� � �������� ������� ������ ������� ���
		//�� �����: ������ �������� (������ ���� �� ��� ������� �������)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//������� ����������� ����
		unsigned long long ullBitIndex = 0;										//������ �������� ����
		for (; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
		{
			if (pElementPresentFlags[i] != ce_ullFullBitWordValue)
			{
				ullBitIndex = BitScanForwardZero(pElementPresentFlags[i]);
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			pElementPresentFlags[i] |= 1ULL << ullBitIndex;			//������������� ���

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

public:

	//�������

	SearchByIndex_BitArray2(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullNumElementsMax)
		{
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray2(const SearchByIndex_BitArray2& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2(SearchByIndex_BitArray2&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ���

		if (pElementPresentFlags)
		{
			//���� ������ ������� ���
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart);		//������, ������� ����� �������� ������������ ��������

			if (ullCurrentElementIndex == ullNumElementsMax)
				throw BitFlagArrayIsFull();

			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//������, ������� ����� �������� ������������ ��������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				//���� ������ ������� ���
				ullCurrentElementIndex = FindAndSetFirstZeroBit(ullCurrentElementIndex);

				if (ullCurrentElementIndex == ullNumElementsMax)
					throw BitFlagArrayIsFull();

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
				return true;
			else
				return false;
		};

		if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
			return true;
		else
			return false;
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� �������� ������� ��������, ���������� �� ������� ������� �� ������ ����������������� ��������� unique_ptr
//������� ��� ����������

template<class ElementType> class SearchByIndex_BitArray2<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//��������� �� ������ ������ ��� ������� �������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	//�������

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//��������� ����� � ������� ������� �������� ����
		//�� ����: Word - ��������������� �����

		auto BitIndex = Word;	//������ ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		auto BitValue = Word;	//�������� ����������� ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		for (BitIndex = 0; BitIndex < sizeof(Word) * 8ULL; BitIndex++)
		{
			BitValue = (Word & (1ULL << BitIndex)) >> BitIndex;
			if (BitValue == 0)
				break;
		}

		return BitIndex;
	}

	unsigned long long FindAndSetFirstZeroBit(unsigned long long ullIdxStart) noexcept
	{
		//����� ������� �������� ���� � ������������� ��� ����������
		//�� ����: ullIdxStart - ������, ������� � �������� ������� ������ ������� ���
		//�� �����: ������ �������� (������ ���� �� ��� ������� �������)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//������� ����������� ����
		unsigned long long ullBitIndex = 0;										//������ �������� ����
		for (; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
		{
			if (pElementPresentFlags[i] != ce_ullFullBitWordValue)
			{
				ullBitIndex = BitScanForwardZero(pElementPresentFlags[i]);
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			pElementPresentFlags[i] |= 1ULL << ullBitIndex;			//������������� ���

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

public:

	//�������

	SearchByIndex_BitArray2(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullNumElementsMax)
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
	}

	SearchByIndex_BitArray2(const SearchByIndex_BitArray2& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2(SearchByIndex_BitArray2&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ���
		//�� �����: true - �������, false - ������

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//���� ������ ������� ���
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart);		//������, ������� ����� �������� ������������ ��������

			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			if (ullCurrentElementIndex == ullNumElementsMax)
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������
		//�� �����: true - �������, false - ������

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//������, ������� ����� �������� ������������ ��������
			bool bFlagArrayIsNotFull = false;		//���� ����, ��� ������ ������ �� ��������, � ����� �������������� �������� � ������� ������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					//���� ������ ������� ���
					ullCurrentElementIndex = FindAndSetFirstZeroBit(ullCurrentElementIndex);

					if (ullCurrentElementIndex == ullNumElementsMax)
					{
						bFlagArrayIsNotFull = true;
						ullCurrentElementIndex = std::numeric_limits<unsigned long long>::max();
					}
				}
				else
					ullCurrentElementIndex = std::numeric_limits<unsigned long long>::max();

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullNumElements++;
			}
			return !bFlagArrayIsNotFull;
		}
		else
			return false;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
			//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
				return true;
			else
				return false;
		};

		if (pElem->pContainer == pContainer)
		{
			if (pElem->ullElementIndex < ullNumElementsMax)
			{
				if (CheckElementFlag(pElem->ullElementIndex) == true)
					return true;
				else
					return false;
			}
			else
			{
				bool bResult = false;
				for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
				{
					if (*it == pElem)
					{
						bResult = true;
						break;
					}
				}
				return bResult;
			}
		}
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

//��������� �������� ������� ��������, ���������� �� ������� ������� �� ������ ����������������� ��������� unique_ptr
//��� �������� �������� ��� ������������� ���������� ����� �� ������ �������� �������� �������� ullCurrentElementIndex ������ ������ ���������
//��� �� ����� ������ ��������������� � �������; ��� �������� �������� ��� �� ����� �� ������ ����������
//������� ���� ����� �������� ��������� ����� �� ������������ - ��� �������� ����� ������������� ����� ���� �� ��������� ���������������� ��������
//��� ������� � ����, ��� ���� ��� ������ ������ ����������� � ������� ������� ������� ������, ������ ��� ������� ���� ������ ���� � �������� �������
//�� �������� ���������� ��������� � ����������

//��������� ��������� ���������� SearchByIndex_BitArray �� ����������� ����, ��� ������ ��� ������� ������ ������ �������������, � � ���������� ���������
//������ �� ���� �������������; ������������ ��������� ��������� ���������� ����� SEH

template<class ElementType> class SearchByIndex_BitArray_MemoryOnRequestLocal<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	unsigned long long* pElementPresentFlags = nullptr;					//��������� �� ������ ������ ��� ������� �������� ����������; ���������� ����������� ��������� �++, �.�. ����� __try/__except �� ������������ ������� ������� ������ ���
	unsigned long long ullCurrentElementIndex = 0;						//������ ��������, � ������� ����� ����������� ����� ������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	unsigned long long ullMinMemorySizeAvailable = 0;					//������ ������� ������, ������� ���������� �������� ��������� � �������
	static constexpr float ce_fMemBusyRatio = 0.9f;						//������ �������� ���������� ������, ������� ������ ��������� (�� ���������)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	template<class Container> void UpdateFlagsArray(const Container* const pContainer)
	{
		//������� � ��������� ������ ������, ������ �� ���� ������� ����, ���������� ����� ������� ����� ��� ����� ���������
		//�� ����: pContainer - ��������� �� ���������, ��� �������� ����������� ������ ������

		//��������������� ������������ �������, ������� � ����, ������ ������� ��������
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//��������� ���������� �������� (�� ������ ������, ���� ����� ����� � �� ������)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			throw BitFlagArrayIsFull();

		SecureZeroMemory(pElementPresentFlags, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//������������� ���
			ullNumBitsInLastQWord--;
		}
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation)
	{
		//������� ���������� ������ �� ���������� ������ �������� �������
		//�� ����: pExceptionInfo - ��������� �� ���������� �� ����������, pNewElem - ������ �� ����������� ������� ����������, bRepeatOperation - ���� ����������
		//�������� �������� ������
		//�� �����: ��������� ������� ����������

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//������� �������� ������ ����� �������� �������
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//�������� ����� ������� ���������

			//�������� �������� ���������� ������
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem)
	{
		//������������� ��� � �������, ��������������� �������� ����������
		//�� ����: ullElementIndex - ������ � �������, pNewElem - ������ �� ����������� ������� ����������
		//�� �����: true - ��������� ���� ������ �������, false - �������

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
		bool bRepeatOperation = true;												//���� ���������� �������� �������� ������
		bool bMemoryCommitSuccess = false;											//���� ���������� �������� ������
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//������������� ���
				bRepeatOperation = false;
				bMemoryCommitSuccess = true;
			}
			__except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? MemoryCommitForBitArray(GetExceptionInformation(), pNewElem, bRepeatOperation) : EXCEPTION_CONTINUE_SEARCH)
			{				
			}
		}

		return bMemoryCommitSuccess;
	}

public:

	//�������

	//�����������
	SearchByIndex_BitArray_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio);
		}
		
		if (ullNumElementsMax)
		{
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(const SearchByIndex_BitArray_MemoryOnRequestLocal& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	~SearchByIndex_BitArray_MemoryOnRequestLocal()
	{
		if (pElementPresentFlags)
		{
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
			pElementPresentFlags = nullptr;
		}
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		if (pElementPresentFlags)
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ��� (� ������ ��������� �� ������������)

		if (pElementPresentFlags)
		{
			if (ullCurrentElementIndex == ullNumElementsMax)
				UpdateFlagsArray(pContainer);

			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			//�������� ��������� ������� ��������� ����� � ������� ������
			bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pNewElem);		//������������� ��� � �������, ��������������� �������� ����������
			if(bMemoryCommitSuccess)
				ullCurrentElementIndex++;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (ullCurrentElementIndex == ullNumElementsMax)
					UpdateFlagsArray(pContainer);

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pCurr);		//������������� ��� � �������, ��������������� �������� ����������			
				if (bMemoryCommitSuccess)
					ullCurrentElementIndex++;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
			//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				{
					//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
					//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
				}

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
				pCurr = pCurr->pNext;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullCurrentElementIndex = 0;
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex)
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//����, �����������, ��� ������� ����������� ���������� (���� true) ���� ��� (���� false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//���� ��������� ������� - ������ �� ������: ������, ��� ���� ��� �� ���� �������� ������, � ������� ��� ����

			return bElementFlag;
		};

		//���� ������ �������� ���������, ��� ��������������� ��� ��� � ������� ���� ������� �������� ������, �� ��������� �������� ����� ���� ��������
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//� ��������� ������, �.�. ������� ���������� �� ��� ��������������� � �������, �� ������� ������ ��� � ���������� ��������

			bool bResult = false;
			for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
			{
				if (*it == pElem)
				{
					bResult = true;
					break;
				}
			}
			return bResult;
		}
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ����������

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ��������� ���������� SearchByIndex_BitArray �� ����������� ����, ��� ������ ��� ������� ������ ������ �������������, � � ���������� ���������
//������ �� ���� �������������; ������������ ��������� ��������� ���������� ����� SEH
//������� ��� ����������

template<class ElementType> class SearchByIndex_BitArray_MemoryOnRequestLocal<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	unsigned long long* pElementPresentFlags = nullptr;					//��������� �� ������ ������ ��� ������� �������� ����������; ���������� ����������� ��������� �++, �.�. ����� __try/__except �� ������������ ������� ������� ������ ���
	unsigned long long ullCurrentElementIndex = 0;						//������ ��������, � ������� ����� ����������� ����� ������� ����������
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	unsigned long long ullMinMemorySizeAvailable = 0;					//������ ������� ������, ������� ���������� �������� ��������� � �������
	static constexpr float ce_fMemBusyRatio = 0.9f;						//������ �������� ���������� ������, ������� ������ ��������� (�� ���������)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	template<class Container> bool UpdateFlagsArray(const Container* const pContainer) noexcept
	{
		//������� � ��������� ������ ������, ������ �� ���� ������� ����, ���������� ����� ������� ����� ��� ����� ���������
		//�� ����: pContainer - ��������� �� ���������, ��� �������� ����������� ������ ������
		//�� �����: true - �������, false - ������ (�������� ������ ������)

		//��������������� ������������ �������, ������� � ����, ������ ������� ��������
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//��������� ���������� �������� (�� ������ ������, ���� ����� ����� � �� ������)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			return false;

		SecureZeroMemory(pElementPresentFlags, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//������������� ���
			ullNumBitsInLastQWord--;
		}
		return true;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//������� ���������� ������ �� ���������� ������ �������� �������
		//�� ����: pExceptionInfo - ��������� �� ���������� �� ����������, pNewElem - ������ �� ����������� ������� ����������, bRepeatOperation - ���� ����������
		//�������� �������� ������
		//�� �����: ��������� ������� ����������

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//������� �������� ������ ����� �������� �������
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//�������� ����� ������� ���������

			//�������� �������� ���������� ������
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//������������� ��� � �������, ��������������� �������� ����������
		//�� ����: ullElementIndex - ������ � �������, pNewElem - ������ �� ����������� ������� ����������
		//�� �����: true - ��������� ���� ������ �������, false - �������

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
		bool bRepeatOperation = true;												//���� ���������� �������� �������� ������
		bool bMemoryCommitSuccess = false;											//���� ���������� �������� ������
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//������������� ���
				bRepeatOperation = false;
				bMemoryCommitSuccess = true;
			}
			__except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? MemoryCommitForBitArray(GetExceptionInformation(), pNewElem, bRepeatOperation) : EXCEPTION_CONTINUE_SEARCH)
			{
			}
		}

		return bMemoryCommitSuccess;
	}

public:

	//�������

	//�����������
	SearchByIndex_BitArray_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = static_cast<unsigned long long>(mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio));
		}

		if (ullNumElementsMax)
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(const SearchByIndex_BitArray_MemoryOnRequestLocal& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	~SearchByIndex_BitArray_MemoryOnRequestLocal()
	{
		if (pElementPresentFlags)
		{
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
			pElementPresentFlags = nullptr;
		}
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		if (pElementPresentFlags)
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ��� (� ������ ��������� �� ������������)

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//��������� ���������� � ����������� ������ �������� ����������
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			if (ullCurrentElementIndex == ullNumElementsMax)
			{
				bool bResult = UpdateFlagsArray(pContainer);
				if (!bResult)
				{
					pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();
					return false;
				}
			}			

			//�������� ��������� ������� ��������� ����� � ������� ������
			bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pNewElem);		//������������� ��� � �������, ��������������� �������� ����������
			if (bMemoryCommitSuccess)
				ullCurrentElementIndex++;
			return true;
		}
		else
			return false;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������
		//�� �����: true - �������, false - ������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			bool bFlagArrayIsNotFull = false;		//���� ����, ��� ������ ������ �� ��������, � ����� �������������� �������� � ������� ������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					if (ullCurrentElementIndex == ullNumElementsMax)
					{
						bool bResult = UpdateFlagsArray(pContainer);
						if (!bResult)
						{
							bFlagArrayIsNotFull = true;

							pCurr->ullElementIndex = std::numeric_limits<unsigned long long>::max();
							pCurr->pContainer = pContainer;
							ullNumElements++;

							continue;
						}
					}

					pCurr->ullElementIndex = ullCurrentElementIndex;
					pCurr->pContainer = pContainer;

					bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pCurr);		//������������� ��� � �������, ��������������� �������� ����������			
					if (bMemoryCommitSuccess)
						ullCurrentElementIndex++;
				}
				else
				{
					pCurr->ullElementIndex = std::numeric_limits<unsigned long long>::max();
					pCurr->pContainer = pContainer;
				}

				ullNumElements++;
			}
			return !bFlagArrayIsNotFull;
		}
		else
			return false;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
			//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				{
					//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
					//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
				}

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
				pCurr = pCurr->pNext;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullCurrentElementIndex = 0;
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex)
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//����, �����������, ��� ������� ����������� ���������� (���� true) ���� ��� (���� false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//���� ��������� ������� - ������ �� ������: ������, ��� ���� ��� �� ���� �������� ������, � ������� ��� ����

			return bElementFlag;
		};

		//���� ������ �������� ���������, ��� ��������������� ��� ��� � ������� ���� ������� �������� ������, �� ��������� �������� ����� ���� ��������
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//� ��������� ������, �.�. ������� ���������� �� ��� ��������������� � �������, �� ������� ������ ��� � ���������� ��������

			bool bResult = false;
			for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
			{
				if (*it == pElem)
				{
					bResult = true;
					break;
				}
			}
			return bResult;
		}
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ����������

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ��������� ���������� SearchByIndex_BitArray2 �� ����������� ����, ��� ������ ��� ������� ������ ������ �������������, � � ���������� ���������
//������ �� ���� �������������; ������������ ��������� ��������� ���������� ����� SEH

template<class ElementType> class SearchByIndex_BitArray2_MemoryOnRequestLocal<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	unsigned long long* pElementPresentFlags = nullptr;					//��������� �� ������ ������ ��� ������� �������� ����������; ���������� ����������� ��������� �++, �.�. ����� __try/__except �� ������������ ������� ������� ������ ���
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	unsigned long long ullMinMemorySizeAvailable = 0;					//������ ������� ������, ������� ���������� �������� ��������� � �������
	static constexpr float ce_fMemBusyRatio = 0.9f;						//������ �������� ���������� ������, ������� ������ ��������� (�� ���������)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	//�������

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//��������� ����� � ������� ������� �������� ����
		//�� ����: Word - ��������������� �����

		auto BitIndex = Word;	//������ ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		auto BitValue = Word;	//�������� ����������� ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		for (BitIndex = 0; BitIndex < sizeof(Word) * 8ULL; BitIndex++)
		{
			BitValue = (Word & (1ULL << BitIndex)) >> BitIndex;
			if (BitValue == 0)
				break;
		}

		return BitIndex;
	}

	unsigned long long FindAndSetFirstZeroBit(unsigned long long ullIdxStart, ptrElementType const& pNewElem) noexcept
	{
		//����� ������� �������� ���� � ������������� ��� ����������
		//�� ����: ullIdxStart - ������, ������� � �������� ������� ������ ������� ���; pNewElem - ������ �� ��������� ������������ ��������
		//�� �����: ������ �������� (������ ���� �� ��� ������� �������)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//������� ����������� ����
		unsigned long long ullBitIndex = 0;										//������ �������� ����
		for (; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
		{
			__try
			{
				if (pElementPresentFlags[i] != ce_ullFullBitWordValue)
				{
					ullBitIndex = BitScanForwardZero(pElementPresentFlags[i]);
					break;
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				ullBitIndex = 0;		//��������� ������� - ������ ��� �� ���� ��������: ��������������� �����, ������ ������ ��� ����� �������
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			SetBit(i * ce_ullNumBitsInWord + ullBitIndex, pNewElem);			//������������� ���

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//������� ���������� ������ �� ���������� ������ �������� �������
		//�� ����: pExceptionInfo - ��������� �� ���������� �� ����������, pNewElem - ������ �� ����������� ������� ����������, bRepeatOperation - ���� ����������
		//�������� �������� ������
		//�� �����: ��������� ������� ����������

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//������� �������� ������ ����� �������� �������
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//�������� ����� ������� ���������

			//�������� �������� ���������� ������
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//������������� ��� � �������, ��������������� �������� ����������
		//�� ����: ullElementIndex - ������ � �������, pNewElem - ������ �� ����������� ������� ����������
		//�� �����: true - ��������� ���� ������ �������, false - �������

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
		bool bRepeatOperation = true;												//���� ���������� �������� �������� ������
		bool bMemoryCommitSuccess = false;											//���� ���������� �������� ������
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//������������� ���
				bRepeatOperation = false;
				bMemoryCommitSuccess = true;
			}
			__except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? MemoryCommitForBitArray(GetExceptionInformation(), pNewElem, bRepeatOperation) : EXCEPTION_CONTINUE_SEARCH)
			{
			}
		}

		return bMemoryCommitSuccess;
	}

public:

	//�������

	SearchByIndex_BitArray2_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = static_cast<decltype(ullMinMemorySizeAvailable)>(mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio));
		}

		if (ullNumElementsMax)
		{
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(const SearchByIndex_BitArray2_MemoryOnRequestLocal& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	~SearchByIndex_BitArray2_MemoryOnRequestLocal()
	{
		if (pElementPresentFlags)
		{
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
			pElementPresentFlags = nullptr;
		}
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		if (pElementPresentFlags)
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ���

		if (pElementPresentFlags)
		{
			//���� ������ ������� ���
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart, pNewElem);		//������, ������� ����� �������� ������������ ��������

			if (ullCurrentElementIndex == ullNumElementsMax)
				throw BitFlagArrayIsFull();

			//��������� ���������� � ����������� ������ �������� ����������
			if (pNewElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//������, ������� ����� �������� ������������ ��������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				//���� ������ ������� ���
				ullCurrentElementIndex = FindAndSetFirstZeroBit(ullCurrentElementIndex, pCurr);

				if (ullCurrentElementIndex == ullNumElementsMax)
					throw BitFlagArrayIsFull();

				if (pCurr->ullElementIndex != std::numeric_limits<unsigned long long>::max())
					pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//����, �����������, ��� ������� ����������� ���������� (���� true) ���� ��� (���� false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//���� ��������� ������� - ������ �� ������: ������, ��� ���� ��� �� ���� �������� ������, � ������� ��� ����

			return bElementFlag;
		};

		//���� ������ �������� ���������, ��� ��������������� ��� ��� � ������� ���� ������� �������� ������, �� ��������� �������� ����� ���� ��������
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//� ��������� ������, �.�. ������� ���������� �� ��� ��������������� � �������, �� ������� ������ ��� � ���������� ��������

			bool bResult = false;
			for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
			{
				if (*it == pElem)
				{
					bResult = true;
					break;
				}
			}
			return bResult;
		}
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ��������� ���������� SearchByIndex_BitArray2 �� ����������� ����, ��� ������ ��� ������� ������ ������ �������������, � � ���������� ���������
//������ �� ���� �������������; ������������ ��������� ��������� ���������� ����� SEH
//������� ��� ����������

template<class ElementType> class SearchByIndex_BitArray2_MemoryOnRequestLocal<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//��������� ��� ��������� �� ������ �������� ���������� � ���������� ��� ��������� ������ � �������

	unsigned long long* pElementPresentFlags = nullptr;					//��������� �� ������ ������ ��� ������� �������� ����������; ���������� ����������� ��������� �++, �.�. ����� __try/__except �� ������������ ������� ������� ������ ���
	unsigned long long ullNumElementsMax = 0;							//������������ ���������� ���������, ������������� �����������
	unsigned long long ullNumElements = 0;								//������� ���������� ��������� ����������

	unsigned long long ullMinMemorySizeAvailable = 0;					//������ ������� ������, ������� ���������� �������� ��������� � �������
	static constexpr float ce_fMemBusyRatio = 0.9f;						//������ �������� ���������� ������, ������� ������ ��������� (�� ���������)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//���������� ��� � ����� �������������� ����� (��� x64 = 64 ���)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//�������� ��������������� �����, ������������ ������� ���������� ������

	//�������

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//��������� ����� � ������� ������� �������� ����
		//�� ����: Word - ��������������� �����

		auto BitIndex = Word;	//������ ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		auto BitValue = Word;	//�������� ����������� ���� (�������������� ������ Value, ����� ������������ ��� ��� ������)
		for (BitIndex = 0; BitIndex < sizeof(Word) * 8ULL; BitIndex++)
		{
			BitValue = (Word & (1ULL << BitIndex)) >> BitIndex;
			if (BitValue == 0)
				break;
		}

		return BitIndex;
	}

	unsigned long long FindAndSetFirstZeroBit(unsigned long long ullIdxStart, ptrElementType const& pNewElem) noexcept
	{
		//����� ������� �������� ���� � ������������� ��� ����������
		//�� ����: ullIdxStart - ������, ������� � �������� ������� ������ ������� ���; pNewElem - ������ �� ��������� ������������ ��������
		//�� �����: ������ �������� (������ ���� �� ��� ������� �������)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//������� ����������� ����
		unsigned long long ullBitIndex = 0;										//������ �������� ����
		for (; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
		{
			__try
			{
				if (pElementPresentFlags[i] != ce_ullFullBitWordValue)
				{
					ullBitIndex = BitScanForwardZero(pElementPresentFlags[i]);
					break;
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				ullBitIndex = 0;		//��������� ������� - ������ ��� �� ���� ��������: ��������������� �����, ������ ������ ��� ����� �������
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			SetBit(i * ce_ullNumBitsInWord + ullBitIndex, pNewElem);			//������������� ���

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//������� ���������� ������ �� ���������� ������ �������� �������
		//�� ����: pExceptionInfo - ��������� �� ���������� �� ����������, pNewElem - ������ �� ����������� ������� ����������, bRepeatOperation - ���� ����������
		//�������� �������� ������
		//�� �����: ��������� ������� ����������

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//������� �������� ������ ����� �������� �������
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//�������� ����� ������� ���������

			//�������� �������� ���������� ������
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//������� ����, ��� ���������� ������ ��� �� �������
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//������������� ��� � �������, ��������������� �������� ����������
		//�� ����: ullElementIndex - ������ � �������, pNewElem - ������ �� ����������� ������� ����������
		//�� �����: true - ��������� ���� ������ �������, false - �������

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//������ ������������ ����� ������ �������
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
		bool bRepeatOperation = true;												//���� ���������� �������� �������� ������
		bool bMemoryCommitSuccess = false;											//���� ���������� �������� ������
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//������������� ���
				bRepeatOperation = false;
				bMemoryCommitSuccess = true;
			}
			__except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) ? MemoryCommitForBitArray(GetExceptionInformation(), pNewElem, bRepeatOperation) : EXCEPTION_CONTINUE_SEARCH)
			{
			}
		}

		return bMemoryCommitSuccess;
	}

public:

	//�������

	SearchByIndex_BitArray2_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)			//�����������
	{
		//�� ����: ullNumElementsMax - ������������ ���������� ���������, ������������� �����������

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//�������� ���������� � ��������� ��������� ������
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = static_cast<decltype(ullMinMemorySizeAvailable)>(mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio));
		}

		if (ullNumElementsMax)
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(const SearchByIndex_BitArray2_MemoryOnRequestLocal& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces) noexcept
	{
		//����������� �����������; ces - ������������ �� Checking Element Strategy

		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	~SearchByIndex_BitArray2_MemoryOnRequestLocal()
	{
		if (pElementPresentFlags)
		{
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
			pElementPresentFlags = nullptr;
		}
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces, Container* const pNewContainer) noexcept
	{
		//��������� ������ ��������� �������� ������������� �������� � ���������� ������ � ���� ��������� ���������� �� ��������� ��� ���������

		if (pElementPresentFlags)
			VirtualFree(pElementPresentFlags, 0, MEM_RELEASE);
		pElementPresentFlags = ces.pElementPresentFlags;
		ces.pElementPresentFlags = nullptr;
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//������������ ������ ��������� ������� � ����������
		//�� ����: pNewElem - ��������� �� ����� �������, pContainer - ��������� �� ���������, ��� �������� �������������� ����� �������, ullIdxStart - ������,
		//������� � �������� ������� ������ ������� ���
		//�� �����: true - �������, false - ������

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//���� ������ ������� ���
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart, pNewElem);		//������, ������� ����� �������� ������������ ��������

			//��������� ���������� � ����������� ������ �������� ����������
			if (pNewElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			if (ullCurrentElementIndex == ullNumElementsMax)
				return false;
			else
				return true;
		}
		else
			return false;
		
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������������ �������� ����������, ������������� � �������
		//�� ����: pContainer - ��������� �� ���������, ������� ����� �������� ��������������� ������ � ���������; itStart - ��������� �������� ������������
		//��������, itEnd - �������� �������� ������������ ��������
		//�� �����: true - �������, false - ������

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//������, ������� ����� �������� ������������ ��������
			bool bFlagArrayIsNotFull = false;		//���� ����, ��� ������ ������ �� ��������, � ����� �������������� �������� � ������� ������
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					//���� ������ ������� ���
					ullCurrentElementIndex = FindAndSetFirstZeroBit(ullCurrentElementIndex, pCurr);

					if (ullCurrentElementIndex == ullNumElementsMax)
					{
						bFlagArrayIsNotFull = true;
						ullCurrentElementIndex = std::numeric_limits<unsigned long long>::max();
					}
				}
				else
					ullCurrentElementIndex = std::numeric_limits<unsigned long long>::max();

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullNumElements++;
			}
			return !bFlagArrayIsNotFull;
		}
		else
			return false;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//������� ������� �� ������� ������, ������� ��������������� ��� �������
		//�� ����: ullIndex - ������ ����������� ���� ��������

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//�������� ��� ������ ���� �� ��� ����������, �.�. ���� �������� ������ ��� ����� �������, � ������� �� ���������
			//� ��������� ������ ������ �������� �� ���� - ������ ��������� ������� ���������

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//������ ���� ������ ������������ �����
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//������� �������� ����������, ���������� �� �������
		//�� ����: itStart - �������� ���������� ���������� ��������, itEnd - �������� ��������� ���������� ��������

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//������ ������������ ����� ������ �������
			unsigned long long ullBitIndex = 0;		//������ ���� ������ ������������ �����
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//������ ������������ ����� ������ �������
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//������ ���� ������ ������������ �����
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//�������� ���
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//������� ������, ��������� ��� ���� � ����

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//���������� ������������ ���������� ���������
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//��������� ����� ��������, �� ������� ��������� ���������� ��������
		//�� ����: pElem - ��������� �� ������� �������, pContainer - ��������� �� ���������, �� �������� ���������� ������ ������� ��������
		//�� �����: true - ������� ������ � ������������ � ����������, false - ����� ������� �� ������ ���� ��������� ����

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//�������� ����� ��� ��������
			//�� ����: ullIndex - ������ ���������������� ��������
			//�� �����: true - ������� ����������� ����������, false - �� ����������� ���� ������ ������� �� ���������� ��������

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//����, �����������, ��� ������� ����������� ���������� (���� true) ���� ��� (���� false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//���� ��������� ������� - ������ �� ������: ������, ��� ���� ��� �� ���� �������� ������, � ������� ��� ����

			return bElementFlag;
		};

		//���� ������ �������� ���������, ��� ��������������� ��� ��� � ������� ���� ������� �������� ������, �� ��������� �������� ����� ���� ��������
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//� ��������� ������, �.�. ������� ���������� �� ��� ��������������� � �������, �� ������� ������ ��� � ���������� ��������

			bool bResult = false;
			for (auto it = pContainer->cbegin(true); it != pContainer->cend(); ++it)
			{
				if (*it == pElem)
				{
					bResult = true;
					break;
				}
			}
			return bResult;
		}
	}

	unsigned long long GetNumElements(void) const noexcept
	{
		//��������� ������� ���������� ��������� � ����������

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//���������� ������� ������, � ������� ����� ����������� ����� ������� ���������� - � ������ ��������� �� ���������� ������

		return 0;
	}
};

#endif	//WINDOWS