#pragma once

//Хищенко Андрей Олегович, 2019 г
//Данный заголовочный файл предоставляет набор классов стратегий для блокировки потоков в многопоточной среде. О классах стратегий можно прочитать в первой главе книги
//«Александреску А. – Современное проектирование на С++».
//Каждый класс этой стратегии должен предоставлять в обязательном порядке четыре функции: LockExclusive, LockShared, UnlockExclusive и UnlockShared для блокировки потоков.
//При этом, если средство блокировки не предоставляет "тонкую блокировку", разделяющую монопольный доступ к ресурсу и доступ на чтение, то пары функций Exclusive/Shared
//могут функционально совпадать.

//Впоследствии я решил добавить в интерфейс стратегии ещё две функции: Lock и Unlock. Они полностью эквивалентны LockExclusive и UnlockExclusive и введены просто для удобства
//в тех классах, где не используется какая-либо "тонкая" блокировка.

//ВКЛЮЧАЕМЫЕ ФАЙЛЫ/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER							//операционная система Windows
#include <Windows.h>					//основной включаемый файл Windows
#endif

#include <mutex>						//C++ STL: мьютекс для блокировки потоков

//КЛАССЫ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//неблокирующая стратегия - все функции пустые
class NoLocking
{
public:

	NoLocking(bool) {};

	void LockExclusive(void) const noexcept {}			//выполняет блокировку ресурса для изменения
	void UnlockExclusive(void) const noexcept {}		//выполняет разблокировку ресурса после изменения
	void LockShared(void) const noexcept {}				//выполняет блокировку ресурса для чтения
	void UnlockShared(void) const noexcept {}			//выполняет разблокировку ресурса после чтения
	void Lock(void) const noexcept {}					//выполняет блокировку ресурса
	void Unlock(void) const noexcept {}					//выполняет разблокировку ресурса
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//блокировка посредством мьютекса стандартной библиотеки C++
class ThreadLocking_STDMutex
{
	mutable std::mutex mLock;		//мьютекс стандартной библиотеки C++

public:

	ThreadLocking_STDMutex(bool bInitialize = true) {}	//конструктор

	//функции управления блокировкой

	void LockExclusive(void) const noexcept				//выполняет блокировку ресурса для изменения
	{
		mLock.lock();
	}

	void UnlockExclusive(void) const noexcept			//выполняет разблокировку ресурса после изменения
	{
		mLock.unlock();
	}

	void LockShared(void) const noexcept				//выполняет блокировку ресурса для чтения
	{
		mLock.lock();
	}

	void UnlockShared(void) const noexcept				//выполняет разблокировку ресурса после чтения
	{
		mLock.unlock();
	}

	void Lock(void) const noexcept						//выполняет блокировку ресурса
	{
		mLock.lock();
	}

	void Unlock(void) const noexcept			//выполняет разблокировку ресурса
	{
		mLock.unlock();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER							//операционная система Windows

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//блокировка посредством критической секции SRWLock
//так как критическую секцию типа SRWLock как-либо удалять не нужно, то деструктор классу не требуется (конструктор также не требуется из инициализации члена
//класса прямо в классе: С++11)

class ThreadLockingWin_SRWLock			//блокировка посредством критической секции SRWLock
{
	mutable SRWLOCK csSRW = { 0 };	//структура для критической секции типа SRWLock (C++11: инициализация члена класса)
									//mutable указывает, что структура csSRW может изменяться, даже если весь объект класса стратегии был определён как const или же
									//объект класса, в котором применена данная стратегия, определён как const

public:

	ThreadLockingWin_SRWLock(bool bInitialize = true)
	{
		//конструктор; bInitialize - флаг того, что требуется инициализация критической секции
		if(bInitialize)
			InitializeSRWLock(&csSRW);		//инициализируем критическую секцию SRWLock
	}

	//функции управления блокировкой

	void LockExclusive(void) const noexcept				//выполняет блокировку ресурса для изменения
	{
		AcquireSRWLockExclusive(&csSRW);
	}

	void UnlockExclusive(void) const noexcept			//выполняет разблокировку ресурса после изменения
	{
		ReleaseSRWLockExclusive(&csSRW);
	}

	void LockShared(void) const noexcept				//выполняет блокировку ресурса для чтения
	{
		AcquireSRWLockShared(&csSRW);
	}

	void UnlockShared(void) const noexcept				//выполняет разблокировку ресурса после чтения
	{
		ReleaseSRWLockShared(&csSRW);
	}

	void Lock(void) const noexcept						//выполняет блокировку ресурса
	{
		AcquireSRWLockExclusive(&csSRW);
	}

	void Unlock(void) const noexcept			//выполняет разблокировку ресурса
	{
		ReleaseSRWLockExclusive(&csSRW);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ThreadLockingWin_CriticalSection			//блокировка посредством критической секции
{
	mutable CRITICAL_SECTION cs = { 0 };	//структура для критической секции (C++11: инициализация члена класса)
											//mutable указывает, что структура csSRW может изменяться, даже если весь объект класса стратегии был определён как const или же
											//объект класса, в котором применена данная стратегия, определён как const

public:
	//конструкторы и деструкторы

	ThreadLockingWin_CriticalSection(bool bInitialize = true)
	{
		if(bInitialize)
			InitializeCriticalSection(&cs);		//стандартная инициализация критической секции
	}

	ThreadLockingWin_CriticalSection(DWORD dwSpinCount)
	{
		InitializeCriticalSectionAndSpinCount(&cs, dwSpinCount);		//инициализация критической секции со спин-блокировкой
	}

	~ThreadLockingWin_CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

	//функции управления блокировкой

	void LockExclusive(void) const noexcept				//выполняет блокировку ресурса для изменения
	{
		EnterCriticalSection(&cs);
	}

	void UnlockExclusive(void) const noexcept			//выполняет разблокировку ресурса после изменения
	{
		LeaveCriticalSection(&cs);
	}

	void LockShared(void) const noexcept				//выполняет блокировку ресурса для чтения, хотя для данной критической секции она эквивалентна LockExclusive
	{
		EnterCriticalSection(&cs);
	}

	void UnlockShared(void) const noexcept				//выполняет разблокировку ресурса после чтения
	{
		LeaveCriticalSection(&cs);
	}

	void Lock(void) const noexcept						//выполняет блокировку ресурса
	{
		EnterCriticalSection(&cs);
	}

	void Unlock(void) const noexcept					//выполняет разблокировку ресурса
	{
		LeaveCriticalSection(&cs);
	}
};

#endif		//_MSC_VER

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//классы-блокировщики, использующие механизм, аналогичный RAII - объект выполняет блокировку в конструкторе и снимает её в деструкторе

//однопоточая модель - нет блокировки
template<class Host, class LockingPolicy> class SingleThreaded
{

public:

	class Lock								//класс-блокировщик
	{
	public:
		Lock(bool) noexcept {}
		Lock(const Host& obj) noexcept {}
		~Lock() noexcept {}
	};

	using VolatileType = Host;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//блокировка на уровне класса
template<class Host, class LockingPolicy> class ClassLevelLockable
{
	static inline LockingPolicy Locker;			//блокировка с использованием выбранной стратегии блокировки

public:

	class Lock								//класс-блокировщик
	{
		bool bLocked = true;										//флаг, указывающий, что объект заблокирован

	public:

		Lock(bool bProtected = false) : bLocked(!bProtected)
		{
			if (!bProtected)
				Locker.LockExclusive();
		}

		Lock(const Host& obj) : Lock() {}

		~Lock() noexcept
		{
			if (bLocked)
				Locker.UnlockExclusive();
		}
	};

	using VolatileType = volatile Host;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//блокировка на уровне объекта
template<class Host, class LockingPolicy> class ObjectLevelLockable
{
public:

	class Lock : public LockingPolicy								//класс-блокировщик
	{
		bool bLocked = true;										//флаг, указывающий, что объект заблокирован

	public:

		Lock(bool bProtected = false) : bLocked(!bProtected)
		{
			if (!bProtected)
				LockingPolicy::LockExclusive();
		}

		Lock(const Host& obj) : Lock() {}

		~Lock() noexcept
		{
			if(bLocked)
				LockingPolicy::UnlockExclusive();
		}
	};

	using VolatileType = volatile Host;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//блокировка на уровне объекта с поддержкой блокировки только на чтение
template<class Host, class LockingPolicy> class ObjectLevelLockable_SharedLocking
{
public:

	class LockExclusive : public LockingPolicy						//класс-блокировщик
	{
		bool bLocked = true;										//флаг, указывающий, что объект заблокирован

	public:

		LockExclusive(bool bProtected = false) : bLocked(!bProtected)
		{
			if (!bProtected)
				LockingPolicy::LockExclusive();
		}

		LockExclusive(const Host& obj) : LockExclusive() {}

		~LockExclusive() noexcept
		{
			if (bLocked)
				LockingPolicy::UnlockExclusive();
		}
	};

	class LockShared : public LockingPolicy								//класс-блокировщик
	{
		bool bLocked = true;										//флаг, указывающий, что объект заблокирован

	public:

		LockShared(bool bProtected = false) : bLocked(!bProtected)
		{
			if (!bProtected)
				LockingPolicy::LockShared();
		}

		LockShared(const Host& obj) : LockShared() {}

		~LockShared() noexcept
		{
			if (bLocked)
				LockingPolicy::UnlockShared();
		}
	};

	using VolatileType = volatile Host;
};