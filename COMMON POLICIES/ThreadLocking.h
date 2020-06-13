#pragma once

//������� ������ ��������, 2019 �
//������ ������������ ���� ������������� ����� ������� ��������� ��� ���������� ������� � ������������� �����. � ������� ��������� ����� ��������� � ������ ����� �����
//�������������� �. � ����������� �������������� �� �++�.
//������ ����� ���� ��������� ������ ������������� � ������������ ������� ������ �������: LockExclusive, LockShared, UnlockExclusive � UnlockShared ��� ���������� �������.
//��� ����, ���� �������� ���������� �� ������������� "������ ����������", ����������� ����������� ������ � ������� � ������ �� ������, �� ���� ������� Exclusive/Shared
//����� ������������� ���������.

//������������ � ����� �������� � ��������� ��������� ��� ��� �������: Lock � Unlock. ��� ��������� ������������ LockExclusive � UnlockExclusive � ������� ������ ��� ��������
//� ��� �������, ��� �� ������������ �����-���� "������" ����������.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER							//������������ ������� Windows
#include <Windows.h>					//�������� ���������� ���� Windows
#endif

#include <mutex>						//C++ STL: ������� ��� ���������� �������

//������///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������������� ��������� - ��� ������� ������
class NoLocking
{
public:

	NoLocking(bool) {};

	void LockExclusive(void) const noexcept {}			//��������� ���������� ������� ��� ���������
	void UnlockExclusive(void) const noexcept {}		//��������� ������������� ������� ����� ���������
	void LockShared(void) const noexcept {}				//��������� ���������� ������� ��� ������
	void UnlockShared(void) const noexcept {}			//��������� ������������� ������� ����� ������
	void Lock(void) const noexcept {}					//��������� ���������� �������
	void Unlock(void) const noexcept {}					//��������� ������������� �������
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���������� ����������� �������� ����������� ���������� C++
class ThreadLocking_STDMutex
{
	mutable std::mutex mLock;		//������� ����������� ���������� C++

public:

	ThreadLocking_STDMutex(bool bInitialize = true) {}	//�����������

	//������� ���������� �����������

	void LockExclusive(void) const noexcept				//��������� ���������� ������� ��� ���������
	{
		mLock.lock();
	}

	void UnlockExclusive(void) const noexcept			//��������� ������������� ������� ����� ���������
	{
		mLock.unlock();
	}

	void LockShared(void) const noexcept				//��������� ���������� ������� ��� ������
	{
		mLock.lock();
	}

	void UnlockShared(void) const noexcept				//��������� ������������� ������� ����� ������
	{
		mLock.unlock();
	}

	void Lock(void) const noexcept						//��������� ���������� �������
	{
		mLock.lock();
	}

	void Unlock(void) const noexcept			//��������� ������������� �������
	{
		mLock.unlock();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER							//������������ ������� Windows

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���������� ����������� ����������� ������ SRWLock
//��� ��� ����������� ������ ���� SRWLock ���-���� ������� �� �����, �� ���������� ������ �� ��������� (����������� ����� �� ��������� �� ������������� �����
//������ ����� � ������: �++11)

class ThreadLockingWin_SRWLock			//���������� ����������� ����������� ������ SRWLock
{
	mutable SRWLOCK csSRW = { 0 };	//��������� ��� ����������� ������ ���� SRWLock (C++11: ������������� ����� ������)
									//mutable ���������, ��� ��������� csSRW ����� ����������, ���� ���� ���� ������ ������ ��������� ��� �������� ��� const ��� ��
									//������ ������, � ������� ��������� ������ ���������, �������� ��� const

public:

	ThreadLockingWin_SRWLock(bool bInitialize = true)
	{
		//�����������; bInitialize - ���� ����, ��� ��������� ������������� ����������� ������
		if(bInitialize)
			InitializeSRWLock(&csSRW);		//�������������� ����������� ������ SRWLock
	}

	//������� ���������� �����������

	void LockExclusive(void) const noexcept				//��������� ���������� ������� ��� ���������
	{
		AcquireSRWLockExclusive(&csSRW);
	}

	void UnlockExclusive(void) const noexcept			//��������� ������������� ������� ����� ���������
	{
		ReleaseSRWLockExclusive(&csSRW);
	}

	void LockShared(void) const noexcept				//��������� ���������� ������� ��� ������
	{
		AcquireSRWLockShared(&csSRW);
	}

	void UnlockShared(void) const noexcept				//��������� ������������� ������� ����� ������
	{
		ReleaseSRWLockShared(&csSRW);
	}

	void Lock(void) const noexcept						//��������� ���������� �������
	{
		AcquireSRWLockExclusive(&csSRW);
	}

	void Unlock(void) const noexcept			//��������� ������������� �������
	{
		ReleaseSRWLockExclusive(&csSRW);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ThreadLockingWin_CriticalSection			//���������� ����������� ����������� ������
{
	mutable CRITICAL_SECTION cs = { 0 };	//��������� ��� ����������� ������ (C++11: ������������� ����� ������)
											//mutable ���������, ��� ��������� csSRW ����� ����������, ���� ���� ���� ������ ������ ��������� ��� �������� ��� const ��� ��
											//������ ������, � ������� ��������� ������ ���������, �������� ��� const

public:
	//������������ � �����������

	ThreadLockingWin_CriticalSection(bool bInitialize = true)
	{
		if(bInitialize)
			InitializeCriticalSection(&cs);		//����������� ������������� ����������� ������
	}

	ThreadLockingWin_CriticalSection(DWORD dwSpinCount)
	{
		InitializeCriticalSectionAndSpinCount(&cs, dwSpinCount);		//������������� ����������� ������ �� ����-�����������
	}

	~ThreadLockingWin_CriticalSection()
	{
		DeleteCriticalSection(&cs);
	}

	//������� ���������� �����������

	void LockExclusive(void) const noexcept				//��������� ���������� ������� ��� ���������
	{
		EnterCriticalSection(&cs);
	}

	void UnlockExclusive(void) const noexcept			//��������� ������������� ������� ����� ���������
	{
		LeaveCriticalSection(&cs);
	}

	void LockShared(void) const noexcept				//��������� ���������� ������� ��� ������, ���� ��� ������ ����������� ������ ��� ������������ LockExclusive
	{
		EnterCriticalSection(&cs);
	}

	void UnlockShared(void) const noexcept				//��������� ������������� ������� ����� ������
	{
		LeaveCriticalSection(&cs);
	}

	void Lock(void) const noexcept						//��������� ���������� �������
	{
		EnterCriticalSection(&cs);
	}

	void Unlock(void) const noexcept					//��������� ������������� �������
	{
		LeaveCriticalSection(&cs);
	}
};

#endif		//_MSC_VER

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//������-������������, ������������ ��������, ����������� RAII - ������ ��������� ���������� � ������������ � ������� � � �����������

//����������� ������ - ��� ����������
template<class Host, class LockingPolicy> class SingleThreaded
{

public:

	class Lock								//�����-�����������
	{
	public:
		Lock(bool) noexcept {}
		Lock(const Host& obj) noexcept {}
		~Lock() noexcept {}
	};

	using VolatileType = Host;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//���������� �� ������ ������
template<class Host, class LockingPolicy> class ClassLevelLockable
{
	static inline LockingPolicy Locker;			//���������� � �������������� ��������� ��������� ����������

public:

	class Lock								//�����-�����������
	{
		bool bLocked = true;										//����, �����������, ��� ������ ������������

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

//���������� �� ������ �������
template<class Host, class LockingPolicy> class ObjectLevelLockable
{
public:

	class Lock : public LockingPolicy								//�����-�����������
	{
		bool bLocked = true;										//����, �����������, ��� ������ ������������

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

//���������� �� ������ ������� � ���������� ���������� ������ �� ������
template<class Host, class LockingPolicy> class ObjectLevelLockable_SharedLocking
{
public:

	class LockExclusive : public LockingPolicy						//�����-�����������
	{
		bool bLocked = true;										//����, �����������, ��� ������ ������������

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

	class LockShared : public LockingPolicy								//�����-�����������
	{
		bool bLocked = true;										//����, �����������, ��� ������ ������������

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