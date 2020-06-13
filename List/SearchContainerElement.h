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

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER					//операционная система Windows
#include <Windows.h>			//основной включаемый файл Windows
#endif

#include <memory>				//интеллектуальные указатели
#include <limits>				//заголовочный файл для класса числовых пределов numeric_limits
#include <string.h>				//для функции memset

#ifdef _MSC_VER					//операционная система Windows
#include "SystemCommon.h"		//системные функции общего назначения
#endif

#ifdef max
#undef max						//данное макроопределение перекрывает функцию max из numeric_limits, поэтому его следует "разопределить"
#endif

//КЛАССЫ ИСКЛЮЧЕНИЙ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SearchContainerElementError {};										//общая ошибка стратегии поиска элемента
class FailCheckPresenceStructure : public SearchContainerElementError {};	//не удалось создать структуру данных для стратегии поиска элемента
class BitFlagArrayIsFull : public SearchContainerElementError {};			//стратегия поиска элемента: заполнен массив для бит флагов, не удаётся создать новый элемент

//ОБЪЯВЛЕНИЯ КЛАССОВ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementType, bool bExceptions> class DirectSearch;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray2;
#ifdef _MSC_VER
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray_MemoryOnRequestLocal;
template<class ElementType, bool bExceptions> class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//классы стратегий поиска элемента

template<class ElementType> class DirectSearch<ElementType, true>			//стратегия прямого поиска - элемент ищется в классе непосредственно
{

	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента и переданной ему стратегии работы с памятью
	unsigned long long ullNumElements = 0;									//текущее количество элементов контейнера

public:

	DirectSearch(unsigned long long) {};			//конструктор с количеством максимальных элементов - не делает ничего

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки (в данной стратегии
		//не используется)
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

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

	template<class Container> void RegisterElement(ptrElementType const pNewElem, const Container* const pContainer, unsigned long long ullIdxStart = 0) { ullNumElements++; }		//регистрирует новый элемент в контейнере - в данной стратегии не делает ничего

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//регистрирует элементы контейнера, присоединяемые к данному - в данной стратегии не делает ничего
		
		//подсчитываем количество элементов в регистрируемом контейнере
		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements++;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept { ullNumElements--; };		//удаляет элемент - в данной простейшей стратегии не делает ничего

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного - в данной простейшей стратегии не делает ничего

		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements--;
	}

	void Clear(void) noexcept { ullNumElements = 0; }

	unsigned long long GetNumElementsMax(void) const noexcept { return 0; }

	unsigned long long GetNumElements(void) const noexcept
	{
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия прямого поиска - вариант без исключений
template<class ElementType> class DirectSearch<ElementType, false>
{

	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента и переданной ему стратегии работы с памятью
	unsigned long long ullNumElements = 0;									//текущее количество элементов контейнера

public:

	DirectSearch(unsigned long long) {};			//конструктор с количеством максимальных элементов - не делает ничего

	//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер - не делает ничего
	template<class Container> void AssignAnotherCES(DirectSearch&& ces, Container* const pNewContainer) noexcept {};

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки (в данной стратегии
		//не используется)
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

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
		//регистрирует новый элемент в контейнере - в данной стратегии не делает ничего
		ullNumElements++;
		return true;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//регистрирует элементы контейнера, присоединяемые к данному - в данной стратегии не делает ничего

		//подсчитываем количество элементов в регистрируемом контейнере
		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements++;
		return true;
	}

	void RemoveElement(unsigned long long ullIndex) noexcept { ullNumElements--; };		//удаляет элемент - в данной простейшей стратегии не делает ничего

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного - в данной простейшей стратегии не делает ничего

		for (auto it = itStart; it != itEnd; ++it)
			ullNumElements--;
	}

	void Clear(void) noexcept { ullNumElements = 0; }

	unsigned long long GetNumElementsMax(void) const noexcept { return 0; }

	unsigned long long GetNumElements(void) const noexcept
	{
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия проверки наличия элемента, основанная на битовом массиве на основе интеллектуального указателя unique_ptr
//при создании элемента ему присваивается уникальный номер на основе текущего значения счётчика ullCurrentElementIndex внутри класса стратегии
//бит по этому номеру устанавливается в единицу; при удалении элемента бит по этому же номеру обнуляется
//нулевые биты после удалённых элементов никак не используются - при создании новых задействуются новые биты по постоянно увеличивающемуся счётчику
//это приведёт к тому, что рано или поздно память исчерпается и придётся сжимать битовый массив, удаляя все нулевые биты внутри него и обновляя счётчик
//до текущего количества элементов в контейнере

template<class ElementType> class SearchByIndex_BitArray<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//указатель на массив флагов для каждого элемента контейнера
	unsigned long long ullCurrentElementIndex = 0;						//индекс элемента, с которым будет создаваться новый элемент контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	template<class Container> void UpdateFlagsArray(const Container* const pContainer)
	{
		//сжимает и обновляет массив флагов, удаляя из него нулевые биты, освобождая таким образом место для новых элементов
		//на вход: pContainer - указатель на контейнер, для которого обновляется массив флагов

		//последовательно устаналиваем индексы, начиная с нуля, внутри каждого элемента
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//обновляем количество индексов (на случай ошибки, хотя можно этого и не делать)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			throw BitFlagArrayIsFull();

		memset(pElementPresentFlags.get(), 0, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//устанавливаем бит
			ullNumBitsInLastQWord--;
		}
	}

public:

	//функции

	SearchByIndex_BitArray(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullNumElementsMax)
		{
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray(const SearchByIndex_BitArray& ces)
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray(SearchByIndex_BitArray&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray&& ces, Container* const pNewContainer) noexcept
	{
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит (в данной стратегии не используется)

		if (pElementPresentFlags)
		{
			if (ullCurrentElementIndex == ullNumElementsMax)
				UpdateFlagsArray(pContainer);

			//сохраняем информацию о регистрации внутри элемента контейнера
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			//отмечаем созданный элемент единичным битом в массиве флагов
			auto ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;					//устанавливаем бит

			ullCurrentElementIndex++;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (ullCurrentElementIndex == ullNumElementsMax)
					UpdateFlagsArray(pContainer);

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
				ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;				//индекс бита внутри учетверённого слова
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;				//устанавливаем бит
				ullCurrentElementIndex++;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

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
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия проверки наличия элемента, основанная на битовом массиве на основе интеллектуального указателя unique_ptr
//вариант без исключений

template<class ElementType> class SearchByIndex_BitArray<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//указатель на массив флагов для каждого элемента контейнера
	unsigned long long ullCurrentElementIndex = 0;						//индекс элемента, с которым будет создаваться новый элемент контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	template<class Container> bool UpdateFlagsArray(const Container* const pContainer) noexcept
	{
		//сжимает и обновляет массив флагов, удаляя из него нулевые биты, освобождая таким образом место для новых элементов
		//на вход: pContainer - указатель на контейнер, для которого обновляется массив флагов
		//на выход: true - успешно, false - ошибка (заполнен массив флагов)

		//последовательно устаналиваем индексы, начиная с нуля, внутри каждого элемента
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//обновляем количество индексов (на случай ошибки, хотя можно этого и не делать)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			return false;

		memset(pElementPresentFlags.get(), 0, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//устанавливаем бит
			ullNumBitsInLastQWord--;
		}
		return true;
	}

public:

	//функции

	SearchByIndex_BitArray(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullNumElementsMax)
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
	}

	SearchByIndex_BitArray(const SearchByIndex_BitArray& ces) noexcept
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray(SearchByIndex_BitArray&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray&& ces, Container* const pNewContainer) noexcept
	{
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит (в данной стратегии не используется)
		//на выход: true - успешно, false - ошибка

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//сохраняем информацию о регистрации внутри элемента контейнера
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

			//отмечаем созданный элемент единичным битом в массиве флагов
			auto ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;					//устанавливаем бит

			ullCurrentElementIndex++;
			return true;
		}
		else
			return false;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента
		//на выход: true - успешно, false - ошибка

		//идея такая: поскольку мы не можем выбросить исключение, то, пока битовый массив флагов не заполнен, элементы регистрируются как обычно
		//если же он заполняется, то мы у оставшихся элементов контейнера присваиваем номера, равные std::numeric_limits<unsigned long long>::max(), указывающие
		//что элемент не был фактически зарегистрирован из-за того, что не хватило места в массиве

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			bool bFlagArrayIsNotFull = false;		//флаг того, что массив флагов не заполнен, и можно регистрировать элементы в штатном режиме
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

					ullQWordIdx = ullCurrentElementIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
					ullBitIndex = ullCurrentElementIndex % ce_ullNumBitsInWord;				//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;				//устанавливаем бит
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
			//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

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
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия проверки наличия элемента, основанная на битовом массиве на основе интеллектуального указателя unique_ptr
//полностью аналогична представленной выше стратегии SearchByIndex_BitArray, но только элемент контейнера регистрируется не по индексу, указываемому счётчиком
//использованных элементов в контейнере, а ищется первый нулевой бит
//за счёт этого скорость работы стратегии существенно снижается по сравнению с предыдущей (SearchByIndex_BitArray), но зато достигается экономия использования
//памяти

template<class ElementType> class SearchByIndex_BitArray2<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//указатель на массив флагов для каждого элемента контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	//функции

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//сканирует слово в поисках первого нулевого бита
		//на вход: Word - просматриваемое слово

		auto BitIndex = Word;	//индекс бита (праравнивается равным Value, чтобы использовать его тип данных)
		auto BitValue = Word;	//значение конкретного бита (праравнивается равным Value, чтобы использовать его тип данных)
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
		//поиск первого нулевого бита с одновременной его установкой
		//на вход: ullIdxStart - индекс, начиная с которого следует искать нулевой бит
		//на выход: индекс элемента (индекс бита во всём битовом массиве)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//счётчик учетверённых слов
		unsigned long long ullBitIndex = 0;										//индекс нулевого бита
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
			pElementPresentFlags[i] |= 1ULL << ullBitIndex;			//устанавливаем бит

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

public:

	//функции

	SearchByIndex_BitArray2(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullNumElementsMax)
		{
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
			if (!pElementPresentFlags)
				throw FailCheckPresenceStructure();
		}
	}

	SearchByIndex_BitArray2(const SearchByIndex_BitArray2& ces) noexcept
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2(SearchByIndex_BitArray2&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2&& ces, Container* const pNewContainer) noexcept
	{
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> void RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0)
	{
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит

		if (pElementPresentFlags)
		{
			//ищем первый нулевой бит
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart);		//индекс, который будет присвоен создаваемому элементу

			if (ullCurrentElementIndex == ullNumElementsMax)
				throw BitFlagArrayIsFull();

			//сохраняем информацию о регистрации внутри элемента контейнера
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//индекс, который будет присвоен создаваемому элементу
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				//ищем первый нулевой бит
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия проверки наличия элемента, основанная на битовом массиве на основе интеллектуального указателя unique_ptr
//вариант без исключений

template<class ElementType> class SearchByIndex_BitArray2<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	std::unique_ptr<unsigned long long[]> pElementPresentFlags;			//указатель на массив флагов для каждого элемента контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	//функции

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//сканирует слово в поисках первого нулевого бита
		//на вход: Word - просматриваемое слово

		auto BitIndex = Word;	//индекс бита (праравнивается равным Value, чтобы использовать его тип данных)
		auto BitValue = Word;	//значение конкретного бита (праравнивается равным Value, чтобы использовать его тип данных)
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
		//поиск первого нулевого бита с одновременной его установкой
		//на вход: ullIdxStart - индекс, начиная с которого следует искать нулевой бит
		//на выход: индекс элемента (индекс бита во всём битовом массиве)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//счётчик учетверённых слов
		unsigned long long ullBitIndex = 0;										//индекс нулевого бита
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
			pElementPresentFlags[i] |= 1ULL << ullBitIndex;			//устанавливаем бит

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

public:

	//функции

	SearchByIndex_BitArray2(unsigned long long ullNumElementsMax) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullNumElementsMax)
			pElementPresentFlags = std::make_unique<unsigned long long[]>(ullNumElementsMax / ce_ullNumBitsInWord + 1);
	}

	SearchByIndex_BitArray2(const SearchByIndex_BitArray2& ces) noexcept
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::make_unique<unsigned long long[]>(ces.ullNumElementsMax / ce_ullNumBitsInWord + 1);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2(SearchByIndex_BitArray2&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	template<class Container> void AssignAnotherCES(SearchByIndex_BitArray2&& ces, Container* const pNewContainer) noexcept
	{
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

		pElementPresentFlags = std::move(ces.pElementPresentFlags);
		ullNumElementsMax = ces.ullNumElementsMax;

		for (auto it = pNewContainer->begin(true); it != pNewContainer->end(); ++it)
			(*it)->pContainer = pNewContainer;
	}

	template<class Container> bool RegisterElement(ptrElementType const pNewElem, Container* const pContainer, unsigned long long ullIdxStart = 0) noexcept
	{
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит
		//на выход: true - успешно, false - ошибка

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//ищем первый нулевой бит
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart);		//индекс, который будет присвоен создаваемому элементу

			//сохраняем информацию о регистрации внутри элемента контейнера
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
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента
		//на выход: true - успешно, false - ошибка

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//индекс, который будет присвоен создаваемому элементу
			bool bFlagArrayIsNotFull = false;		//флаг того, что массив флагов не заполнен, и можно регистрировать элементы в штатном режиме
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					//ищем первый нулевой бит
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
			//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

//стратегия проверки наличия элемента, основанная на битовом массиве на основе интеллектуального указателя unique_ptr
//при создании элемента ему присваивается уникальный номер на основе текущего значения счётчика ullCurrentElementIndex внутри класса стратегии
//бит по этому номеру устанавливается в единицу; при удалении элемента бит по этому же номеру обнуляется
//нулевые биты после удалённых элементов никак не используются - при создании новых задействуются новые биты по постоянно увеличивающемуся счётчику
//это приведёт к тому, что рано или поздно память исчерпается и придётся сжимать битовый массив, удаляя все нулевые биты внутри него и обновляя счётчик
//до текущего количества элементов в контейнере

//стратегия полностью аналогична SearchByIndex_BitArray за исключением того, что память под битовый массив только резервируется, а в дальнейшем передаётся
//только по мере необходимости; используется локальная обработка исключений через SEH

template<class ElementType> class SearchByIndex_BitArray_MemoryOnRequestLocal<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	unsigned long long* pElementPresentFlags = nullptr;					//указатель на массив флагов для каждого элемента контейнера; используем стандартные указатели С++, т.к. блоки __try/__except не поддерживают объекты классов внутри них
	unsigned long long ullCurrentElementIndex = 0;						//индекс элемента, с которым будет создаваться новый элемент контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	unsigned long long ullMinMemorySizeAvailable = 0;					//предел размера памяти, который необходимо оставить доступным в системе
	static constexpr float ce_fMemBusyRatio = 0.9f;						//предел процента занимаемой памяти, который нельзя превышать (по умолчанию)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	template<class Container> void UpdateFlagsArray(const Container* const pContainer)
	{
		//сжимает и обновляет массив флагов, удаляя из него нулевые биты, освобождая таким образом место для новых элементов
		//на вход: pContainer - указатель на контейнер, для которого обновляется массив флагов

		//последовательно устаналиваем индексы, начиная с нуля, внутри каждого элемента
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//обновляем количество индексов (на случай ошибки, хотя можно этого и не делать)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			throw BitFlagArrayIsFull();

		SecureZeroMemory(pElementPresentFlags, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//устанавливаем бит
			ullNumBitsInLastQWord--;
		}
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation)
	{
		//передаёт физическую память по указанному адресу битового массива
		//на вход: pExceptionInfo - указатель на информацию об исключении, pNewElem - ссылка на создаваемый элемент контейнера, bRepeatOperation - флаг повторения
		//операции передачи памяти
		//на выход: результат фильтра исключения

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//пробуем передать память части битового массива
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//получаем адрес попытки обращения

			//пытаемся передать физическую память
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem)
	{
		//устанавливает бит в массиве, соответствующий элементу контейнера
		//на вход: ullElementIndex - индекс в массиве, pNewElem - ссылка на создаваемый элемент контейнера
		//на выход: true - установка бита прошла успешно, false - неудача

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
		bool bRepeatOperation = true;												//флаг повторения операции передачи памяти
		bool bMemoryCommitSuccess = false;											//флаг успешности передачи памяти
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//устанавливаем бит
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

	//функции

	//конструктор
	SearchByIndex_BitArray_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
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
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

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
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

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
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит (в данной стратегии не используется)

		if (pElementPresentFlags)
		{
			if (ullCurrentElementIndex == ullNumElementsMax)
				UpdateFlagsArray(pContainer);

			//сохраняем информацию о регистрации внутри элемента контейнера
			pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;

			//отмечаем созданный элемент единичным битом в массиве флагов
			bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pNewElem);		//устанавливаем бит в массиве, соответствующий элементу контейнера
			if(bMemoryCommitSuccess)
				ullCurrentElementIndex++;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (ullCurrentElementIndex == ullNumElementsMax)
					UpdateFlagsArray(pContainer);

				pCurr->ullElementIndex = ullCurrentElementIndex;
				pCurr->pContainer = pContainer;

				bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pCurr);		//устанавливаем бит в массиве, соответствующий элементу контейнера			
				if (bMemoryCommitSuccess)
					ullCurrentElementIndex++;

				ullNumElements++;
			}
		}
	}

	void RemoveElement(unsigned long long ullIndex) noexcept
	{
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
			//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				{
					//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
					//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
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
		//очищает массив, сбрасывая все биты в нуль

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
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex)
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//флаг, указывающий, что элемент принадлежит контейнеру (если true) либо нет (если false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//если нарушение доступа - ничего не делаем: значит, под этот бит не была выделена память, и считаем его нулём

			return bElementFlag;
		};

		//если индекс элемента указывает, что соответствующий ему бит в массиве была успешно передана память, то проверяем значение этого бита напрямую
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//в противном случае, т.к. элемент фактически не был зарегистрирован в массиве, то придётся искать его в контейнере напрямую

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия полностью аналогична SearchByIndex_BitArray за исключением того, что память под битовый массив только резервируется, а в дальнейшем передаётся
//только по мере необходимости; используется локальная обработка исключений через SEH
//вариант без исключений

template<class ElementType> class SearchByIndex_BitArray_MemoryOnRequestLocal<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	unsigned long long* pElementPresentFlags = nullptr;					//указатель на массив флагов для каждого элемента контейнера; используем стандартные указатели С++, т.к. блоки __try/__except не поддерживают объекты классов внутри них
	unsigned long long ullCurrentElementIndex = 0;						//индекс элемента, с которым будет создаваться новый элемент контейнера
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	unsigned long long ullMinMemorySizeAvailable = 0;					//предел размера памяти, который необходимо оставить доступным в системе
	static constexpr float ce_fMemBusyRatio = 0.9f;						//предел процента занимаемой памяти, который нельзя превышать (по умолчанию)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	template<class Container> bool UpdateFlagsArray(const Container* const pContainer) noexcept
	{
		//сжимает и обновляет массив флагов, удаляя из него нулевые биты, освобождая таким образом место для новых элементов
		//на вход: pContainer - указатель на контейнер, для которого обновляется массив флагов
		//на выход: true - успешно, false - ошибка (заполнен массив флагов)

		//последовательно устаналиваем индексы, начиная с нуля, внутри каждого элемента
		ullCurrentElementIndex = 0;
		for (auto it = pContainer->begin(); it != pContainer->end(); ++it)
			(*it)->ullElementIndex = ullCurrentElementIndex++;
		ullNumElements = ullCurrentElementIndex;		//обновляем количество индексов (на случай ошибки, хотя можно этого и не делать)

		if (ullCurrentElementIndex >= ullNumElementsMax)
			return false;

		SecureZeroMemory(pElementPresentFlags, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long));
		auto ullNumQWords = ullCurrentElementIndex / ce_ullNumBitsInWord;
		for (unsigned long long i = 0; i < ullNumQWords; i++)
			pElementPresentFlags[i] = ce_ullFullBitWordValue;

		auto ullNumBitsInLastQWord = ullCurrentElementIndex % ce_ullNumBitsInWord;
		while (ullNumBitsInLastQWord != 0)
		{
			pElementPresentFlags[ullNumQWords] |= 1ULL << ullNumBitsInLastQWord;					//устанавливаем бит
			ullNumBitsInLastQWord--;
		}
		return true;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//передаёт физическую память по указанному адресу битового массива
		//на вход: pExceptionInfo - указатель на информацию об исключении, pNewElem - ссылка на создаваемый элемент контейнера, bRepeatOperation - флаг повторения
		//операции передачи памяти
		//на выход: результат фильтра исключения

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//пробуем передать память части битового массива
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//получаем адрес попытки обращения

			//пытаемся передать физическую память
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//устанавливает бит в массиве, соответствующий элементу контейнера
		//на вход: ullElementIndex - индекс в массиве, pNewElem - ссылка на создаваемый элемент контейнера
		//на выход: true - установка бита прошла успешно, false - неудача

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
		bool bRepeatOperation = true;												//флаг повторения операции передачи памяти
		bool bMemoryCommitSuccess = false;											//флаг успешности передачи памяти
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//устанавливаем бит
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

	//функции

	//конструктор
	SearchByIndex_BitArray_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
			SearchByIndex_BitArray_MemoryOnRequestLocal::ullMinMemorySizeAvailable = static_cast<unsigned long long>(mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio));
		}

		if (ullNumElementsMax)
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(const SearchByIndex_BitArray_MemoryOnRequestLocal& ces) noexcept
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		ullNumElementsMax = ces.ullNumElementsMax;
		ullCurrentElementIndex = ces.ullCurrentElementIndex;
	}

	SearchByIndex_BitArray_MemoryOnRequestLocal(SearchByIndex_BitArray_MemoryOnRequestLocal&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

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
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

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
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит (в данной стратегии не используется)

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//сохраняем информацию о регистрации внутри элемента контейнера
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

			//отмечаем созданный элемент единичным битом в массиве флагов
			bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pNewElem);		//устанавливаем бит в массиве, соответствующий элементу контейнера
			if (bMemoryCommitSuccess)
				ullCurrentElementIndex++;
			return true;
		}
		else
			return false;
	}

	template<class Container, class Iterator> bool RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента
		//на выход: true - успешно, false - ошибка

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			bool bFlagArrayIsNotFull = false;		//флаг того, что массив флагов не заполнен, и можно регистрировать элементы в штатном режиме
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

					bool bMemoryCommitSuccess = SetBit(ullCurrentElementIndex, pCurr);		//устанавливаем бит в массиве, соответствующий элементу контейнера			
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
			//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				{
					//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
					//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
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
		//очищает массив, сбрасывая все биты в нуль

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
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex)
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//флаг, указывающий, что элемент принадлежит контейнеру (если true) либо нет (если false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//если нарушение доступа - ничего не делаем: значит, под этот бит не была выделена память, и считаем его нулём

			return bElementFlag;
		};

		//если индекс элемента указывает, что соответствующий ему бит в массиве была успешно передана память, то проверяем значение этого бита напрямую
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//в противном случае, т.к. элемент фактически не был зарегистрирован в массиве, то придётся искать его в контейнере напрямую

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера

		return ullCurrentElementIndex;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия полностью аналогична SearchByIndex_BitArray2 за исключением того, что память под битовый массив только резервируется, а в дальнейшем передаётся
//только по мере необходимости; используется локальная обработка исключений через SEH

template<class ElementType> class SearchByIndex_BitArray2_MemoryOnRequestLocal<ElementType, true>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	unsigned long long* pElementPresentFlags = nullptr;					//указатель на массив флагов для каждого элемента контейнера; используем стандартные указатели С++, т.к. блоки __try/__except не поддерживают объекты классов внутри них
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	unsigned long long ullMinMemorySizeAvailable = 0;					//предел размера памяти, который необходимо оставить доступным в системе
	static constexpr float ce_fMemBusyRatio = 0.9f;						//предел процента занимаемой памяти, который нельзя превышать (по умолчанию)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	//функции

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//сканирует слово в поисках первого нулевого бита
		//на вход: Word - просматриваемое слово

		auto BitIndex = Word;	//индекс бита (праравнивается равным Value, чтобы использовать его тип данных)
		auto BitValue = Word;	//значение конкретного бита (праравнивается равным Value, чтобы использовать его тип данных)
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
		//поиск первого нулевого бита с одновременной его установкой
		//на вход: ullIdxStart - индекс, начиная с которого следует искать нулевой бит; pNewElem - ссылка на указатель создаваемого элемента
		//на выход: индекс элемента (индекс бита во всём битовом массиве)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//счётчик учетверённых слов
		unsigned long long ullBitIndex = 0;										//индекс нулевого бита
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
				ullBitIndex = 0;		//нарушение доступа - память ещё не была передана: останавливаемся здесь, считая первый бит слова искомым
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			SetBit(i * ce_ullNumBitsInWord + ullBitIndex, pNewElem);			//устанавливаем бит

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//передаёт физическую память по указанному адресу битового массива
		//на вход: pExceptionInfo - указатель на информацию об исключении, pNewElem - ссылка на создаваемый элемент контейнера, bRepeatOperation - флаг повторения
		//операции передачи памяти
		//на выход: результат фильтра исключения

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//пробуем передать память части битового массива
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//получаем адрес попытки обращения

			//пытаемся передать физическую память
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//устанавливает бит в массиве, соответствующий элементу контейнера
		//на вход: ullElementIndex - индекс в массиве, pNewElem - ссылка на создаваемый элемент контейнера
		//на выход: true - установка бита прошла успешно, false - неудача

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
		bool bRepeatOperation = true;												//флаг повторения операции передачи памяти
		bool bMemoryCommitSuccess = false;											//флаг успешности передачи памяти
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//устанавливаем бит
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

	//функции

	SearchByIndex_BitArray2_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
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
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		if (!pElementPresentFlags)
			throw FailCheckPresenceStructure();
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

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
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

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
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит

		if (pElementPresentFlags)
		{
			//ищем первый нулевой бит
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart, pNewElem);		//индекс, который будет присвоен создаваемому элементу

			if (ullCurrentElementIndex == ullNumElementsMax)
				throw BitFlagArrayIsFull();

			//сохраняем информацию о регистрации внутри элемента контейнера
			if (pNewElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
				pNewElem->ullElementIndex = ullCurrentElementIndex;
			pNewElem->pContainer = pContainer;
		}
		ullNumElements++;
	}

	template<class Container, class Iterator> void RegisterContainer(Container* const pContainer, Iterator itStart, Iterator itEnd = Iterator{})
	{
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//индекс, который будет присвоен создаваемому элементу
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				//ищем первый нулевой бит
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if (ullIndex >= ullNumElementsMax || pElementPresentFlags == nullptr)
			return;

		auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
		pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
				ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
				pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит

				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//флаг, указывающий, что элемент принадлежит контейнеру (если true) либо нет (если false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//если нарушение доступа - ничего не делаем: значит, под этот бит не была выделена память, и считаем его нулём

			return bElementFlag;
		};

		//если индекс элемента указывает, что соответствующий ему бит в массиве была успешно передана память, то проверяем значение этого бита напрямую
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//в противном случае, т.к. элемент фактически не был зарегистрирован в массиве, то придётся искать его в контейнере напрямую

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия полностью аналогична SearchByIndex_BitArray2 за исключением того, что память под битовый массив только резервируется, а в дальнейшем передаётся
//только по мере необходимости; используется локальная обработка исключений через SEH
//вариант без исключений

template<class ElementType> class SearchByIndex_BitArray2_MemoryOnRequestLocal<ElementType, false>
{
	using ptrElementType = typename ElementType::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента контейнера и переданной ему стратегии работы с памятью

	unsigned long long* pElementPresentFlags = nullptr;					//указатель на массив флагов для каждого элемента контейнера; используем стандартные указатели С++, т.к. блоки __try/__except не поддерживают объекты классов внутри них
	unsigned long long ullNumElementsMax = 0;							//максимальное количество элементов, обслуживаемое контейнером
	unsigned long long ullNumElements = 0;								//текущее количество элементов контейнера

	unsigned long long ullMinMemorySizeAvailable = 0;					//предел размера памяти, который необходимо оставить доступным в системе
	static constexpr float ce_fMemBusyRatio = 0.9f;						//предел процента занимаемой памяти, который нельзя превышать (по умолчанию)

	static constexpr unsigned long long ce_ullNumBitsInWord = sizeof(unsigned long long) * 8ULL;	//количество бит в одном обрабатываемом слове (для x64 = 64 бит)
	static constexpr unsigned long long ce_ullFullBitWordValue = 0xFFFFFFFFFFFFFFFFULL;	//значение обрабатываемого слова, заполненного целиком единичными битами

	//функции

	template<class Type> Type BitScanForwardZero(Type Word) noexcept
	{
		//сканирует слово в поисках первого нулевого бита
		//на вход: Word - просматриваемое слово

		auto BitIndex = Word;	//индекс бита (праравнивается равным Value, чтобы использовать его тип данных)
		auto BitValue = Word;	//значение конкретного бита (праравнивается равным Value, чтобы использовать его тип данных)
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
		//поиск первого нулевого бита с одновременной его установкой
		//на вход: ullIdxStart - индекс, начиная с которого следует искать нулевой бит; pNewElem - ссылка на указатель создаваемого элемента
		//на выход: индекс элемента (индекс бита во всём битовом массиве)

		unsigned long long i = ullIdxStart / ce_ullNumBitsInWord;				//счётчик учетверённых слов
		unsigned long long ullBitIndex = 0;										//индекс нулевого бита
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
				ullBitIndex = 0;		//нарушение доступа - память ещё не была передана: останавливаемся здесь, считая первый бит слова искомым
				break;
			}
		}
		if (i == ullNumElementsMax)
			ullBitIndex = 0;
		else
			SetBit(i * ce_ullNumBitsInWord + ullBitIndex, pNewElem);			//устанавливаем бит

		return i * ce_ullNumBitsInWord + ullBitIndex;
	}

	DWORD MemoryCommitForBitArray(LPEXCEPTION_POINTERS pExceptionInfo, ptrElementType const& pNewElem, bool& bRepeatOperation) noexcept
	{
		//передаёт физическую память по указанному адресу битового массива
		//на вход: pExceptionInfo - указатель на информацию об исключении, pNewElem - ссылка на создаваемый элемент контейнера, bRepeatOperation - флаг повторения
		//операции передачи памяти
		//на выход: результат фильтра исключения

		auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
		if (mse.ullAvailPageFile > ullMinMemorySizeAvailable)
		{
			//пробуем передать память части битового массива
			PVOID pvAddrTouched = (PVOID)pExceptionInfo->ExceptionRecord->ExceptionInformation[1];		//получаем адрес попытки обращения

			//пытаемся передать физическую память
			if (!VirtualAlloc(pvAddrTouched, sizeof(unsigned long long), MEM_COMMIT, PAGE_READWRITE))
			{
				pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
				bRepeatOperation = false;
			}
			else
				SecureZeroMemory(pvAddrTouched, sizeof(unsigned long long));
		}
		else
		{
			pNewElem->ullElementIndex = std::numeric_limits<unsigned long long>::max();		//признак того, что установить данный бит не удалось
			bRepeatOperation = false;
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	bool SetBit(unsigned long long ullElementIndex, ptrElementType const& pNewElem) noexcept
	{
		//устанавливает бит в массиве, соответствующий элементу контейнера
		//на вход: ullElementIndex - индекс в массиве, pNewElem - ссылка на создаваемый элемент контейнера
		//на выход: true - установка бита прошла успешно, false - неудача

		auto ullQWordIdx = ullElementIndex / ce_ullNumBitsInWord;					//индекс учетверённого слова внутри массива
		unsigned long long ullBitIndex = ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
		bool bRepeatOperation = true;												//флаг повторения операции передачи памяти
		bool bMemoryCommitSuccess = false;											//флаг успешности передачи памяти
		while (bRepeatOperation)
		{
			__try
			{
				pElementPresentFlags[ullQWordIdx] |= 1ULL << ullBitIndex;						//устанавливаем бит
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

	//функции

	SearchByIndex_BitArray2_MemoryOnRequestLocal(unsigned long long ullNumElementsMax, unsigned long long ullMinMemorySizeAvailable = 0) : ullNumElementsMax(ullNumElementsMax)			//конструктор
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, обслуживаемое контейнером

		if (ullMinMemorySizeAvailable != 0)
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = ullMinMemorySizeAvailable;
		else
		{
			auto mse = SystemCommon::GetGlobalMemoryStatus();		//получаем информацию о состоянии системной памяти
			SearchByIndex_BitArray2_MemoryOnRequestLocal::ullMinMemorySizeAvailable = static_cast<decltype(ullMinMemorySizeAvailable)>(mse.ullTotalPageFile * abs(1.0f - ce_fMemBusyRatio));
		}

		if (ullNumElementsMax)
			pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(const SearchByIndex_BitArray2_MemoryOnRequestLocal& ces) noexcept
	{
		//конструктор копирования; ces - аббревиатура от Checking Element Strategy

		pElementPresentFlags = static_cast<unsigned long long*>(VirtualAlloc(NULL, (ces.ullNumElementsMax / ce_ullNumBitsInWord + 1) * sizeof(unsigned long long), MEM_RESERVE, PAGE_READWRITE));
		ullNumElementsMax = ces.ullNumElementsMax;
	}

	SearchByIndex_BitArray2_MemoryOnRequestLocal(SearchByIndex_BitArray2_MemoryOnRequestLocal&& ces) noexcept
	{
		//конструктор перемещения; ces - аббревиатура от Checking Element Strategy

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
		//присвоить другую стратегию проверки существования элемента и установить ссылки у всех элементов контейнера на владеющий ими контейнер

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
		//регистрирует заново созданный элемент в контейнере
		//на вход: pNewElem - указатель на новый элемент, pContainer - указатель на контейнер, для которого регистрируется новый элемент, ullIdxStart - индекс,
		//начиная с которого следует искать нулевой бит
		//на выход: true - успешно, false - ошибка

		ullNumElements++;
		if (pElementPresentFlags)
		{
			//ищем первый нулевой бит
			unsigned long long ullCurrentElementIndex = FindAndSetFirstZeroBit(ullIdxStart, pNewElem);		//индекс, который будет присвоен создаваемому элементу

			//сохраняем информацию о регистрации внутри элемента контейнера
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
		//регистрирует элементы контейнера, присоединённые к данному
		//на вход: pContainer - указатель на контейнер, которым нужно обновить соответствующие данные в элементах; itStart - начальный итератор обновляемого
		//элемента, itEnd - конечный итератор обновляемого элемента
		//на выход: true - успешно, false - ошибка

		if (pElementPresentFlags)
		{
			unsigned long long ullCurrentElementIndex = 0;		//индекс, который будет присвоен создаваемому элементу
			bool bFlagArrayIsNotFull = false;		//флаг того, что массив флагов не заполнен, и можно регистрировать элементы в штатном режиме
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (!bFlagArrayIsNotFull)
				{
					//ищем первый нулевой бит
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
		//удаляет элемент из массива флагов, обнуляя соответствующий ему элемент
		//на вход: ullIndex - индекс обнуляемого бита элемента

		if ((ullIndex >= ullNumElementsMax && ullIndex != std::numeric_limits<unsigned long long>::max()) || pElementPresentFlags == nullptr)
			return;

		if (ullIndex != std::numeric_limits<unsigned long long>::max())
		{
			//обнуляем бит только если он был установлен, т.е. была передана память той части массива, в которой он находится
			//в противном случае ничего обнулять не надо - просто уменьшаем счётчик элементов

			auto ullQWordIdx = ullIndex / ce_ullNumBitsInWord;				//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = ullIndex % ce_ullNumBitsInWord;	//индекс бита внутри учетверённого слова
			pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
		}
		ullNumElements--;
	}

	template<class Iterator> void RemoveContainer(Iterator itStart, Iterator itEnd = Iterator{}) noexcept
	{
		//удаляет элементы контейнера, удаляемого из данного
		//на вход: itStart - итератор начального удаляемого элемента, itEnd - итератор конечного удаляемого элемента

		if (pElementPresentFlags)
		{
			unsigned long long ullQWordIdx = 0;		//индекс учетверённого слова внутри массива
			unsigned long long ullBitIndex = 0;		//индекс бита внутри учетверённого слова
			for (auto it = itStart; it != itEnd; ++it)
			{
				ptrElementType& pCurr = *it;
				if (pCurr->ullElementIndex < ullNumElementsMax)
				{
					ullQWordIdx = pCurr->ullElementIndex / ce_ullNumBitsInWord;		//индекс учетверённого слова внутри массива
					ullBitIndex = pCurr->ullElementIndex % ce_ullNumBitsInWord;		//индекс бита внутри учетверённого слова
					pElementPresentFlags[ullQWordIdx] &= ~(1ULL << ullBitIndex);	//обнуляем бит
				}
				pCurr->ullElementIndex = 0;
				pCurr->pContainer = nullptr;

				ullNumElements--;
			}
		}
	}

	void Clear(void) noexcept
	{
		//очищает массив, сбрасывая все биты в нуль

		if (pElementPresentFlags)
		{
			for (unsigned long long i = 0; i < ullNumElementsMax / ce_ullNumBitsInWord; i++)
				pElementPresentFlags[i] = 0;
		}
		ullNumElements = 0;
	}

	unsigned long long GetNumElementsMax(void) const noexcept
	{
		//возвращает максимальное количество элементов
		return ullNumElementsMax;
	}

	template<class Container> bool FindElement(ptrElementType const pElem, const Container* const pContainer) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pContainer - указатель на контейнер, из которого вызывается данная функция проверки
		//на выход: true - элемент найден и присутствует в контейнере, false - такой элемент не найден либо контейнер пуст

		auto CheckElementFlag = [this](unsigned long long ullIndex) -> bool
		{
			//проверка флага для элемента
			//на вход: ullIndex - индекс просматриваемого элемента
			//на выход: true - элемент принадлежит контейнеру, false - не принадлежит либо индекс выходит за допустимый диапазон

			if (ullIndex >= ullNumElementsMax)
				return false;

			bool bElementFlag = false;		//флаг, указывающий, что элемент принадлежит контейнеру (если true) либо нет (если false)
			__try
			{
				if (((pElementPresentFlags[ullIndex / ce_ullNumBitsInWord] >> (ullIndex % ce_ullNumBitsInWord)) & 1ULL) == 1ULL)
					bElementFlag = true;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}		//если нарушение доступа - ничего не делаем: значит, под этот бит не была выделена память, и считаем его нулём

			return bElementFlag;
		};

		//если индекс элемента указывает, что соответствующий ему бит в массиве была успешно передана память, то проверяем значение этого бита напрямую
		if (pElem->ullElementIndex != std::numeric_limits<unsigned long long>::max())
		{
			if (pElem->pContainer == pContainer && CheckElementFlag(pElem->ullElementIndex) == true)
				return true;
			else
				return false;
		}
		else
		{
			//в противном случае, т.к. элемент фактически не был зарегистрирован в массиве, то придётся искать его в контейнере напрямую

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
		//возращает текущее количество элементов в контейнере

		return ullNumElements;
	}

	unsigned long long GetCurrentElementIndex(void) const noexcept
	{
		//возвращает текущий индекс, с которым будет создаваться новый элемент контейнера - в данной стратегии не возвращает ничего

		return 0;
	}
};

#endif	//WINDOWS