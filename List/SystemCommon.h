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

//Common system functions declarations.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>							//основной включаемый файл Windows

namespace SystemCommon							//системные функции общего назначения
{

//ПРОТОТИПЫ ФУНКЦИЙ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MEMORYSTATUSEX GetGlobalMemoryStatus(void) noexcept;		//получает информацию о состоянии системной памяти

}