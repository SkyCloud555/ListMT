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

//Memory policies header.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <memory>				//���������������� ���������

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ������ � ������� ��� ������ ���������� ���������� C/�++////////////////////////////////////////////////////////////////////////////////

//���������� ������ �� ���������

template<class Type> class DefaultAllocator_InternalPointer
{
public:
	
	template<class... ArgTypes> static Type* Create(ArgTypes... Args) noexcept		//��������� ��������� ������ ��� �������
	{
		return new Type(Args...);
	}
};

template<class Type> class DefaultAllocator_InternalPointer<Type[]>		//��������� ������������� ��� ��������
{
public:

	static Type* Create(size_t size) noexcept		//��������� ��������� ������ ��� �������
	{
		if (size != 0)
			return new Type[size];
		else
			return nullptr;
	}
};

//��������� �� ���������

template<class Type> class DefaultDeleter_InternalPointer
{
public:

	void operator()(Type* ptr) noexcept				//��� ����, ����� ����� ���� ������������ � ���������� ������� ���� unique_ptr
	{
		Delete(ptr);
	}

	static void Delete(Type *ptr) noexcept			//��������� �������� ������� �� ������
	{
		if(ptr)
			delete ptr;		
	}
};

template<class Type> class DefaultDeleter_InternalPointer<Type[]>		//��������� ������������� ��� ��������
{
public:

	void operator()(Type* ptr) noexcept				//��� ����, ����� ����� ���� ������������ � ���������� ������� ���� unique_ptr
	{
		Delete(ptr);
	}

	static void Delete(Type *ptr) noexcept			//��������� �������� ������� �� ������
	{
		if(ptr)
			delete[] ptr;		
	}
};

//��� �������, ��� �������� �������� ���������, ��������� � ��������� Type �������
//Deleter � Allocator - ��� ��������� � ���������� ������ ��� �������� ���� Type
template<class Type, class _Deleter = DefaultDeleter_InternalPointer<Type>, class _Allocator = DefaultAllocator_InternalPointer<Type>> class InternalPointer
{
public:

	//���������� �����

	using ptrType = Type * ;			//��� ��������� �� ��� �������
	using Allocator = _Allocator;		//���������� ������
	using Deleter = _Deleter;			//���������

	using shared_ptrType = ptrType;		//��� ��������� ���������������� ����������
	using weak_ptrType = ptrType;		//��� ��������� ���������������� ����������

	//�������

	template<class... ArgTypes> static ptrType Create(ArgTypes... Args) noexcept
	{
		//������� ������ ������ � ���������� ��������� �� ����
		//�� ����: Args - ���������-��������� ��� ������������ ������� ���� Type
		//�� �����: ��������� �� ��������� ������ ���� Type

		return Allocator::Create(Args...);		//������ ������ � ������� ����������� ���������� ������
	}

	static void Delete(ptrType& pObject) noexcept
	{
		//������� ������ �� ������, ������������ ��������� �� ���� � ������� ��������
		//�� ����: pObject - ������ �� ��������� ������� ��� ��������, DeleterFunc - ��������� �� �������-���������

		if (pObject)
		{
			Deleter::Delete(pObject);			//������� ������ � ������� ����������� ���������
			pObject = nullptr;
		}
	}

	static ptrType MakePointer(Type& obj) noexcept
	{
		//���������� ��������� ��������� ���� �� ������������ ������
		//�� �����: ��������� �� ������ ���� Type

		return &obj;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//��������� ������ � ������� ��� ������ ���������������� ���������� shared_ptr/weak_ptr �++////////////////////////////////////////////////////////

//���������� ������ �� ���������

template<class Type> class DefaultAllocator_SmartSharedPointer
{
public:

	template<class... ArgTypes> static std::shared_ptr<Type> Create(ArgTypes... Args) noexcept			//��������� ��������� ������ ��� �������
	{
		return std::make_shared<Type>(Args...);
	}
};

template<class Type> class DefaultAllocator_SmartSharedPointer<Type[]>		//��������� ������������� ��� ��������
{
public:

	static std::shared_ptr<Type> Create(size_t size) noexcept		//��������� ��������� ������ ��� �������
	{
		if (size != 0)
			return std::make_shared<Type[]>(size);
		else
			return nullptr;
	}
};

//��������� �� ���������

template<class Type> class DefaultDeleter_SmartSharedPointer
{
public:

	void operator()(std::shared_ptr<Type>* ptr) noexcept				//��� ����, ����� ����� ���� ������������ � ���������� ������� ���� unique_ptr
	{
		Delete(*ptr);
	}

	static void Delete(std::shared_ptr<Type>& ptr) noexcept			//��������� �������� ������� �� ������
	{
		ptr = nullptr;									//���������� ���������,��� �������� � �������� �������, ���� ��� ������ ��������
	}
};

template<class Type> class DefaultDeleter_SmartSharedPointer<Type[]>		//��������� ������������� ��� ��������
{
public:

	void operator()(std::shared_ptr<Type>* ptr) noexcept				//��� ����, ����� ����� ���� ������������ � ���������� ������� ���� unique_ptr
	{
		Delete(*ptr);
	}

	static void Delete(std::shared_ptr<Type>& ptr) noexcept			//��������� �������� ������� �� ������
	{
		ptr = nullptr;									//���������� ���������,��� �������� � �������� �������, ���� ��� ������ ��������
	}
};

//��� �������, ��� �������� �������� ���������, ��������� � ��������� Type �������
//Deleter � Allocator - ��� ��������� � ���������� ������ ��� �������� ���� Type
template<class Type, class _Deleter = DefaultDeleter_SmartSharedPointer<Type>, class _Allocator = DefaultAllocator_SmartSharedPointer<Type>> class SmartSharedPointer
{
public:

	//���������� �����

	using ptrType = std::shared_ptr<Type>;			//��� ��������� �� ��� �������
	using Allocator = _Allocator;					//���������� ������
	using Deleter = _Deleter;						//���������

	using shared_ptrType = ptrType;					//��������� ��� ����������� ��������
	using weak_ptrType = std::weak_ptr<Type>;		//������ ���������

	//�������

	template<class... ArgTypes> static ptrType Create(ArgTypes... Args) noexcept
	{
		//������� ������ ������ � ���������� ��������� �� ����
		return Allocator::Create(Args...);		//������ ������ � ������� ����������� ���������� ������
	}

	static void Delete(ptrType& pObject) noexcept
	{
		//������������� ��������� �� ������ � ������� ��������; ���� ��� - ��������� ���������, ������� ������� ���� ������, �� ������ ����� �����
		//�� ������
		Deleter::Delete(pObject);			//������� ������ � ������� ����������� ���������
	}

	static ptrType MakePointer(Type& obj) noexcept
	{
		//���������� ��������� ��������� ���� �� ������������ ������
		//�� �����: ��������� �� ������ ���� Type

		return std::shared_ptr<Type>(&obj);
	}
};