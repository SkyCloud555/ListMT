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

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <type_traits>					//работа с типами - применяется здесь для сравнения типов
#include <algorithm>					//вычисление манимума и максимума (std::min и std::max)
#include "..\COMMON POLICIES\CommonPolicies.h"				//общесистемные стратегии
#include "ListElement.h"				//элементы списка

#ifdef min
#undef min
#endif

namespace ListMT						//пространство имён многопоточного списка (односвязного и двусвязного)
{

//ГЛОБАЛЬНЫЕ КОНСТАНТЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//в случае участия в операции двух разных список создавать результат на основе списка с минимальной связностью из двух - если true, иначе - с максимальной
//связностью (если false)
constexpr inline bool ce_bGetMinLinksList = true;

//СТРУКТУРЫ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//базовый класс списка

template<class ListElement> class ListBase
{
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента списка и переданной ему стратегии работы с памятью

protected:

	//указатели на узлы списка
	ptrListElement pFirst{ nullptr };			//указатель на первый узел списка (C++ 11: инициализация члена класса)
	ptrListElement pLast{ nullptr };			//указатель на последний узел списка (C++ 11: инициализация члена класса)
};

//классы списков

//значения параметров шаблона класса: _ListElement - тип элемента списка, LockingPolicy - стратегия блокировки потоков (см. LockingPolicies.h),
//CheckingPresenceElementPolicy - стратегия проверки наличия элемента в списке, bExceptions - флаг поддержки исключений
//общее определение шаблонных классов списка
template<class ListElement, class LockingPolicy = ThreadLocking_STDMutex, template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_OneLinked;
template<class ListElement, class LockingPolicy = ThreadLocking_STDMutex, template<class, bool> class CheckingPresenceElementPolicy = DirectSearch, bool bExceptions = true> class List_TwoLinked;

//односвязный список
template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_OneLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, false> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, false>
{
public:

	//объявления типов
	using ListElement = _ListElement;								//сохраняем переданный в параметре шаблона тип элемента для доступа к нему из производных классов
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;		//извлекаем тип указателя из класса элемента списка и переданной ему стратегии работы с памятью
	
	//проверка, что внутренний указатель С++ (Type *) может использоваться только со стратегией проверки элемента прямого поиска (DirectSearch)
	static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
		std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>> == false),
		"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

private:

	//свойства и константы

	bool bTemporary = false;	//флаг временного списка, указывающий, что создаваемый список является временным, а значит, ряд операций с ним
								//должны выполняться по-другому (C++ 11: инициализация члена класса)

	static constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;		//максимальное количество элементов, создаваемое в процессе работы списка (при использовании продвинутых стратегий проверки наличия элемента)

	//вспомогательные функции
	ptrListElement FindPreviousElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//выполняет поиск элемента, который указывает на интересующий элемент
		//на вход: pElem - указатель на интересующий элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над
		//элементом списка защищена извне функции
		//на выход: указатель на предыдущий элемент относительно текущего либо nullptr, если такой элемент не найден либо список пуст

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}
		if (!bProtected)
			LockListShared();		//блокируем список на время операции
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		if (pElem == ListBase<ListElement>::pFirst)
		{
			if (!bProtected)
				UnlockListShared();		//разблокируем список
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
			UnlockListShared();		//разблокируем список

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return pCurr;
	}

	void SetEmpty(bool bProtected = false) noexcept
	{
		//опустошает список, устанавливая указатели на первый и последний элементы в нулевые значения; сами элементы из памяти не удаляются,
		//т.е. элементы списка физически остаются в памяти
		//функция используется в процессе операции физического удаления списка, а также при работе со временными списками и операциями
		//добавления/объединения списков
		//на вход: bProtected - флаг того, что операция над элементом списка, защищена извне функции		

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = nullptr;
		CheckingPresenceElementPolicy<ListElement, false>::Clear();

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected, ListErrorCodes::eListErrorCode* pErrorCode = nullptr) noexcept
	{
		//вспомогательная функция, реализующая работу двух функций Delete и Remove посредством введения дополнительного флага
		//на вход: pElem - элемент списка, предназначенный для удаления; bDelete - флаг, указывающий, удалять элемент из памяти или просто
		//исключить его из списка; bCheckPresence - флаг проверки наличия элемента в списке прежде, чем его удалять (страховка для
		//многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком), bProtected - флаг того, что операция над элементом
		//списка защищена извне функции, pErrorCode - указатель на возвращаемый код ошибки
		//на выход: true - успешно, false - список пуст либо ошибка

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			if (!bProtected)
				UnlockListExclusive();	//разблокируем список
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return false;			//список пуст
		}

		//используем флаг bSerial = true, поскольку проверка фактического наличия элемента в списке будет проводиться ниже, сейчас же нужно
		//только получить указатель на следующий элемент
		ptrListElement pNext = GetNext(pElem, pErrorCode, true, true);
		if (pElem == ListBase<ListElement>::pFirst)		//переданный элемент является первым элементом
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
			if (bDelete)
				ListElement::MemoryPolicy::Delete(pElem);
			ListBase<ListElement>::pFirst = pNext;
			if (!bProtected)
				UnlockListExclusive();		//разблокируем список
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return true;
		}

		ptrListElement pPrev = FindPreviousElement(pElem, pErrorCode, true);		//ищем элемент, указывающий на переданный
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

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return true;
	}

public:

	//функции

	List_OneLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ce_ullNumElementsMax)		//конструктор
	{
		//на вход: bTemporary - флаг того, что список создаётся как временный; этот флаг передаётся в класс стратегии блокировки потока

		List_OneLinked::bTemporary = bTemporary;
	}

	List_OneLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ullNumElementsMax)	//конструктор для инициализации стратегии поиска элементов
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, которые могут быть созданы в списке за время его работы; это значение передаётся в класс
		//стратегии поиска элемента

		List_OneLinked::bTemporary = false;
	}

	List_OneLinked(const List_OneLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, false>(list)	//конструктор копирования
	{
		MakeCopy(list);				//создаём полную копию переданного списка
	}

	List_OneLinked(List_OneLinked&& list) : CheckingPresenceElementPolicy<ListElement, false>(0)		//конструктор перемещения: служит для создания временных объектов в выражениях
	{
		//настраиваем указатели на значения указателей передаваемого в параметре списка
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		CheckingPresenceElementPolicy<ListElement, false>::AssignAnotherCES(std::move(list), this);
		list.SetEmpty();
	}

	~List_OneLinked() noexcept					//деструктор
	{
		if (!bTemporary)
			Empty();							//удаляем все элементы из списка, если объект не временный
	}

	//вспомогательные функции

	static constexpr unsigned int GetLinksNumber(void) { return 1; }		//число связей списка

	bool FindElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над
		//элементом списка защищена извне функции
		//на выход: true - элемент найден и присутствует в списке, false - такой элемент не найден либо список пуст

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (!bProtected)
			LockListShared();		//блокируем список на время операции

		bool bResult = CheckingPresenceElementPolicy<ListElement, false>::FindElement(pElem, static_cast<const List_OneLinked *>(this));

		if (!bProtected)
			UnlockListShared();		//разблокируем список

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return bResult;
	}

	//функции работы со списком

	template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в списке после переданного в параметрах узла
		//на вход: pElem - указатель на элемент узла, после которого необходимо создать новый узел, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования индекса предыдущего
		//относительно создаваемого элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//которые при добавлении нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не
		//искать нулевой бит, использовать позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из
		//файла) это значительно ускоряет работу); Args - параметры шаблонных типов, передаваемые конструктору ListElement
		//если список пуст, то значение pElem игнорируется
		//на выход: указатель на созданный элемент списка

		if (pElem == nullptr && pErrorCode != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode != nullptr)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
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

		//проверяем, является ли это первым элементом списка
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в текущем элементе списка на только что созданный
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pCurr;
			pCurr->pNext = pNext;
			//проверяем, является ли текущий элемент последним
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;			

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в списке перед переданным в параметрах узлом
		//на вход: pElem - указатель на элемент узла, перед которым необходимо создать новый узел, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования
		//индекса предыдущего относительно создаваемого элемента (это связано со стратегией поиска элемента
		//SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении нового элемента в список ищут свободный (нулевой) бит
		//в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать позицию последнего созданного элемента - при
		//последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу); Args - параметры шаблонных типов,
		//передаваемые конструктору ListElement если список пуст, то значение pElem игнорируется
		//на выход: указатель на созданный элемент списка

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode != nullptr)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
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

		//проверяем, является ли это первым элементом списка (в смысле, список до этой операции добавления элемента был пуст)
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в предыдущем элементе списка на только что созданный
			pPrev->pNext = pCurr;
			pCurr->pNext = pElem;
			//проверяем, является ли текущий элемент первым
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
			{
				if (pPrev)
					ullElementIndexStart = pPrev->ullElementIndex;
			}
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddLast(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент после последнего элемента списка
		//на вход: pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого элемента (это связано со стратегией
		//поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении нового элемента в список ищут
		//свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать позицию последнего
		//созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу); Args -
		//параметры шаблонных типов, передаваемые конструктору ListElement
		//на выход: указатель на созданный элемент списка

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в последнем элементе списка на только что созданный
			ListBase<ListElement>::pLast->pNext = pCurr;
			pCurr->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			ListBase<ListElement>::pLast = pCurr;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
				*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddFirst(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в начале списка
		//на вход: pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//Args - параметры шаблонных типов, передаваемые конструктору ListElement
		//на выход: указатель на созданный элемент списка

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			pCurr->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в созданном элементе списка на прежний первый элемент
			ptrListElement pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pCurr;
			pCurr->pNext = pNext;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	bool InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент в список после переданного в параметрах узла
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в список в заданном месте
		//на вход: pElem - указатель на элемент узла, после которого необходимо вставить новый узел, pElemToInsert - указатель на вставляемый
		//элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		//если список пуст, то значение pElem игнорируется

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
		if (!FindElement(pElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//проверяем, является ли это первым элементом списка
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в текущем элементе списка на только что созданный
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pElemToInsert;
			pElemToInsert->pNext = pNext;
			//проверяем, является ли текущий элемент последним
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент в список перед переданным в параметрах узлом
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в список в заданном месте
		//на вход: pElem - указатель на элемент узла, перед которым необходимо вставить новый узел, pElemToInsert - указатель на вставляемый
		//элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		//если список пуст, то значение pElem игнорируется

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
		ptrListElement pPrev = FindPreviousElement(pElem, true);
		if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//проверяем, является ли это первым элементом списка (в смысле, список до этой операции добавления элемента был пуст)
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в предыдущем элементе списка на только что созданный
			pPrev->pNext = pElemToInsert;
			pElemToInsert->pNext = pElem;
			//проверяем, является ли текущий элемент первым
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pPrev->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return true;
	}

	bool InsertLast(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент после последнего элемента списка
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в конце списка
		//на вход: pElemToInsert - указатель на вставляемый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в последнем элементе списка на только что созданный
			ListBase<ListElement>::pLast->pNext = pElemToInsert;
			pElemToInsert->pNext = nullptr;
			if (bUsePreviousElementIndex)
			{
				if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
					ullElementIndexStart = ListBase<ListElement>::pLast->ullElementIndex;
			}
			ListBase<ListElement>::pLast = pElemToInsert;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return true;
	}

	bool InsertFirst(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет новый элемент в начале списка
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в начале списка
		//на вход: pElemToInsert - указатель на вставляемый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции
		//на выход: true - успешно, false - неудача

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pElemToInsert;
			pElemToInsert->pNext = nullptr;
		}
		else
		{
			//устанавливаем указатель в созданном элементе списка на прежний первый элемент
			ptrListElement pNext = ListBase<ListElement>::pFirst;
			ListBase<ListElement>::pFirst = pElemToInsert;
			pElemToInsert->pNext = pNext;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool Delete(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//удаляет переданный элемент списка, обновляя значения указателя предыдущего элемента
	{
		//удаляет переданный элемент списка, обновляя значения указателя предыдущего элемента
		//на вход: pElem - элемент списка, предназначенный для удаления; pErrorCode - указатель на возвращаемый код ошибки, bCheckPresence - флаг проверки
		//наличия элемента в списке прежде, чем его удалять (страховка для многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком),
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - успешно, false - список пуст либо ошибка

		//вызываем внутреннюю вспомогательную функцию, указав ей удалить элемент списка из памяти
		return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected, pErrorCode);
	}

	bool Remove(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//исключает переданный элемент списка, обновляя значения указателя предыдущего элемента
	{
		//исключает указанный элемент из списка, сам элемент физически не удаляется из памяти
		//на вход: pElem - элемент списка, предназначенный для удаления; pErrorCode - указатель на возвращаемый код ошибки, bCheckPresence - флаг проверки
		//наличия элемента в списке прежде, чем его удалять (страховка для многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком),
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - успешно, false - список пуст либо ошибка

		//вызываем внутреннюю вспомогательную функцию, указав ей исключить элемент из списка без удаления из памяти
		return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected, pErrorCode);
	}

	ptrListElement Find(const ListElement& liElem, bool bProtected = false) const noexcept
	{
		//выполняет поиск элемента и возвращает указатель на него, если он найден; элемент списка должен поддерживать операцию ==
		//на вход: liElem - ссылка на интересующий элемент; выполняется поиск ПО СОДЕРЖИМОМУ, а не проверка присутствия элемента в списке (liElem в общем
		//случае не принадлежит списку), bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на элемент списка, если элемент с заданным содержимым найден, или nullptr в противном случае

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (*pCurr == liElem)
			{
				if (!bProtected)
					UnlockListShared();			//разблокируем список на чтение
				return pCurr;
			}
			pCurr = pCurr->pNext;
		}

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		return ptrListElement(nullptr);
	}

	unsigned long long CalculateElementsNumber(bool bProtected = false) const noexcept
	{
		//подсчитывает количество узлов в списке
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: количество элементов в списке

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, false>::GetNumElements();			//число элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullNumElements;
	}

	unsigned long long FindElementNumber(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//ищет порядковый номер элемента в списке
		//на вход: pElem - элемент списка, номер которого надо найти; pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции
		//на выход: номер элемента в списке

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullElementNumber = 0;			//номер элемента
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
				UnlockListShared();			//разблокируем список на чтение
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return ullElementNumber;
	}

	void MakeCopy(const List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//выполняет замещение текущего списка переданным, т.е. опустошает текущий список и создаёт копию переданного
		//на вход: list - ссылка на копируемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем копируемый список на время операции
		list.LockListShared();

		Empty(true);		//очищаем список
		ptrListElement pCurrToCopy = list.GetFirst(true);							//получаем первый элемент копируемого списка
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//указатели на текущий и предыдущий элементы (данного списка)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);	//создаём новый элемент, инициализируя его элементом копируемого списка
			if (pCurr == nullptr)										//неудача при выделении памяти - генерируем исключение
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//разблокируем копируемый список
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//переходим к следующему элементу копируемого списка
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем копируемый список
		list.UnlockListShared();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		
	}

	template<class List> void MakeCopy(const List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//выполняет замещение текущего списка переданным, т.е. опустошает текущий список и создаёт копию переданного
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на копируемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем копируемый список на время операции
		list.LockListShared();

		Empty(true);		//очищаем список
		ptrListElement pCurrToCopy = list.GetFirst(true);							//получаем первый элемент копируемого списка
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//указатели на текущий и предыдущий элементы (данного списка)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);	//создаём новый элемент, инициализируя его элементом копируемого списка
			if (pCurr == nullptr)										//неудача при выделении памяти - генерируем исключение
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//разблокируем копируемый список
				list.UnlockListShared();
				return;
			}
			if (pPrev)
				pPrev->pNext = pCurr;
			pCurr->pNext = nullptr;
			pPrev = pCurr;
			if (ListBase<ListElement>::pFirst == nullptr)
				ListBase<ListElement>::pFirst = pCurr;
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//переходим к следующему элементу копируемого списка
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем копируемый список
		list.UnlockListShared();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		
	}

	void Empty(bool bProtected = false) noexcept
	{
		//опустошает список, физически удаляя все его элементы из памяти
		//на вход: bProtected - флаг того, что операция над элементом списка, защищена извне функции

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//удаляем все элементы списка
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		ptrListElement pNext = nullptr;
		while (pCurr != nullptr)
		{
			pNext = pCurr->pNext;
			ListElement::MemoryPolicy::Delete(pCurr);
			pCurr = pNext;
		}
		SetEmpty(true);					//устанавливаем указатели в нулевые значения

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListAfter(List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к концу текущего, после чего опустошает (очищает) указанный список
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//продолжаем текущий список, указав на начало добавляемого
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//устанавливаем последний элемент текущего списка (если добавляемый список не пуст)			
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListAfter(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к концу текущего, после чего опустошает (очищает) указанный список
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//продолжаем текущий список, указав на начало добавляемого
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//устанавливаем последний элемент текущего списка (если добавляемый список не пуст)			
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListBefore(List_OneLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к началу текущего, после чего опустошает (очищает) указанный список
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//последний элемент добавляемого теперь указывает на начало текущего
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_OneLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//устанавливаем первый элемент текущего списка (если добавляемый список не пуст)
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListBefore(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к началу текущего, после чего опустошает (очищает) указанный список
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//последний элемент добавляемого теперь указывает на начало текущего
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//устанавливаем первый элемент текущего списка (если добавляемый список не пуст)
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	List_OneLinked Split(ptrListElement const pSplitElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//разбивает текущий список, отсекая его после указанного элемента
		//на вход: pSplitElem - указатель на элемент, после которого выполняется разбиение списка; pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: новый список, содержащий отсечённую часть

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		if (!pSplitElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		//проверяем, является ли переданный элемент частью списка
		if (!FindElement(pSplitElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		List_OneLinked list(CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax(), true);				//создаём локальный список, в котором будет храниться результат
		//проверяем, является ли переданный элемент последним; если нет, то разбиваем список; если да, то ничего не делаем и возвращаем
		//пустой локальный list
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

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return list;
	}

	bool IsEmpty(bool bProtected = false) const noexcept
	{
		//проверяет, является ли список пустым
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - список пуст, false - список НЕ пуст

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		bool bResult = false;
		if (ListBase<ListElement>::pFirst == nullptr && ListBase<ListElement>::pLast == nullptr)
			bResult = true;

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		return bResult;
	}

	unsigned long long GetNumElementsMax_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//возвращает максимальное количество элементов, обслуживаемое стратегией поиска элементов
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: количество элементов в списке

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax();			//максимальное количество элементов, обслуживаемое стратегией поиска элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullNumElementsMax;
	}

	unsigned long long GetCurrentElementIndex_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//возвращает текущий индекс, используемый для вновсь создаваемого элемента списка
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: текущий индекс, используемый для вновсь создаваемого элемента списка

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, false>::GetCurrentElementIndex();			//максимальное количество элементов, обслуживаемое стратегией поиска элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullCurrentElementIndex;
	}

	//блокировка и разблокировка списка

	void LockListExclusive(void) const noexcept				//блокирует список для изменения
	{
		LockingPolicy::LockExclusive();						//обращаемся к переданной через параметр шаблона стратегии блокировки потоков
	}

	void UnlockListExclusive(void) const noexcept			//разблокирует список после изменения
	{
		LockingPolicy::UnlockExclusive();					//обращаемся к переданной через параметр шаблона стратегии блокировки потоков
	}

	void LockListShared(void) const noexcept				//блокирует список для чтения
	{
		LockingPolicy::LockShared();						//обращаемся к переданной через параметр шаблона стратегии блокировки потоков
	}

	void UnlockListShared(void) const noexcept				//разблокирует список после чтения
	{
		LockingPolicy::UnlockShared();						//обращаемся к переданной через параметр шаблона стратегии блокировки потоков
	}

	//перемещение по узлам списка

	ptrListElement GetFirst(bool bProtected = false) const noexcept
	{
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент списка

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetFirstAndRemove(bool bProtected = false) const noexcept
	{
		//возвращает первый элемент списка и удаляет его из списка за одно действие
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент, уже не являющийся частью списка

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;
		Remove(pFirst, false, true);

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetNext(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const noexcept		//возвращает указатель на следующий элемент, на который указывает переданный узел
	{
		//на вход: pCurr - указатель на элемент списка, относительно которого нужно перейти к следующему, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bSerial - флаг последовательного перебора - в этом случае не
		//вызывается функция FindElement для ускорения работы (скажем, если мы где-то последовательно проходим по списку от элемента к элементу, то нет
		//смысла искать каждый элемент заново)
		//на выход: указатель на элемент списка, расположенный после переданного в параметрах функции

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
		bool bPresent = true;		//флаг присутствия в списке переданного элемента
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//ищем элемент только в том случае, если работа функции извне не защищена или не указан флаг последовательного перебора
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

	ptrListElement GetPrev(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept		//возвращает указатель на предыдущий элемент, на который указывает переданный узел
	{
		//на вход: pCurr - указатель на текущий элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над
		//элементом списка защищена извне функции
		//на выход: указатель на элемент, расположенный после переданного в параметрах функции

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
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на последний элемент списка

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	ptrListElement GetLastAndRemove(bool bProtected = false) const noexcept
	{
		//возвращает последний элемент списка и удаляет его из списка за одно действие
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент, уже не являющийся частью списка

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;
		Remove(pLast, false, true);

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	//операции

	List_OneLinked& operator=(List_OneLinked& list) noexcept
	{
		//операция копирующего присваивания
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	template<class List> List_OneLinked& operator=(List& list) noexcept
	{
		//операция копирующего присваивания
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	List_OneLinked& operator=(List_OneLinked&& list) noexcept
	{
		//операция перемещающего присваивания
		//на вход: list - rvalue-ссылка на копируемый список (подразумевается, что список временный)
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		LockListExclusive();		//блокируем список

		Empty(true);				//очищаем текущий список
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		list.SetEmpty();

		UnlockListExclusive();		//разблокируем список
		return *this;
	}

	template<class List> List_OneLinked& operator=(List&& list) noexcept
	{
		//операция перемещающего присваивания
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - rvalue-ссылка на копируемый список (подразумевается, что список временный)
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		LockListExclusive();		//блокируем список

		Empty(true);				//очищаем текущий список
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();

		UnlockListExclusive();		//разблокируем список
		return *this;
	}

	List_OneLinked& operator+=(List_OneLinked& list) noexcept
	{
		//операция добавления списка в конец текущего с очищением добавляемого
		//на вход: list - ссылка на добавляемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		AddListAfter(list);		//добавляем список в конец текущего, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	template<class List> List_OneLinked& operator+=(List& list) noexcept
	{
		//операция добавления списка в конец текущего с очищением добавляемого
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		AddListAfter(list);		//добавляем список в конец текущего, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	//поддержка итераторов - используются только во вспомогательных внутренних процедурах типа стратегии проверки наличия элемента; именно поэтому функции
	//работы с ними объявлены как protected

	protected:

	class ListIterator					//класс итератора для перебора элементов
	{
		ptrListElement pCurrElement{ nullptr };						//текущий элемент, на который указывает итератор
		const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//указатель на список, которому принадлежит данный элемент
		bool bProtected = true;									//флаг того, что список блокирован извне итератора

	public:

		ListIterator() {}
		ListIterator(ptrListElement pElem, const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			ptrListElement& pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//префиксный инкремент: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		ListIterator operator++(int) noexcept									//постфиксный инкремент: it++
		{
			ListIterator itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itPrev;
		}

		bool operator==(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = (pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}

		bool operator!=(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = false;
			if (pList == nullptr || li.pList == nullptr)
				bResult = !(pCurrElement == li.pCurrElement);
			else
				bResult = !(pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}
	};

	class ListIteratorConst					//класс константного итератора для перебора элементов
	{
		ptrListElement pCurrElement{ nullptr };					//текущий элемент, на который указывает итератор
		const List_OneLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//указатель на список, которому принадлежит данный элемент
		bool bProtected = true;									//флаг того, что список блокирован извне итератора

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
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			const ptrListElement& c_pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return c_pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator const ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//префиксный инкремент: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		ListIteratorConst operator++(int)			//постфиксный инкремент: it++
		{
			ListIteratorConst itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itPrev;
		}

		bool operator==(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}

		bool operator!=(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = false;
			if (pList == nullptr || lic.pList == nullptr)
				bResult = !(pCurrElement == lic.pCurrElement);
			else
				bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}
	};

	using iterator = ListIterator;
	using const_iterator = ListIteratorConst;

	ListIterator begin(bool bProtected = false) noexcept
	{
		if(!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIterator lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return lit;
	}

	ListIterator end() noexcept
	{
		return ListIterator();
	}

	ListIteratorConst begin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return c_lit;
	}

	ListIteratorConst end() const noexcept
	{
		return ListIteratorConst();
	}

	ListIteratorConst cbegin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return c_lit;
	}

	ListIteratorConst cend() const noexcept
	{
		return ListIteratorConst();
	}

	//привилегированные классы
	template<class ElementType, bool bExceptions> friend class DirectSearch;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

	//привилегированные функции

	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//значения параметров шаблона класса: _ListElement - тип элемента списка, LockingPolicy - стратегия блокировки потоков (см. LockingPolicies.h),
//CheckingPresenceElementPolicy - стратегия проверки наличия элемента в списке

//двусвязный список
template<class _ListElement, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy> class List_TwoLinked<_ListElement, LockingPolicy, CheckingPresenceElementPolicy, false> : public ListBase<_ListElement>, public LockingPolicy, protected CheckingPresenceElementPolicy<_ListElement, false>
{
public:

	//объявления типов
	using ListElement = _ListElement;								//сохраняем переданный в параметре шаблона тип элемента для доступа к нему из производных классов
	using ptrListElement = typename ListElement::MemoryPolicy::ptrType;					//извлекаем тип указателя из класса элемента списка и переданной ему стратегии работы с памятью
	using weak_ptrListElement = typename ListElement::MemoryPolicy::weak_ptrType;		//извлекаем тип для слабого указателя (поддержка интеллектуальных указателей)

	//проверка, что внутренний указатель С++ (Type *) может использоваться только со стратегией проверки элемента прямого поиска (DirectSearch)
	static_assert(!(std::is_same_v<typename ListElement::MemoryPolicy, InternalPointer<ListElement>> == true &&
		std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>> == false),
		"Internal C++ pointer memory policy can be used only with DirectSearch policy.");

private:

	//свойства и константы

	bool bTemporary = false;		//флаг временного списка, указывающий, что создаваемый список является временным, а значит, ряд операций с ним
									//должны выполняться по-другому (C++ 11: инициализация члена класса)

	static constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;		//максимальное количество элементов, создаваемое в процессе работы списка (при использовании продвинутых стратегий проверки наличия элемента)

	//вспомогательные функции
	void SetEmpty(bool bProtected = false) noexcept
	{
		//опустошает список, устанавливая указатели на первый и последний элементы в нулевые значения; сами элементы из памяти не удаляются,
		//т.е. элементы списка физически остаются в памяти
		//функция используется в процессе операции физического удаления списка, а также при работе со временными списками и операциями
		//добавления/объединения списков
		//на вход: bProtected - флаг того, что операция над элементом списка, защищена извне функции

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = nullptr;
		CheckingPresenceElementPolicy<ListElement, false>::Clear();

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

public:

	//функции

	List_TwoLinked(bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ce_ullNumElementsMax)					//конструктор
	{
		//на вход: bTemporary - флаг того, что список создаётся как временный

		List_TwoLinked::bTemporary = bTemporary;
	}

	List_TwoLinked(unsigned long long ullNumElementsMax, bool bTemporary = false) : LockingPolicy(!bTemporary), CheckingPresenceElementPolicy<ListElement, false>(ullNumElementsMax)	//конструктор для инициализации стратегии поиска элементов
	{
		//на вход: ullNumElementsMax - максимальное количество элементов, которые могут быть созданы в списке за время его работы; это значение передаётся в класс
		//стратегии поиска элемента

		List_TwoLinked::bTemporary = false;
	}

	List_TwoLinked(const List_TwoLinked& list) : LockingPolicy(true), CheckingPresenceElementPolicy<ListElement, false>(list)		//конструктор копирования
	{
		MakeCopy(list);				//создаём полную копию переданного списка
	}

	List_TwoLinked(List_TwoLinked&& list) : CheckingPresenceElementPolicy<ListElement, false>(0)				//конструктор перемещения: служит для создания временных объектов в выражениях
	{
		//настраиваем указатели на значения указателей передаваемого в параметре списка
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		CheckingPresenceElementPolicy<ListElement, false>::AssignAnotherCES(std::move(list), this);
		list.SetEmpty();
	}

	~List_TwoLinked() noexcept					//деструктор
	{
		if (!bTemporary)
			Empty();							//удаляем все элементы из списка, если объект не временный
	}

	//вспомогательные функции

	static constexpr unsigned int GetLinksNumber(void) { return 2; }		//число связей списка

	bool FindElement(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//выполняет поиск элемента, на который указывает переданный параметр
		//на вход: pElem - указатель на искомый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над
		//элементом списка защищена извне функции
		//на выход: true - элемент найден и присутствует в списке, false - такой элемент не найден либо список пуст

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (!bProtected)
			LockListShared();		//блокируем список на время операции

		bool bResult = CheckingPresenceElementPolicy<ListElement, false>::FindElement(pElem, static_cast<const List_TwoLinked *>(this));

		if (!bProtected)
			UnlockListShared();		//разблокируем список

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return bResult;
	}

	inline bool DeleteElement_Internal(ptrListElement pElem, bool bDelete, bool bCheckPresence, bool bProtected, ListErrorCodes::eListErrorCode* pErrorCode = nullptr) noexcept
	{
		//вспомогательная функция, реализующая работу двух функций Delete и Remove посредством введения дополнительного флага
		//на вход: pElem - элемент списка, предназначенный для удаления; bDelete - флаг, указывающий, удалять элемент из памяти или просто
		//исключить его из списка; bCheckPresence - флаг проверки наличия элемента в списке прежде, чем его удалять (страховка для
		//многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком), bProtected - флаг того, что операция над элементом
		//списка защищена извне функции, pErrorCode - указатель на возвращаемый код ошибки
		//на выход: true - успешно, false - список пуст либо ошибка

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			if (!bProtected)
				UnlockListExclusive();	//разблокируем список
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return false;				//список пуст
		}

		//используем флаг bSerial = true, поскольку проверка фактического наличия элемента в списке будет проводиться ниже, сейчас же нужно
		//только получить указатель на следующий элемент
		ptrListElement pNext = GetNext(pElem, pErrorCode, true, true);
		if (pElem == ListBase<ListElement>::pFirst)		//переданный элемент является первым элементом
		{
			CheckingPresenceElementPolicy<ListElement, false>::RemoveElement(pElem->ullElementIndex);
			if (bDelete)
				ListElement::MemoryPolicy::Delete(pElem);
			ListBase<ListElement>::pFirst = pNext;
			if (ListBase<ListElement>::pFirst)
			{
				//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
				if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
					ListBase<ListElement>::pFirst->pPrev.lock() = nullptr;
				else
					ListBase<ListElement>::pFirst->pPrev = nullptr;
			}
			if (!bProtected)
				UnlockListExclusive();		//разблокируем список
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Success;
			return true;
		}

		if (!(bCheckPresence == true && FindElement(pElem, pErrorCode, true) == true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();		//разблокируем список
			return true;
		}

		ptrListElement pPrev = nullptr;
		//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
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

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return true;
	}

	//функции работы со списком

	template<class... ArgTypes> ptrListElement AddAfter(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в список после переданного в параметрах узла
		//на вход: pElem - указатель на элемент узла, после которого необходимо создать новый узел, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования индекса предыдущего
		//относительно создаваемого элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//которые при добавлении нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не
		//искать нулевой бит, использовать позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из
		//файла) это значительно ускоряет работу); Args - параметры шаблонных типов, передаваемые конструктору ListElement
		//если список пуст, то значение pElem игнорируется
		//на выход: указатель на созданный элемент списка

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
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

		//проверяем, является ли это первым элементом списка
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
			//устанавливаем указатели в текущем элементе и соседних с ним
			ptrListElement pNext = pElem->pNext;
			pElem->pNext = pCurr;
			pCurr->pNext = pNext;
			//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
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
			//проверяем, является ли текущий элемент последним
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pCurr;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddBefore(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в список перед переданным в параметрах узлом
		//на вход: pElem - указатель на элемент узла, до которого необходимо создать новый узел, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования индекса предыдущего
		//относительно создаваемого элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal,
		//которые при добавлении нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не
		//искать нулевой бит, использовать позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из
		//файла) это значительно ускоряет работу); Args - параметры шаблонных типов, передаваемые конструктору ListElement
		//если список пуст, то значение pElem игнорируется
		//на выход: указатель на созданный элемент списка

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
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

		//проверяем, является ли это первым элементом списка (в смысле, список до этой операции добавления элемента был пуст)
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)//C++17: if constexpr
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//устанавливаем указатели в текущем элементе и соседних с ним
			//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
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
			//проверяем, является ли текущий элемент первым
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pCurr;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddLast(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент после последнего элемента списка
		//на вход: pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого элемента (это связано со стратегией
		//поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении нового элемента в список ищут
		//свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать позицию последнего
		//созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу); Args -
		//параметры шаблонных типов, передаваемые конструктору ListElement
		//на выход: указатель на созданный элемент списка

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (ListBase<ListElement>::pFirst == nullptr)
		{
			ListBase<ListElement>::pFirst = ListBase<ListElement>::pLast = pCurr;
			//если тип pPrev является weak_ptr, то обращаться к нему можно только после преобразования в shared_ptr с помощью функции lock()
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				pCurr->pNext = pCurr->pPrev.lock() = nullptr;
			else
				pCurr->pNext = pCurr->pPrev = nullptr;
		}
		else
		{
			//устанавливаем указатель в последнем элементе списка на только что созданный
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
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	template<class... ArgTypes> ptrListElement AddFirst(ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, ArgTypes... Args) noexcept
	{
		//создаёт новый элемент в начало списка
		//на вход: pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//Args - параметры шаблонных типов, передаваемые конструктору ListElement
		//на выход: указатель на созданный элемент списка

		ptrListElement pCurr = ListElement::MemoryPolicy::Create(Args...);
		if (!pCurr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
			return nullptr;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
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
			//устанавливаем указатель в созданном элементе списка на прежний первый элемент
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
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pCurr, this) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return pCurr;
	}

	bool InsertAfter(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент в список после переданного в параметрах узла
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в список в заданном месте
		//на вход: pElem - указатель на элемент узла, после которого необходимо вставить новый узел, pElemToInsert - указатель на вставляемый
		//элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		//если список пуст, то значение pElem игнорируется

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
		if (!FindElement(pElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//проверяем, является ли это первым элементом списка
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
			//устанавливаем указатели в текущем элементе и соседних с ним
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
			//проверяем, является ли текущий элемент последним
			if (pElem == ListBase<ListElement>::pLast)
				ListBase<ListElement>::pLast = pElemToInsert;
		}
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
		if (bUsePreviousElementIndex)
		{
			if constexpr (!std::is_same_v<CheckingPresenceElementPolicy<ListElement, false>, DirectSearch<ListElement, false>>)
				ullElementIndexStart = pElem->ullElementIndex;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertBefore(ptrListElement const pElem, ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент в список перед переданным в параметрах узлом
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в список в заданном месте
		//на вход: pElem - указатель на элемент узла, перед которым необходимо вставить новый узел, pElemToInsert - указатель на вставляемый
		//элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом списка защищена извне функции,
		//bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		//если список пуст, то значение pElem игнорируется

		if (pElem == nullptr || pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли переданный элемент частью списка
		ptrListElement pPrev = FindPreviousElement(pElem, true);
		if (pPrev == nullptr && ListBase<ListElement>::pFirst != nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			if (!bProtected)
				UnlockListExclusive();
			return false;
		}

		//проверяем, является ли это первым элементом списка (в смысле, список до этой операции добавления элемента был пуст)
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
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
			//устанавливаем указатели в текущем элементе и соседних с ним
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
			//проверяем, является ли текущий элемент первым
			if (pElem == ListBase<ListElement>::pFirst)
				ListBase<ListElement>::pFirst = pElemToInsert;
		}
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElem, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertLast(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bUsePreviousElementIndex = false) noexcept
	{
		//добавляет новый элемент после последнего элемента списка
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в конце списка
		//на вход: pElemToInsert - указатель на вставляемый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции, bUsePreviousElementIndex - флаг использования индекса предыдущего относительно создаваемого
		//элемента (это связано со стратегией поиска элемента SearchByIndex_BitArray2/SearchByIndex_BitArray2_MemoryOnRequestLocal, которые при добавлении
		//нового элемента в список ищут свободный (нулевой) бит в соответствующем битовом массиве; с данным флагом мы указываем не искать нулевой бит, использовать
		//позицию последнего созданного элемента - при последовательном создании списка (например, при загрузке данных из файла) это значительно ускоряет работу)
		//на выход: true - успешно, false - неудача

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
		unsigned long long ullElementIndexStart = 0;		//индекс элемента, с которого нужно производить поиск свободного бита в стратегии поиска элемента
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
			//устанавливаем указатель в последнем элементе списка на только что созданный
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
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this, ullElementIndexStart) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool InsertFirst(ptrListElement const pElemToInsert, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет новый элемент в начале списка
		//добавляемый элемент при этом создаётся извне, и задача функции - включить его в начале списка
		//на вход: pElemToInsert - указатель на вставляемый элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции
		//на выход: true - успешно, false - неудача

		if (pElemToInsert == nullptr)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return false;
		}

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//проверяем, является ли это первым элементом списка
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
			//устанавливаем указатель в созданном элементе списка на прежний первый элемент
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
		if (!CheckingPresenceElementPolicy<ListElement, false>::RegisterElement(pElemToInsert, this) && pErrorCode)	//регистрируем элемент в стратегии поиска элемента для облегчения его поиска в дальнейшем
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();

		return true;
	}

	bool Delete(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//удаляет переданный элемент списка, обновляя значения указателя предыдущего элемента
	{
		//удаляет переданный элемент списка, обновляя значения указателя предыдущего элемента
		//на вход: pElem - элемент списка, предназначенный для удаления; pErrorCode - указатель на возвращаемый код ошибки, bCheckPresence - флаг проверки
		//наличия элемента в списке прежде, чем его удалять (страховка для многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком),
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - успешно, false - список пуст либо ошибка

		//вызываем внутреннюю вспомогательную функцию, указав ей удалить элемент списка из памяти
		return DeleteElement_Internal(pElem, true, bCheckPresence, bProtected, pErrorCode);
	}

	bool Remove(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bCheckPresence = false, bool bProtected = false) noexcept	//исключает переданный элемент списка, обновляя значения указателя предыдущего элемента
	{
		//исключает указанный элемент из списка, сам элемент физически не удаляется из памяти
		//на вход: pElem - элемент списка, предназначенный для удаления; pErrorCode - указатель на возвращаемый код ошибки, bCheckPresence - флаг проверки
		//наличия элемента в списке прежде, чем его удалять (страховка для многопоточности, чтобы не удалить повторно элемент, удалённый другим потоком),
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - успешно, false - список пуст либо ошибка

		//вызываем внутреннюю вспомогательную функцию, указав ей исключить элемент из списка без удаления из памяти
		return DeleteElement_Internal(pElem, false, bCheckPresence, bProtected, pErrorCode);
	}

	ptrListElement Find(const ListElement& liElem, bool bProtected = false) const noexcept
	{
		//выполняет поиск элемента и возвращает указатель на него, если он найден; элемент списка должен поддерживать операцию ==
		//на вход: liElem - ссылка на интересующий элемент; выполняется поиск ПО СОДЕРЖИМОМУ, а не проверка присутствия элемента в списке (liElem в общем
		//случае не принадлежит списку), bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на элемент списка, если элемент с заданным содержимым найден, или nullptr в противном случае

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		while (pCurr)
		{
			if (*pCurr == liElem)
			{
				if (!bProtected)
					UnlockListShared();			//разблокируем список на чтение
				return pCurr;
			}
			pCurr = pCurr->pNext;
		}

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		return ptrListElement(nullptr);
	}

	unsigned long long CalculateElementsNumber(bool bProtected = false) const noexcept
	{
		//подсчитывает количество узлов в списке
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: количество элементов в списке

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullNumElements = CheckingPresenceElementPolicy<ListElement, false>::GetNumElements();			//число элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullNumElements;
	}

	unsigned long long FindElementNumber(ptrListElement const pElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) const noexcept
	{
		//ищет порядковый номер элемента в списке
		//на вход: pElem - элемент списка, номер которого надо найти; pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция
		//над элементом списка защищена извне функции
		//на выход: номер элемента в списке

		if (!pElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullElementNumber = 0;			//номер элемента
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
				UnlockListShared();			//разблокируем список на чтение
			return ListErrors::ce_ullWrongElementNumber;
		}

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		return ullElementNumber;
	}

	void MakeCopy(const List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//выполняет замещение текущего списка переданным, т.е. опустошает текущий список и создаёт копию переданного
		//на вход: list - ссылка на копируемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем копируемый список на время операции
		list.LockListShared();

		Empty(true);		//очищаем список
		ptrListElement pCurrToCopy = list.GetFirst(true);							//получаем первый элемент копируемого списка
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//указатели на текущий и предыдущий элементы (данного списка)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);				//создаём новый элемент, инициализируя его элементом копируемого списка
			if (pCurr == nullptr)									//неудача при выделении памяти - генерируем исключение
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//разблокируем копируемый список
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
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//переходим к следующему элементу копируемого списка
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем копируемый список
		list.UnlockListShared();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void MakeCopy(const List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//выполняет замещение текущего списка переданным, т.е. опустошает текущий список и создаёт копию переданного
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на копируемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем копируемый список на время операции
		list.LockListShared();

		Empty(true);		//очищаем список
		ptrListElement pCurrToCopy = list.GetFirst(true);							//получаем первый элемент копируемого списка
		ptrListElement pCurr = nullptr, pPrev = nullptr;							//указатели на текущий и предыдущий элементы (данного списка)
		while (pCurrToCopy != nullptr)
		{
			pCurr = ListElement::MemoryPolicy::Create(*pCurrToCopy);				//создаём новый элемент, инициализируя его элементом копируемого списка
			if (pCurr == nullptr)									//неудача при выделении памяти - генерируем исключение
			{
				ListBase<ListElement>::pLast = pPrev;
				if (pErrorCode)
					*pErrorCode = ListErrorCodes::eListErrorCode::FailElemCreation;
				if (!bProtected)
					UnlockListExclusive();
				//разблокируем копируемый список
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
			pCurrToCopy = list.GetNext(pCurrToCopy, true, true);	//переходим к следующему элементу копируемого списка
		}
		ListBase<ListElement>::pLast = pCurr;
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, iterator{ ListBase<ListElement>::pFirst, this });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;

		//разблокируем копируемый список
		list.UnlockListShared();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	void Empty(bool bProtected = false) noexcept
	{
		//опустошает список, физически удаляя все его элементы из памяти
		//на вход: bProtected - флаг того, что операция над элементом списка, защищена извне функции

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		//удаляем все элементы списка
		ptrListElement pCurr = ListBase<ListElement>::pFirst;
		ptrListElement pNext = nullptr;
		while (pCurr != nullptr)
		{
			pNext = pCurr->pNext;
			ListElement::MemoryPolicy::Delete(pCurr);
			pCurr = pNext;
		}
		SetEmpty(true);					//устанавливаем указатели в нулевые значения

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListAfter(List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к концу текущего, после чего опустошает (очищает) указанный список
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//продолжаем текущий список, указав на начало добавляемого
		if (list.GetFirst(true))
		{
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				list.GetFirst(true)->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				list.GetFirst(true)->pPrev = ListBase<ListElement>::pLast;
		}
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//устанавливаем последний элемент текущего списка (если добавляемый список не пуст)
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListAfter(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к концу текущего, после чего опустошает (очищает) указанный список
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		ListBase<ListElement>::pLast->pNext = list.GetFirst(true);			//продолжаем текущий список, указав на начало добавляемого
		if (list.GetFirst(true))
		{
			if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
				list.GetFirst(true)->pPrev.lock() = ListBase<ListElement>::pLast;
			else
				list.GetFirst(true)->pPrev = ListBase<ListElement>::pLast;
		}
		if (list.GetLast(true))
			ListBase<ListElement>::pLast = list.GetLast(true);				//устанавливаем последний элемент текущего списка (если добавляемый список не пуст)
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	void AddListBefore(List_TwoLinked& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к началу текущего, после чего опустошает (очищает) указанный список
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//последний элемент добавляемого теперь указывает на начало текущего
		if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			ListBase<ListElement>::pFirst->pPrev.lock() = list.GetLast(true);
		else
			ListBase<ListElement>::pFirst->pPrev = list.GetLast(true);
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, List_TwoLinked::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//устанавливаем первый элемент текущего списка (если добавляемый список не пуст)
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	template<class List> void AddListBefore(List& list, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//добавляет элементы указанного списка к началу текущего, после чего опустошает (очищает) указанный список
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над элементом
		//списка, защищена извне функции

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();
		//блокируем добавляемый список на время операции
		list.LockListExclusive();

		if (list.GetLast(true))
			list.GetLast(true)->pNext = ListBase<ListElement>::pFirst;			//последний элемент добавляемого теперь указывает на начало текущего
		if constexpr (std::is_same_v<typename ListElement::MemoryPolicy, SmartSharedPointer<ListElement>>)		//C++17: if constexpr
			ListBase<ListElement>::pFirst->pPrev.lock() = list.GetLast(true);
		else
			ListBase<ListElement>::pFirst->pPrev = list.GetLast(true);
		bool bRegisterContainer = CheckingPresenceElementPolicy<ListElement, false>::RegisterContainer(this, typename List::iterator{ list.GetFirst(true), &list });
		if (!bRegisterContainer && pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::SearchContainerElementError;
		if (list.GetFirst(true))
			ListBase<ListElement>::pFirst = list.GetFirst(true);				//устанавливаем первый элемент текущего списка (если добавляемый список не пуст)
		list.SetEmpty(true);						//очищаем добавляемый список, сбрасывая его указатели в нулевые значения

		//разблокируем добавляемый список
		list.UnlockListExclusive();
		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();
	}

	List_TwoLinked Split(ptrListElement const pSplitElem, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false) noexcept
	{
		//разбивает текущий список, отсекая его после указанного элемента
		//на вход: pSplitElem - указатель на элемент, после которого выполняется разбиение списка; pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: новый список, содержащий отсечённую часть

		if (pErrorCode)
			*pErrorCode = ListErrorCodes::eListErrorCode::Success;

		//блокируем список на время операции
		if (!bProtected)
			LockListExclusive();

		if (!pSplitElem)
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::Nullptr;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		//проверяем, является ли переданный элемент частью списка
		if (!FindElement(pSplitElem, true))
		{
			if (pErrorCode)
				*pErrorCode = ListErrorCodes::eListErrorCode::NotPartOfList;
			pSplitElem = ListBase<ListElement>::pLast;
		}

		List_TwoLinked list(CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax(), true);				//создаём локальный список, в котором будет храниться результат
		//проверяем, является ли переданный элемент последним; если нет, то разбиваем список; если да, то ничего не делаем и возвращаем
		//пустой локальный list
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

		//разблокируем список
		if (!bProtected)
			UnlockListExclusive();		

		return list;
	}

	bool IsEmpty(bool bProtected = false) const noexcept
	{
		//проверяет, является ли список пустым
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: true - список пуст, false - список НЕ пуст

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		bool bResult = false;
		if (ListBase<ListElement>::pFirst == nullptr && ListBase<ListElement>::pLast == nullptr)
			bResult = true;

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение

		return bResult;
	}

	unsigned long long GetNumElementsMax_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//возвращает максимальное количество элементов, обслуживаемое стратегией поиска элементов
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: количество элементов в списке

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullNumElementsMax = CheckingPresenceElementPolicy<ListElement, false>::GetNumElementsMax();			//максимальное количество элементов, обслуживаемое стратегией поиска элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullNumElementsMax;
	}

	unsigned long long GetCurrentElementIndex_SearchElementPolicy(bool bProtected = false) const noexcept
	{
		//возвращает текущий индекс, используемый для вновсь создаваемого элемента списка
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: количество элементов в списке

		if (!bProtected)
			LockListShared();			//блокируем список на чтение

		unsigned long long ullCurrentElementIndex = CheckingPresenceElementPolicy<ListElement, false>::GetCurrentElementIndex();			//максимальное количество элементов, обслуживаемое стратегией поиска элементов

		if (!bProtected)
			UnlockListShared();			//разблокируем список на чтение
		return ullCurrentElementIndex;
	}

	//блокировка и разблокировка списка

	void LockListExclusive(void) const noexcept				//блокирует список для изменения
	{
		LockingPolicy::LockExclusive();
	}

	void UnlockListExclusive(void) const noexcept			//разблокирует список после изменения
	{
		LockingPolicy::UnlockExclusive();
	}

	void LockListShared(void) const noexcept				//блокирует список для чтения
	{
		LockingPolicy::LockShared();
	}

	void UnlockListShared(void) const noexcept				//разблокирует список после чтения
	{
		LockingPolicy::UnlockShared();
	}

	//перемещение по узлам списка

	ptrListElement GetFirst(bool bProtected = false) const noexcept
	{
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент списка

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetFirstAndRemove(bool bProtected = false) const noexcept
	{
		//возвращает первый элемент списка и удаляет его из списка за одно действие
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент, уже не являющийся частью списка

		if (!bProtected)
			LockListShared();

		ptrListElement pFirst = ListBase<ListElement>::pFirst;
		Remove(pFirst, false, true);

		if (!bProtected)
			UnlockListShared();

		return pFirst;
	}

	ptrListElement GetNext(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const	noexcept	//возвращает указатель на следующий элемент, на который указывает переданный узел
	{
		//на вход: pCurr - указатель на элемент списка, относительно которого нужно перейти к следующему, pErrorCode - указатель на возвращаемый код ошибки,
		//bProtected - флаг того, что операция над элементом списка защищена извне функции, bSerial - флаг последовательного перебора - в этом случае не
		//вызывается функция FindElement для ускорения работы (скажем, если мы где-то последовательно проходим по списку от элемента к элементу, то нет
		//смысла искать каждый элемент заново)
		//на выход: указатель на элемент списка, расположенный после переданного в параметрах функции

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
		bool bPresent = true;		//флаг присутствия в списке переданного элемента
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//ищем элемент только в том случае, если работа функции извне не защищена или не указан флаг последовательного перебора
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

	ptrListElement GetPrev(ptrListElement const pCurr, ListErrorCodes::eListErrorCode* pErrorCode = nullptr, bool bProtected = false, bool bSerial = false) const	noexcept	//возвращает указатель на предыдущий элемент, на который указывает переданный узел
	{
		//на вход: pCurr - указатель на текущий элемент, pErrorCode - указатель на возвращаемый код ошибки, bProtected - флаг того, что операция над
		//элементом списка защищена извне функции
		//на выход: указатель на элемент, расположенный после переданного в параметрах функции

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
		bool bPresent = true;		//флаг присутствия в списке переданного элемента
		if (bProtected == false || bSerial == false)
			bPresent = FindElement(pCurr, pErrorCode, true);		//ищем элемент только в том случае, если работа функции извне не защищена или не указан флаг последовательного перебора
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
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на последний элемент списка

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	ptrListElement GetLastAndRemove(bool bProtected = false) const noexcept
	{
		//возвращает последний элемент списка и удаляет его из списка за одно действие
		//на вход: bProtected - флаг того, что операция над элементом списка защищена извне функции
		//на выход: указатель на первый элемент, уже не являющийся частью списка

		if (!bProtected)
			LockListShared();

		ptrListElement pLast = ListBase<ListElement>::pLast;
		Remove(pLast, false, true);

		if (!bProtected)
			UnlockListShared();

		return pLast;
	}

	//операции

	List_TwoLinked& operator=(List_TwoLinked& list) noexcept
	{
		//операция копирующего присваивания
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	template<class List> List_TwoLinked& operator=(List& list) noexcept
	{
		//операция копирующего присваивания
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на копируемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		MakeCopy(list);			//копируем список, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	List_TwoLinked& operator=(List_TwoLinked&& list) noexcept
	{
		//операция перемещающего присваивания
		//на вход: list - rvalue-ссылка на копируемый список (подразумевается, что список временный)
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		LockListExclusive();		//блокируем список

		Empty(true);				//очищаем текущий список
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();
		list.SetEmpty();

		UnlockListExclusive();		//разблокируем список
		return *this;
	}

	template<class List> List_TwoLinked& operator=(List&& list) noexcept
	{
		//операция перемещающего присваивания
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - rvalue-ссылка на копируемый список (подразумевается, что список временный)
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		LockListExclusive();		//блокируем список

		Empty(true);				//очищаем текущий список
		ListBase<ListElement>::pFirst = list.GetFirst();
		ListBase<ListElement>::pLast = list.GetLast();

		UnlockListExclusive();		//разблокируем список
		return *this;
	}

	List_TwoLinked& operator+=(List_TwoLinked& list) noexcept
	{
		//операция добавления списка в конец текущего с очищением добавляемого
		//на вход: list - ссылка на добавляемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		AddListAfter(list);		//добавляем список в конец текущего, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	template<class List> List_TwoLinked& operator+=(List& list) noexcept
	{
		//операция добавления списка в конец текущего с очищением добавляемого
		//шаблонная функция - может быть передан список с другими стратегиями блокировки и проверки наличия элемента: главное, чтобы класс элемента и
		//стратегии памяти в нём совпадали
		//на вход: list - ссылка на добавляемый список
		//на выход: возвращаем ссылку на текущий список, чтобы результат операции присваивания можно было использовать в другой операции

		AddListAfter(list);		//добавляем список в конец текущего, исключения не обрабатываем: они будут обрабатываться извне
		return *this;
	}

	//поддержка итераторов - используются только во вспомогательных внутренних процедурах типа стратегии проверки наличия элемента; именно поэтому функции
	//работы с ними объявлены как protected

protected:

	class ListIterator					//класс итератора для перебора элементов
	{
		ptrListElement pCurrElement{ nullptr };						//текущий элемент, на который указывает итератор
		const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//указатель на список, которому принадлежит данный элемент
		bool bProtected = true;									//флаг того, что список блокирован извне итератора

	public:

		ListIterator() {}
		ListIterator(ptrListElement pElem, const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList, bool bProtected = true)
			noexcept : pCurrElement(pElem), pList(pList), bProtected(bProtected) {}
		ListIterator(const ListIterator& li) noexcept : pCurrElement(li.pCurrElement), pList(li.pList) {}

		ptrListElement& operator*()
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			ptrListElement& pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//префиксный инкремент: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		void operator--()		//префиксный декремент: --it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			pCurrElement = pCurrElement->pPrev;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		ListIterator operator++(int) noexcept									//постфиксный инкремент: it++
		{
			ListIterator itPrev = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itPrev;
		}

		ListIterator operator--(int) noexcept									//постфиксный декремент: it--
		{
			ListIterator itCurr = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}
			pCurrElement = pCurrElement->pPrev;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itCurr;
		}

		bool operator==(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = (pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}

		bool operator!=(const ListIterator li)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = false;
			if (pList == nullptr || li.pList == nullptr)
				bResult = !(pCurrElement == li.pCurrElement);
			else
				bResult = !(pCurrElement == li.pCurrElement && pList == li.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}
	};

	class ListIteratorConst					//класс константного итератора для перебора элементов
	{
		ptrListElement pCurrElement{ nullptr };					//текущий элемент, на который указывает итератор
		const List_TwoLinked<ListElement, LockingPolicy, CheckingPresenceElementPolicy, false>* pList = nullptr;		//указатель на список, которому принадлежит данный элемент
		bool bProtected = true;									//флаг того, что список блокирован извне итератора

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
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			const ptrListElement& c_pli = pCurrElement;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return c_pli;
		}
		operator bool() noexcept { return pCurrElement != nullptr; }
		operator const ptrListElement() noexcept { return pCurrElement; }

		void operator++()		//префиксный инкремент: ++it
		{
			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}

			pCurrElement = pCurrElement->pNext;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		void operator--()		//префиксный декремент: --it
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			if (!pList->FindElement(pCurrElement, nullptr, true))
			{
				if (!bProtected)
					pList->UnlockListShared();
				throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
			}

			pCurrElement = pCurrElement->pPrev;

			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
		}

		ListIteratorConst operator++(int)			//постфиксный инкремент: it++
		{
			ListIteratorConst itPrev = *this;

			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			if (!pList->FindElement(pCurrElement, nullptr, true))
			{
				if (!bProtected)
					pList->UnlockListShared();
				throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
			}
			pCurrElement = pCurrElement->pNext;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itPrev;
		}

		ListIteratorConst operator--(int)			//постфиксный декремент: it--
		{
			ListIteratorConst itCurr = *this;

			if (!bProtected)
			{
				pList->LockListShared();		//блокируем список на время операции
				if (!pList->FindElement(pCurrElement, nullptr, true))
				{
					if (!bProtected)
						pList->UnlockListShared();
					throw ListErrors::NotPartOfList();	//генерация исключения - переданный элемент не является частью списка
				}
			}
			pCurrElement = pCurrElement->pPrev;
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции

			return itCurr;
		}

		bool operator==(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = (pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}

		bool operator!=(const ListIteratorConst lic)
		{
			if (!bProtected)
				pList->LockListShared();		//блокируем список на время операции
			bool bResult = false;
			if (pList == nullptr || lic.pList == nullptr)
				bResult = !(pCurrElement == lic.pCurrElement);
			else
				bResult = !(pCurrElement == lic.pCurrElement && pList == lic.pList);
			if (!bProtected)
				pList->UnlockListShared();		//разблокируем список после операции
			return bResult;
		}
	};

	using iterator = ListIterator;
	using const_iterator = ListIteratorConst;

	ListIterator begin(bool bProtected = false) noexcept
	{
		if(!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIterator lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return lit;
	}

	ListIterator end() noexcept
	{
		return ListIterator();
	}

	ListIteratorConst begin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return c_lit;
	}

	ListIteratorConst end() const noexcept
	{
		return ListIteratorConst();
	}

	ListIteratorConst cbegin(bool bProtected = false) const noexcept
	{
		if (!bProtected)
			LockListShared();		//блокируем список на время операции
		ListIteratorConst c_lit(ListBase<ListElement>::pFirst, this);
		if (!bProtected)
			UnlockListShared();		//разблокируем список

		return c_lit;
	}

	ListIteratorConst cend() const noexcept
	{
		return ListIteratorConst();
	}

	//привилегированные классы
	template<class ElementType, bool bExceptions> friend class DirectSearch;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2;
#ifdef _MSC_VER
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray_MemoryOnRequestLocal;
	template<class ElementType, bool bExceptions> friend class SearchByIndex_BitArray2_MemoryOnRequestLocal;
#endif

	//привилегирированные функции

	template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
		template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
		template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
	friend auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2);
};

//ФУНКЦИИ//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ListElement, class LockingPolicy1, class LockingPolicy2, template<class, bool> class CheckingPresenceElementPolicy1,
template<class, bool> class CheckingPresenceElementPolicy2, bool bExceptions1, bool bExceptions2,
template<class, class, template<class, bool> class, bool> class ListType1, template<class, class, template<class, bool> class, bool> class ListType2>
auto operator+(ListType1<ListElement, LockingPolicy1, CheckingPresenceElementPolicy1, bExceptions1>& list1, ListType2<ListElement, LockingPolicy2, CheckingPresenceElementPolicy2, bExceptions2>& list2)
{
	//операция объединения списков: она объединяет списки, переданные в параметрах, во временный список, после чего очищает исходные объединяемые списки
	//на вход: list1 и list2 - объединяемые списки

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


	//блокируем списки: здесь - потенциальная взаимная блокировка; нужен способ заблокировать оба списка одновременно за единую атомарную операцию типа std::lock(mutex1, mutex2)
	list1.LockListExclusive();
	list2.LockListExclusive();

	//создаём локальный список, в котором будет храниться результат
	ListTypeResult list(list1.GetNumElementsMax_SearchElementPolicy(true) + list2.GetNumElementsMax_SearchElementPolicy(true), false);

	//устанавливаем указатели, объединяющие списки (один или оба из них могут оказаться пустыми, поэтому нужны проверки)
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

	//регистрируем элементы списка в соответствующей стратегии поиска элемента
	list.CheckingPresenceElementPolicyResult::RegisterContainer(&list, typename ListTypeResult::iterator{ list.ListBase<ListElement>::pFirst, &list });

	//очищаем объединяемые списки, сбрасывая указатели
	list1.SetEmpty(true);
	list2.SetEmpty(true);

	//разблокируем списки
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

	//выбираем тип элемента ListElementCompound: если стратегия поиска элемента = DirectSearch, то назначаем ListElementCompound_OneLinked, для всех остальных
	//стратегий - ListElementCompound_OneLinked_CP
	//поскольку и передаваемый в параметре CheckingPresenceElementPolicy, и DirectSearch нужно инстанцировать чем-то конкретным для проведения сравнения,
	//то передаётся ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy> в качестве параметра стратегии поиска элемента
	using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_OneLinked<ElementData, _MemoryPolicy>, ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>>;
	using List = List_OneLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, false>;

	//функции

	List_OneLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//конструктор

	List_OneLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //конструктор для инициализации стратегии поиска элементов

	List_OneLinked_DataAdapter(const List_OneLinked_DataAdapter& list) : List(list) {}	//конструктор копирования

	List_OneLinked_DataAdapter(List_OneLinked_DataAdapter&& list) : List(list) {}		//конструктор перемещения: служит для создания временных объектов в выражениях
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ElementData, template<class> class _MemoryPolicy, class LockingPolicy, template<class, bool> class CheckingPresenceElementPolicy>
	class List_TwoLinked_DataAdapter<ElementData, _MemoryPolicy, LockingPolicy, CheckingPresenceElementPolicy, false> : 
	public List_TwoLinked<std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_OneLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>, LockingPolicy, CheckingPresenceElementPolicy, false>
{
public:

	//выбираем тип элемента ListElementCompound: если стратегия поиска элемента = DirectSearch, то назначаем ListElementCompound_TwoLinked, для всех остальных
	//стратегий - ListElementCompound_TwoLinked_CP
	//поскольку и передаваемый в параметре CheckingPresenceElementPolicy, и DirectSearch нужно инстанцировать чем-то конкретным для проведения сравнения,
	//то передаётся ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy> в качестве параметра стратегии поиска элемента
	using ListElementCompound = std::conditional_t<std::is_same_v<CheckingPresenceElementPolicy<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, false>, DirectSearch<ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>, false>>, ListElementCompound_TwoLinked<ElementData, _MemoryPolicy>, ListElementCompound_TwoLinked_CP<ElementData, _MemoryPolicy>>;
	using List = List_TwoLinked<ListElementCompound, LockingPolicy, CheckingPresenceElementPolicy, false>;

	//функции

	List_TwoLinked_DataAdapter(bool bTemporary = false) : List(bTemporary) {}	//конструктор

	List_TwoLinked_DataAdapter(unsigned long long ullNumElementsMax, bool bTemporary = false) : List(ullNumElementsMax, bTemporary) {} //конструктор для инициализации стратегии поиска элементов

	List_TwoLinked_DataAdapter(const List_TwoLinked_DataAdapter& list) : List(list) {}	//конструктор копирования

	List_TwoLinked_DataAdapter(List_TwoLinked_DataAdapter&& list) : List(list) {}		//конструктор перемещения: служит для создания временных объектов в выражениях
};

}		//конец определения пространства имён ListMT