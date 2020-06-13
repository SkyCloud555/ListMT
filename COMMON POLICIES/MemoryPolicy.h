#pragma once

//Хищенко Андрей Олегович, 2019 г
//Заголовочный файл для классов стратегий управления памятью и работы с указателями. О классах стратегий можно прочитать в первой главе книги
//«Александреску А. – Современное проектирование на С++».

//Данная стратегия имеет ограничения и применима только к работы с единичными (!!!) объектами, не массивами.
//Каждый класс стратегии предоставляет тип ptrType, задающий тип указателя, а также статические функции Create и Delete для создания и удаления объекта
//заданного типа.

//Аналогично, удалитель Deleter - это класс, предоставляющий статическую функцию Delete.
//Выделитель Allocator - это класс, предоставляющий статическую функцию Create.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <memory>				//интеллектуальные указатели

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия работы с памятью при помощи встроенных указателей C/С++////////////////////////////////////////////////////////////////////////////////

//выделитель памяти по умолчанию

template<class Type> class DefaultAllocator_InternalPointer
{
public:
	
	template<class... ArgTypes> static Type* Create(ArgTypes... Args) noexcept		//выполняет выделение памяти для объекта
	{
		return new Type(Args...);
	}
};

template<class Type> class DefaultAllocator_InternalPointer<Type[]>		//частичная специализация для массивов
{
public:

	static Type* Create(size_t size) noexcept		//выполняет выделение памяти для массива
	{
		if (size != 0)
			return new Type[size];
		else
			return nullptr;
	}
};

//удалитель по умолчанию

template<class Type> class DefaultDeleter_InternalPointer
{
public:

	void operator()(Type* ptr) noexcept				//для того, чтобы можно было использовать в удалителях классов типа unique_ptr
	{
		Delete(ptr);
	}

	static void Delete(Type *ptr) noexcept			//выполняет удаление объекта из памяти
	{
		if(ptr)
			delete ptr;		
	}
};

template<class Type> class DefaultDeleter_InternalPointer<Type[]>		//частичная специализация для массивов
{
public:

	void operator()(Type* ptr) noexcept				//для того, чтобы можно было использовать в удалителях классов типа unique_ptr
	{
		Delete(ptr);
	}

	static void Delete(Type *ptr) noexcept			//выполняет удаление массива из памяти
	{
		if(ptr)
			delete[] ptr;		
	}
};

//тип объекта, для которого создаётся указатель, передаётся в параметре Type шаблона
//Deleter и Allocator - это удалители и выделители памяти для объектов типа Type
template<class Type, class _Deleter = DefaultDeleter_InternalPointer<Type>, class _Allocator = DefaultAllocator_InternalPointer<Type>> class InternalPointer
{
public:

	//объявления типов

	using ptrType = Type * ;			//тип указателя на тип объекта
	using Allocator = _Allocator;		//выделитель памяти
	using Deleter = _Deleter;			//удалитель

	using shared_ptrType = ptrType;		//для поддержки интеллектуальных указателей
	using weak_ptrType = ptrType;		//для поддержки интеллектуальных указателей

	//функции

	template<class... ArgTypes> static ptrType Create(ArgTypes... Args) noexcept
	{
		//функция создаёт объект и возвращает указатель на него
		//на вход: Args - аргументы-параметры для конструктора объекта типа Type
		//на выход: указатель на созданный объект типа Type

		return Allocator::Create(Args...);		//создаём объект с помощью переданного выделителя памяти
	}

	static void Delete(ptrType& pObject) noexcept
	{
		//удаляет объект из памяти, устанавливая указатель на него в нулевое значение
		//на вход: pObject - ссылка на указатель объекта для удаления, DeleterFunc - указатель на функцию-удалитель

		if (pObject)
		{
			Deleter::Delete(pObject);			//удаляем объект с помощью переданного удалителя
			pObject = nullptr;
		}
	}

	static ptrType MakePointer(Type& obj) noexcept
	{
		//возвращает указатель заданного типа на существующий объект
		//на выход: указатель на объект типа Type

		return &obj;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//стратегия работы с памятью при помощи интеллектуальных указателей shared_ptr/weak_ptr С++////////////////////////////////////////////////////////

//выделитель памяти по умолчанию

template<class Type> class DefaultAllocator_SmartSharedPointer
{
public:

	template<class... ArgTypes> static std::shared_ptr<Type> Create(ArgTypes... Args) noexcept			//выполняет выделение памяти для объекта
	{
		return std::make_shared<Type>(Args...);
	}
};

template<class Type> class DefaultAllocator_SmartSharedPointer<Type[]>		//частичная специализация для массивов
{
public:

	static std::shared_ptr<Type> Create(size_t size) noexcept		//выполняет выделение памяти для массива
	{
		if (size != 0)
			return std::make_shared<Type[]>(size);
		else
			return nullptr;
	}
};

//удалитель по умолчанию

template<class Type> class DefaultDeleter_SmartSharedPointer
{
public:

	void operator()(std::shared_ptr<Type>* ptr) noexcept				//для того, чтобы можно было использовать в удалителях классов типа unique_ptr
	{
		Delete(*ptr);
	}

	static void Delete(std::shared_ptr<Type>& ptr) noexcept			//выполняет удаление объекта из памяти
	{
		ptr = nullptr;									//сбрасываем указатель,что приводит к удалению объекта, если нет других объектов
	}
};

template<class Type> class DefaultDeleter_SmartSharedPointer<Type[]>		//частичная специализация для массивов
{
public:

	void operator()(std::shared_ptr<Type>* ptr) noexcept				//для того, чтобы можно было использовать в удалителях классов типа unique_ptr
	{
		Delete(*ptr);
	}

	static void Delete(std::shared_ptr<Type>& ptr) noexcept			//выполняет удаление массива из памяти
	{
		ptr = nullptr;									//сбрасываем указатель,что приводит к удалению объекта, если нет других объектов
	}
};

//тип объекта, для которого создаётся указатель, передаётся в параметре Type шаблона
//Deleter и Allocator - это удалители и выделители памяти для объектов типа Type
template<class Type, class _Deleter = DefaultDeleter_SmartSharedPointer<Type>, class _Allocator = DefaultAllocator_SmartSharedPointer<Type>> class SmartSharedPointer
{
public:

	//объявления типов

	using ptrType = std::shared_ptr<Type>;			//тип указателя на тип объекта
	using Allocator = _Allocator;					//выделитель памяти
	using Deleter = _Deleter;						//удалитель

	using shared_ptrType = ptrType;					//указатель для совместного владения
	using weak_ptrType = std::weak_ptr<Type>;		//слабый указатель

	//функции

	template<class... ArgTypes> static ptrType Create(ArgTypes... Args) noexcept
	{
		//функция создаёт объект и возвращает указатель на него
		return Allocator::Create(Args...);		//создаём объект с помощью переданного выделителя памяти
	}

	static void Delete(ptrType& pObject) noexcept
	{
		//устанавливаем указатель на объект в нулевое значение; если это - последний указатель, которым владеет этот объект, то объект будет удалён
		//из памяти
		Deleter::Delete(pObject);			//удаляем объект с помощью переданного удалителя
	}

	static ptrType MakePointer(Type& obj) noexcept
	{
		//возвращает указатель заданного типа на существующий объект
		//на выход: указатель на объект типа Type

		return std::shared_ptr<Type>(&obj);
	}
};