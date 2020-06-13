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

//Common system functions module.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "SystemCommon.h"						//прототипы функций

namespace SystemCommon							//системные функции общего назначения
{

//ФУНКЦИИ//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MEMORYSTATUSEX GetGlobalMemoryStatus(void) noexcept
{
	//получает информацию о состоянии системной памяти
	//на выход: системная структура с информацией о состоянии системной памяти

	MEMORYSTATUSEX mse = { 0 };			//структура для получения текущего состояния оперативной памяти
	mse.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mse);			//получаем текущее состояние оперативной памяти
	return mse;
}

}