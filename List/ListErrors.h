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

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <limits>

namespace ListMT						//������������ ��� �������������� ������ (������������ � �����������)
{

	namespace ListErrorCodes										
	{
		//������������ ��� ����� ������ �������������� ������ (�������� �� ListErrors, �.�. ��� eListErrorCode ������ enum class ������ ������������ enum, �.�.
		//������������ ����� ����� ����� �������� ����� ����, � �������������� �� eListErrorCode ������������� � ���������������� �������� ����������)

		enum eListErrorCode : unsigned int { Success, WrongParam, Nullptr, NotPartOfList, OutOfMemory, FailElemCreation, SearchContainerElementError };
	}

	namespace ListErrors				//������������ ��� ���������� �������������� ������
	{
#ifdef max
#undef max						//������ ���������������� ����������� ������� max �� numeric_limits, ������� ��� ������� "�������������"
#endif

		constexpr unsigned long long ce_ullWrongElementNumber = std::numeric_limits<unsigned long long>::max();

		//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//������ ��� ��������� ����������

		//���� ������:
		//1) ������� ��������� �������� - ������� ���������
		//2) ������� ��������� �������� - ���������� ������� �� �������� ������ ������
		//3) ������� ��������� ������ ��� �������� ������ ����

		class ListError {};								//����� ������ ��� ������ �������
		class WrongParam : public ListError {};			//������� �������� ��������
		class Nullptr : public WrongParam {};			//������� �������� ��������: ������� ���������
		class NotPartOfList : public WrongParam {};		//������� �������� ��������: ���������� ������� �� �������� ������ ������
		class OutOfMemory : public ListError {};		//������� ��� ��������� ������
		class FailElemCreation : public OutOfMemory {};	//������� ��� ��������� ������: �� ������� ������� ����		
	}
}