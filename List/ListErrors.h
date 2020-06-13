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

//List error exception classes and error codes definitions.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <limits>

namespace ListMT						//пространство имён многопоточного списка (односвязного и двусвязного)
{

	namespace ListErrorCodes										
	{
		//пространство имён кодов ошибок многопоточного списка (выделено из ListErrors, т.к. для eListErrorCode вместо enum class решено использовать enum, т.к.
		//впоследствии можно будет легко добавить новые коды, а идентификаторы из eListErrorCode перекрываются с соответствующими классами исключений)

		enum eListErrorCode : unsigned int { Success, WrongParam, Nullptr, NotPartOfList, OutOfMemory, FailElemCreation, SearchContainerElementError };
	}

	namespace ListErrors				//пространство имён исключений многопоточного списка
	{
#ifdef max
#undef max						//данное макроопределение перекрывает функцию max из numeric_limits, поэтому его следует "разопределить"
#endif

		constexpr unsigned long long ce_ullWrongElementNumber = std::numeric_limits<unsigned long long>::max();

		//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//классы для генерации исключений

		//виды ошибок:
		//1) передан ошибочный параметр - нулевой указатель
		//2) передан ошибочный параметр - переданный элемент не является частью списка
		//3) неудача выделения памяти при создании нового узла

		class ListError {};								//общая ошибка при работе списков
		class WrongParam : public ListError {};			//передан неверный параметр
		class Nullptr : public WrongParam {};			//передан неверный параметр: нулевой указатель
		class NotPartOfList : public WrongParam {};		//передан неверный параметр: переданный элемент не является частью списка
		class OutOfMemory : public ListError {};		//неудача при выделении памяти
		class FailElemCreation : public OutOfMemory {};	//неудача при выделении памяти: не удалось создать узел		
	}
}