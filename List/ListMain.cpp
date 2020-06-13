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

//Main test program module.

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>					//�������� ���������� ���� Windows
#include <tchar.h>						//��������� �������� Unicode
#include <process.h>					//��������� ��������������� ��� ���������� �/C++
#include <random>						//��������� �����
#include <vector>						//������������ ������ STL: ��� �������� ��������� ������������������
#include <algorithm>					//���������: ��� ������������� ����������� ���������
#include <fstream>						//��� ��������� �����-������
#include <iomanip>						//������������ �������
#include <mmsystem.h>					//�������������� ������
#include <commctrl.h>					//�������� ���������� ������ �����������
#include "ListDataExample.h"			//����� ������ � �������
#include "resource.h"					//����������� ��������

//���������� ���������/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned __int64 c_u64NumNodesInitial = 10000;			//�������������� ���������� ����� ��� �������� ������
const unsigned short int c_usiBufferSize = 256;					//����� ���������������� ���������� ������
const unsigned short int c_usiNumThreadsMax = 256;				//������������ ���������� �������

//��������� ������ ������� �������
const unsigned int c_uiNumSteps = 100;							//�������� ��������
const unsigned int c_uiSleepPeriodMax = 5;						//����� ������� "���������" �������
const unsigned int c_uiRecalculateNodesNumber = 500;			//�������� ��� ����������� ����������� ����, ��� ����� ����� ������������� ����� ����� ������

constexpr unsigned long long ce_ullMaxNumElementsInList = 2000000;	//��������� ������������������: ���������� ��������� ������, ��������� ������� ��������� ���������������
constexpr bool ce_bPerformanceMeasure = false;						//���� ��������� �����������������
constexpr unsigned int ce_uiNumSecondsMeasureMax = 2400;			//������ �� ������� ��������� � ��������

//���������� �����/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using PTHREAD_START = unsigned(__stdcall *)(void *);		//������ ���� ��� �������� ��������� ������� ������ ��� ������ _beginthreadex

//���������� ����������////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned __int64 u64NumNodesInitial = c_u64NumNodesInitial;		//�������������� ���������� ����� ��� �������� ������
unsigned short int usiNumThreads = 0;							//����� �������
unsigned __int64 u64SearchValue = 0;							//�������� ��� ������ � ����

HWND hMainDlg = NULL;											//���������� �������� ���� �������
bool bOneLinked = true;											//���� ����, ����� ������ ��������
//bool bErrorExceptions = true;									//���� ����, ��� ��� ��������� ������ ����������� ����������, � �� ������� ���� ������
HANDLE *hThreads = nullptr;										//��������� �������
HANDLE hStopEvent = NULL;										//��������� ������� ��� ������ ������������ �������
HANDLE hUserTaskEvent = NULL;									//��������� ������� ��� ������������ �� �������� ������������
volatile unsigned int uiNumNodes = 0;							//���������� ��������� ������
volatile bool bExit = false;									//���� ���������� ������
CRITICAL_SECTION csUIUpdate = { 0 };							//����������� ������ ��� ���������� ���������� � ���� �������
UINT idMMTimer = NULL;											//������������� ��������������� �������
volatile unsigned long ulCyclesPerSecond = 0;					//���������� ������ � ������� �� ���� ������� ��� ������ �������� ������ ������

//������� ������� �������
//������ ��������������� ���� ��� ��������
//using ListElementData_1L = ListElementData_OneLinked;				//��� ���� ������������ ������
using ListElementData_1L = ListElementData_OneLinked2_CP<>;		//��� ���� ������������ ������ (������������ ������������ ������� ���������)
//using ListElementData_1L = ListElementData_OneLinked_CP;			//��� ���� ������������ ������ (����������� ������������ ������� ���������)
//using ListElementData_2L = ListElementData_TwoLinked;				//��� ���� ����������� ������
//using ListElementData_2L = ListElementData_TwoLinked_CP;			//��� ���� ����������� ������
using ListElementData_2L = ListElementData_TwoLinked2_CP<>;			//��� ���� ����������� ������ (������������ ������������ ������� ���������)

constexpr bool bUseExceptions = false;										//���� ������������� ������ � ������������ ��� ���
//constexpr bool bUseExceptions = true;										//���� ������������� ������ � ������������ ��� ���

using LockingPolicy = ThreadLockingWin_SRWLock;
template<class ListElement, bool bUseExceptions> using SearchElementPolicy = SearchByIndex_BitArray2_MemoryOnRequestLocal<ListElement, bUseExceptions>;
//template<class ListElement, bool bUseExceptions> using SearchElementPolicy = DirectSearch<ListElement, bUseExceptions>;

constexpr unsigned long long ce_ullNumElementsMax = sizeof(unsigned long long) * 1024ULL * 1024ULL * 256ULL;
//ListData<List_OneLinked<ListElementData_1L, LockingPolicy, SearchElementPolicy>> listOneLinked;			//����������� ������
//ListData<List_TwoLinked<ListElementData_2L, LockingPolicy, SearchElementPolicy>> listTwoLinked;			//���������� ������
ListData<List_OneLinked<ListElementData_1L, LockingPolicy, SearchElementPolicy, bUseExceptions>, bUseExceptions> listOneLinked(ce_ullNumElementsMax);			//����������� ������
ListData<List_TwoLinked<ListElementData_2L, LockingPolicy, SearchElementPolicy, bUseExceptions>, bUseExceptions> listTwoLinked(ce_ullNumElementsMax);			//���������� ������

std::vector<std::pair<unsigned long long, unsigned long>> vPerformanceMarks;		//������� ��������� ���������� ��������� ������ � ����� ������ � ������� �� ������ �������
bool bMeasureFinished = false;							//����, ��� ��������� ������������������ ����������� �� ���������� ������� ���������� ��������� ������

//�������//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK MMTimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	//���������� ��������� �� ��������������� �������; ������ ������������ ��� ������ ���������� ������������ ����� � �������

	static unsigned int uiNumSeconds = 0;

	if (ce_bPerformanceMeasure == true && bMeasureFinished == false)
	{
		auto ullNumElements = bOneLinked ? listOneLinked.CalculateElementsNumber() : listTwoLinked.CalculateElementsNumber();
		if (ce_ullMaxNumElementsInList != 0 && (ullNumElements >= ce_ullMaxNumElementsInList || uiNumSeconds >= ce_uiNumSecondsMeasureMax))
		{
			InterlockedExchange8(reinterpret_cast<volatile CHAR*>(&bExit), true);		//������ �������� � ���������� ������ �������
			SendMessage(hMainDlg, WM_COMMAND, IDCANCEL, 0);
			bMeasureFinished = true;
			return;
		}

		if (ullNumElements > 0)
		{
			uiNumSeconds++;
			vPerformanceMarks.push_back(std::make_pair(ullNumElements, ulCyclesPerSecond));
		}
	}

	static TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
	_stprintf_s(szTextBuffer, TEXT("%u"), ulCyclesPerSecond);
	SetDlgItemText(hMainDlg, IDC_ELEMENTS_PER_SECOND, szTextBuffer);
	InterlockedExchange((LONG *)&ulCyclesPerSecond, 0);	
}

template<class List> void CalculateListSizeAndAverage(List& list, bool bUserRequest = false)
{
	//��������������� ��������� �������, ������� ���������� ���������� ����� ������ � ������� ��������� � ��������������� �������� ���������� �������
	//�� ����: list - ������ �� ������, ��� �������� ����� ���������� ���������� ����� (��� ��� ��������� �� ���������), bUserRequest - 
	//���� ����, ��� ��� ������� ������� �� ������� ������������, � ������, ��������� �������� ����� �������� � ������� ��� � ����
	
	//���������� �������� ���������� ��� ������������� ����������� ������ ��� ������ ������� �� ����������� ���� (�.�. � ��������� ������)
	//�����, ���������� ����� ���� ���������: ������� ����� ������ � ����������� ����� � ������ ����� SetDlgItemText, ������ ����������� �
	//������
	//� ��� �����, ����� ����� ������� ��������� ������� ������� SetDlgItemText, ��������� ����� ������ � ��� �� ������� � ����������� ��
	//����������� ������; �� ��������� ����� ��� ����������� �� SetDlgItemText, ������� ����� ���� �������� ������ ���������, � �� ���
	//������������! ���������� �������� ����������
	//� ���������� � ��������� �� ����������� ������ �����: �� ����� ��� ������ ������ ������ ��������� � ������� ���������� ������, � ������,
	//��� ����� � �������������� �������������

	TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����

	auto time = GetTickCount();					//������ ������ �������
	auto ullNumNodesLocal = list.CalculateElementsNumber();		//���������� �����
	unsigned __int64 u64AverageValue = list.ComputeAverageValue();		//������� ��������
	if (bUserRequest)
	{
		double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
		_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
		SetDlgItemText(hMainDlg, IDC_TIME, szTextBuffer);
		SetDlgItemText(hMainDlg, IDC_STATUS, TEXT("Defined the number of elements and average value of list elements."));
	}
	InterlockedExchange(&uiNumNodes, ullNumNodesLocal);
	_stprintf_s(szTextBuffer, TEXT("%u"), ullNumNodesLocal);
	//EnterCriticalSection(&csUIUpdate);
	SetDlgItemText(hMainDlg, IDC_NUM_NODES, szTextBuffer);
	_stprintf_s(szTextBuffer, TEXT("%I64u"), u64AverageValue);
	SetDlgItemText(hMainDlg, IDC_AVERAGE_VALUE, szTextBuffer);
	//LeaveCriticalSection(&csUIUpdate);

	auto ullNumElementsListMax = list.GetNumElementsMax_SearchElementPolicy();
	if (ullNumElementsListMax != 0)
	{
		auto ullCurrentElementIndex = list.GetCurrentElementIndex_SearchElementPolicy();
		SendDlgItemMessage(hMainDlg, IDC_BIT_ARRAY_PROGRESS, PBM_SETPOS, LOWORD(static_cast<double>(ullCurrentElementIndex) / static_cast<double>(ullNumElementsListMax) * 100.0), 0);
	}
}

//��������� ������� ��������� ������, ������������ ������ �� �������: ���������� � �������� ���������
//��������� �������� ������ ������ ������ � ��� ���������

template<class List, class ListElement> DWORD WINAPI WorkingThreadProc(PVOID pvParam)
{
	//������ ������ ������� ������ ���������� ����������: ��� ������� ������ ���������� ��������� ����, � ����� ����������� ����
	//� ������ �������� ����� ������������ ������� �� ������ ����� ��� ����� �� ����������� ���������� �����, � ����� ���� ���������� ����������
	//������ ���� � ����� ��������, ���� �������� ����, � �������� ��������� �������; ����� - ����� �������� �����
	//��� ����������� ������������ � ����������� ������

	//C++ STD RANDOM////////////////////////////
	std::default_random_engine dre(GetTickCount() + rand());
	std::uniform_int_distribution<int> diSteps(0, c_uiNumSteps - 1);
	std::uniform_int_distribution<int> diChoice(0, 1);
	std::uniform_int_distribution<int> diValue(0, RAND_MAX - 1);
	////////////////////////////////////////////

	using ptrListElement = typename List::ptrListElement;		//��������� ��� ��������� �� ������ ������

	List* pList = reinterpret_cast<List *>(pvParam);		//�������� ��������� �� ������ ������, ���������� ����� �������� �������
	List& list = *pList;						//������ ������ �� ������ ��� ��������

	//���������� ��������� ��������� � ������
	unsigned __int64 u64CurrentNodeNumber = (unsigned __int64)(double(rand()) / double(RAND_MAX) * u64NumNodesInitial);
	ptrListElement pCurr = list.GetFirst();			//������� �������, � ������� ������������ ������
	ptrListElement pNext = nullptr;					//��������� ������� ������
	ptrListElement pPrev = nullptr;					//���������� ������� ������
	//��������� �� ��������� �������
	try
	{
		for (unsigned __int64 i = 0; i < u64CurrentNodeNumber; i++)
		{
			pPrev = pCurr;
			pCurr = list.GetNext(pCurr);
		}
	}
	catch (typename ListErrors::NotPartOfList)
	{
		pCurr = pPrev;
	}

	unsigned int uiNumSteps = 100;						//�������� �������� �� ������
	bool bForward = true;								//���� ����������� ��������
	while (!bExit)
	{
		WaitForSingleObject(hStopEvent, INFINITE);		//������� �� ������� ������������

		if (pCurr == nullptr || (pCurr != nullptr && list.FindElement(pCurr) == false))
			pCurr = list.GetFirst();
		uiNumSteps = diSteps(dre);		
		try
		{
			bForward = diChoice(dre);
			if (bForward)							//��������� ������� �����
			{
				for (unsigned int i = 0; i < uiNumSteps; i++)
				{
					if (pCurr)
						pPrev = pCurr;
					pCurr = list.GetNext(pCurr);
				}
			}
			else		//������� �����
			{
				for (unsigned int i = 0; i < uiNumSteps; i++)
				{
					if (pCurr)
						pNext = pCurr;
					pCurr = list.GetPrev(pCurr);
				}
			}
		}
		catch (typename ListErrors::Nullptr) 		//������������� ���������� � �������� � ������� �������� ���������
		{
			if (bForward)
				pCurr = pPrev;
			else
				pCurr = pNext;
		}
		catch (typename ListErrors::NotPartOfList)
		{
			pCurr = list.GetFirst();
		}

		if (diChoice(dre) == 0)
		{
			if (pCurr)		//������ �� ����
				list.Add(pCurr, diValue(dre));
			else			//������ ����
			{
				list.Add(diValue(dre));
				pCurr = list.GetFirst();
			}
		}
		else							//������� ��������
		{
			try
			{
				pNext = list.GetNext(pCurr);
				pPrev = list.GetPrev(pCurr);				
			}
			catch (typename ListErrors::ListError) 		//������������� ���������� �� ������
			{
				pCurr = list.GetFirst();
			}

			try
			{
				list.Delete(pCurr, true);
			}
			catch(typename ListErrors::ListError) {}

			pCurr = pNext;
			if (!pCurr)
				pCurr = list.GetFirst();			
		}

		//� ��������� ������������ ���������� ����������� ���������� ����� � ������ � ������� � ���� �������
		//if (rand() % c_uiRecalculateNodesNumber == 0)
		//{
		//	//CalculateListSizeAndAverage(list);
		//	auto ullNumElementsListMax = list.GetNumElementsMax_SearchElementPolicy();
		//	if (ullNumElementsListMax != 0)
		//	{
		//		auto ullCurrentElementIndex = list.GetCurrentElementIndex_SearchElementPolicy();
		//		SendDlgItemMessage(hMainDlg, IDC_BIT_ARRAY_PROGRESS, PBM_SETPOS, LOWORD(static_cast<double>(ullCurrentElementIndex) / static_cast<double>(ullNumElementsListMax) * 100.0), 0);
		//	}
		//}

		//Sleep((rand() % (c_uiSleepPeriodMax - 1) + 1) * 1000);		//�������� �� ��������� �����
		//Sleep(100);		//�������� �� ��������� �����

		InterlockedExchangeAdd((LONG*)&ulCyclesPerSecond, 1);
	}

	return 0;
}

//��������� ������� ��������� ������, ������������ ������ �� �������: ���������� � �������� ���������
//��������� �������� ������ ������ ������ � ��� ���������
//������� ��� ������ ��� ����������
template<class List, class ListElement> DWORD WINAPI WorkingThreadProcE(PVOID pvParam)
{
	//������ ������ ������� ������ ���������� ����������: ��� ������� ������ ���������� ��������� ����, � ����� ����������� ����
	//� ������ �������� ����� ������������ ������� �� ������ ����� ��� ����� �� ����������� ���������� �����, � ����� ���� ���������� ����������
	//������ ���� � ����� ��������, ���� �������� ����, � �������� ��������� �������; ����� - ����� �������� �����
	//��� ����������� ������������ � ����������� ������

	//C++STD RANDOM////////////////////////////
	std::default_random_engine dre(GetTickCount() + rand());
	std::uniform_int_distribution<int> diSteps(0, c_uiNumSteps - 1);
	std::uniform_int_distribution<int> diChoice(0, 1);
	std::uniform_int_distribution<int> diValue(0, RAND_MAX - 1);
	///////////////////////////////////////////

	using ptrListElement = typename List::ptrListElement;		//��������� ��� ��������� �� ������ ������

	List* pList = reinterpret_cast<List*>(pvParam);		//�������� ��������� �� ������ ������, ���������� ����� �������� �������
	List& list = *pList;						//������ ������ �� ������ ��� ��������

	//���������� ��������� ��������� � ������
	unsigned __int64 u64CurrentNodeNumber = (unsigned __int64)(double(rand()) / double(RAND_MAX) * u64NumNodesInitial);
	ptrListElement pCurr = list.GetFirst();			//������� �������, � ������� ������������ ������
	ptrListElement pNext = nullptr;					//��������� ������� ������
	ptrListElement pPrev = nullptr;					//���������� ������� ������

	ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;	//��� ������ ��� �������� ������

	//��������� �� ��������� �������	
	for (unsigned __int64 i = 0; i < u64CurrentNodeNumber; i++)
	{
		pPrev = pCurr;
		pCurr = list.GetNext(pCurr, &eError);
		if (pCurr == nullptr && eError == ListErrorCodes::eListErrorCode::NotPartOfList)
			pCurr = pPrev;
	}

	unsigned int uiNumSteps = 100;						//�������� �������� �� ������
	bool bForward = true;								//���� ����������� ��������
	while (!bExit)
	{
		WaitForSingleObject(hStopEvent, INFINITE);		//������� �� ������� ������������

		if (pCurr == nullptr || (pCurr != nullptr && list.FindElement(pCurr) == false))
			pCurr = list.GetFirst();
		//uiNumSteps = rand() % c_uiNumSteps;
		//bForward = (rand() % 2 == 0);
		uiNumSteps = diSteps(dre);
		bForward = diChoice(dre);
		if (bForward)							//��������� ������� �����
		{
			for (unsigned int i = 0; i < uiNumSteps; i++)
			{
				if (pCurr)
					pPrev = pCurr;
				pCurr = list.GetNext(pCurr, &eError);
				if (pCurr == nullptr)
				{
					if (eError == ListErrorCodes::eListErrorCode::Nullptr)
						pCurr = pPrev;
					if (eError == ListErrorCodes::eListErrorCode::NotPartOfList)
						pCurr = list.GetFirst();
				}
			}
		}
		else		//������� �����
		{
			for (unsigned int i = 0; i < uiNumSteps; i++)
			{
				if (pCurr)
					pNext = pCurr;
				pCurr = list.GetPrev(pCurr, &eError);
			}
			if (pCurr == nullptr)
			{
				if (eError == ListErrorCodes::eListErrorCode::Nullptr)
					pCurr = pNext;
				if (eError == ListErrorCodes::eListErrorCode::NotPartOfList)
					pCurr = list.GetFirst();
			}
		}

		//if (rand() % 2 == 0)			//��������� ��������
		if(diChoice(dre) == 0)
		{
			if (pCurr)		//������ �� ����
				eError = list.Add(pCurr, diValue(dre));
			else			//������ ����
			{
				eError = list.Add(diValue(dre));
				pCurr = list.GetFirst();
			}
		}
		else							//������� ��������
		{
			pNext = list.GetNext(pCurr, &eError);
			pPrev = list.GetPrev(pCurr, &eError);
			if ((pNext == nullptr || pPrev == nullptr) && eError != ListErrorCodes::eListErrorCode::Success)
				pCurr = list.GetFirst();

			list.Delete(pCurr, &eError, true);

			pCurr = pNext;
			if (!pCurr)
				pCurr = list.GetFirst();			
		}

		//� ��������� ������������ ���������� ����������� ���������� ����� � ������ � ������� � ���� �������
		//if (rand() % c_uiRecalculateNodesNumber == 0)
		//{
		//	//CalculateListSizeAndAverage(list);
		//	auto ullNumElementsListMax = list.GetNumElementsMax_SearchElementPolicy();
		//	if (ullNumElementsListMax != 0)
		//	{
		//		auto ullCurrentElementIndex = list.GetCurrentElementIndex_SearchElementPolicy();
		//		SendDlgItemMessage(hMainDlg, IDC_BIT_ARRAY_PROGRESS, PBM_SETPOS, LOWORD(static_cast<double>(ullCurrentElementIndex) / static_cast<double>(ullNumElementsListMax) * 100.0), 0);
		//	}
		//}

		//Sleep((rand() % (c_uiSleepPeriodMax - 1) + 1) * 1000);		//�������� �� ��������� �����
		//Sleep(100);		//�������� �� ��������� �����

		InterlockedExchangeAdd((LONG*)&ulCyclesPerSecond, 1);
	}

	return 0;
}

template<bool bUseExceptions> DWORD WINAPI CreateListThreadProc(PVOID pvParam)		//������� ������, ���������� ������ � ������������ �������� ������ ��� ������ � ���
{
	srand(GetTickCount());
	if(ce_bPerformanceMeasure)
		vPerformanceMarks.reserve(ce_uiNumSecondsMeasureMax + 1);

	//������ ������� ������� � ������������ �� �������� ������������
	hUserTaskEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//������ ������
	SetDlgItemText(hMainDlg, IDC_STATUS, TEXT("Creating of list..."));
	bool bResult = true;			//��������� ��������
	if (bOneLinked)
	{
		for (unsigned __int64 i = 0; i < u64NumNodesInitial; i++)
		{
			if constexpr (bUseExceptions)
				bResult = listOneLinked.Add(rand(), false, true);
			else
				bResult = listOneLinked.Add(rand(), false, true) == ListErrorCodes::eListErrorCode::Success;
			if (!bResult)
			{
				MessageBox(hMainDlg, TEXT("Couldn't create a new element of the list!"), TEXT("Error"), MB_OK | MB_ICONERROR);
				EndDialog(hMainDlg, 0);
				return 0;
			}
		}
	}
	else
	{
		for (unsigned __int64 i = 0; i < u64NumNodesInitial; i++)
		{
			if constexpr (bUseExceptions)
				bResult = listTwoLinked.Add(rand(), false, true);
			else
				bResult = listTwoLinked.Add(rand(), false, true) == ListErrorCodes::eListErrorCode::Success;
			if (!bResult)
			{
				MessageBox(hMainDlg, TEXT("Couldn't create a new element of the list!"), TEXT("Error"), MB_OK | MB_ICONERROR);
				EndDialog(hMainDlg, 0);
				return 0;
			}
		}
	}

	//��������� �������� ����������
	SetDlgItemText(hMainDlg, IDC_STATUS, TEXT(""));
	EnableWindow(GetDlgItem(hMainDlg, IDC_FIND_NODE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_SEARCH_VALUE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_CALCULATE_LIST_SIZE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_CALCULATE_AVERAGE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_SEARCH_VALUE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_FIND_NODE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_STOP_MULTITHREADING), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_NODE_NUMBER), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_NODE_VALUE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_READ), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_WRITE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_CREATE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDC_DELETE), TRUE);

	//������ ������� ��� ������ ������������ �������� �������
	hStopEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	//������ �������� ������
	if (bOneLinked)
	{
		for (unsigned short int i = 0; i < usiNumThreads; i++)
		{
			if constexpr (bUseExceptions)
				hThreads[i + 1] = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)WorkingThreadProc<decltype(listOneLinked), ListElementData_1L>, &listOneLinked, 0, NULL);
			else
				hThreads[i + 1] = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)WorkingThreadProcE<decltype(listOneLinked), ListElementData_1L>, &listOneLinked, 0, NULL);
		}
	}
	else
	{
		for (unsigned short int i = 0; i < usiNumThreads; i++)
		{
			if constexpr (bUseExceptions)
				hThreads[i + 1] = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)WorkingThreadProc<decltype(listTwoLinked), ListElementData_2L>, &listTwoLinked, 0, NULL);
			else
				hThreads[i + 1] = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)WorkingThreadProcE<decltype(listTwoLinked), ListElementData_2L>, &listTwoLinked, 0, NULL);
		}
	}

	return 0;
}

template<class _List, class ptrListElement> ptrListElement FindListElementByNumber(_List& list, unsigned long long ullElementNumber)
{
	//���������� ��������� �� ������� ������ �� ��� ������
	//�� ����: list - ������ �� ������, ullElementNumber - ����� ��������
	//�� �����: ��������� �� ������� ������ ���� nullptr, ���� �������� �� ������ ������ �� ����������

	//�������, ��� ������ ��� ������������

	auto listElement = list.GetFirst(true);
	ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
	for (unsigned int i = 0; i < ullElementNumber; i++)
	{
		if (!listElement)
			break;
		if constexpr (!bUseExceptions)
			listElement = list.GetNext(listElement, &eError, true, true);
		else
			listElement = list.GetNext(listElement, true, true);
	}

	return listElement;
}

template<class _List> bool ReadListElementByNumber(_List& list, unsigned long long ullElementNumber, unsigned long long& ullReadValue)
{
	//������ �������� �������� ������ �� ��� ������
	//�� ����: list - ������ �� ������, ullElementNumber - ����� ��������, ullReadValue - ������ �� ����������, � ������� ����� ���������� �������� �� ������
	//�� �����: true - �������, false - �������� �� ������ ������ �� ����������

	list.LockListShared();		//��������� ������ �� ����� ��������

	ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
	auto listElement = FindListElementByNumber<_List, typename _List::ptrListElement>(list, ullElementNumber);
	if (listElement)
	{
		if constexpr (!bUseExceptions)
			ullReadValue = list.GetValue(listElement, &eError, true);
		else
			ullReadValue = list.GetValue(listElement, true);
	}	

	list.UnlockListShared();	//������������ ������
	
	if (listElement)
		return true;
	else
		return false;
}

template<class List> bool WriteListElementByNumber(List& list, unsigned long long ullElementNumber, unsigned long long& ullWriteValue)
{
	//���������� �������� �������� ������ �� ��� ������
	//�� ����: list - ������ �� ������, ullElementNumber - ����� ��������, ullWriteValue - ������ �� ����������, �������� ������� ����� ������������ � ������� ������
	//�� �����: true - �������, false - �������� �� ������ ������ �� ����������

	list.LockListShared();		//��������� ������ �� ����� ��������

	auto listElement = FindListElementByNumber<List, typename List::ptrListElement>(list, ullElementNumber);
	if (listElement)
		list.SetValue(listElement, ullWriteValue, true);

	list.UnlockListShared();	//������������ ������

	if (listElement)
		return true;
	else
		return false;
}

template<class List> bool CreateListElementByNumber(List& list, unsigned long long ullElementNumber, unsigned long long& ullValue)
{
	//������ ����� ������� � ������ � �������� ��������� ����� �������� � �������� �������
	//�� ����: list - ������ �� ������, ullElementNumber - ����� ��������, ullValue - ������ �� ����������, �������� ������� ����� ������������ � ������� ������
	//�� �����: true - �������, false - �������� �� ������ ������ �� ����������

	list.LockListShared();		//��������� ������ �� ����� ��������

	auto listElement = FindListElementByNumber<List, typename List::ptrListElement>(list, ullElementNumber);
	if (listElement)
		list.Add(listElement, ullValue, true);

	list.UnlockListShared();	//������������ ������
	
	if (listElement)
		return true;
	else
		return false;
}

template<class _List> bool DeleteListElementByNumber(_List& list, unsigned long long ullElementNumber)
{
	//������� �������� �������� ������ �� ��� ������
	//�� ����: list - ������ �� ������, ullElementNumber - ����� ��������
	//�� �����: true - �������, false - �������� �� ������ ������ �� ����������

	list.LockListShared();		//��������� ������ �� ����� ��������

	ListErrorCodes::eListErrorCode eError = ListErrorCodes::eListErrorCode::Success;
	auto listElement = FindListElementByNumber<_List, typename _List::ptrListElement>(list, ullElementNumber);
	if (listElement)
	{
		if constexpr (!bUseExceptions)
			list.Delete(listElement, &eError, true, true);
		else
			list.Delete(listElement, true, true);
	}

	list.UnlockListShared();	//������������ ������

	if (listElement)
		return true;
	else
		return false;
}

//���������� ��������� �������� ���� �������
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		//������������� ����������� ����
		hMainDlg = hDlg;
		EnableWindow(GetDlgItem(hDlg, IDC_FIND_NODE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_SEARCH_VALUE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CALCULATE_LIST_SIZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CALCULATE_AVERAGE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_SEARCH_VALUE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_FIND_NODE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STOP_MULTITHREADING), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_START_MULTITHREADING), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_NODE_NUMBER), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_NODE_VALUE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_READ), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_WRITE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CREATE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_DELETE), FALSE);
		{
			TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
			_stprintf_s(szTextBuffer, TEXT("%I64u"), u64NumNodesInitial);
			SetDlgItemText(hDlg, IDC_NUM_NODES_INIT, szTextBuffer);
			_stprintf_s(szTextBuffer, TEXT("%u"), usiNumThreads);
			SetDlgItemText(hDlg, IDC_NUM_THREADS, szTextBuffer);

			if (bOneLinked)
				CheckRadioButton(hDlg, IDC_ONE_LINKED, IDC_TWO_LINKED, IDC_ONE_LINKED);
			else
				CheckRadioButton(hDlg, IDC_ONE_LINKED, IDC_TWO_LINKED, IDC_TWO_LINKED);

			SendDlgItemMessage(hDlg, IDC_BIT_ARRAY_PROGRESS, PBM_SETRANGE, 0, (LPARAM)MAKELPARAM(0, 100));
		}

		//������ �������������� ������
		idMMTimer = timeSetEvent(1000, 100, MMTimerProc, NULL, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ONE_LINKED:
			bOneLinked = true;
			return TRUE;
		case IDC_TWO_LINKED:
			bOneLinked = false;
			return TRUE;
		case IDC_CREATE_LIST:			//������ ������
			{
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_NUM_THREADS, szTextBuffer, c_usiBufferSize);
				_stscanf_s(szTextBuffer, TEXT("%hu"), &usiNumThreads);
				GetDlgItemText(hDlg, IDC_NUM_NODES_INIT, szTextBuffer, c_usiBufferSize);
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &u64NumNodesInitial);
				if (usiNumThreads > c_usiNumThreadsMax)
				{
					_stprintf_s(szTextBuffer, TEXT("Threads limit exceeded! The maximum number of threads is %u."), c_usiNumThreadsMax);
					MessageBox(hDlg, szTextBuffer, TEXT("Data input error"), MB_OK | MB_ICONINFORMATION);
					return TRUE;
				}

				//������ �����, ������� ������� ������, � ����� ������� �������� ������ � ���������� usiNumThreads
				EnableWindow(GetDlgItem(hDlg, IDC_NUM_NODES_INIT), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_NUM_THREADS), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CREATE_LIST), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_ONE_LINKED), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_TWO_LINKED), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CS), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_SRW_LOCK), FALSE);
				hThreads = new HANDLE[usiNumThreads + 1];		//�������� ������ ��� ���������� �������
				if (!hThreads)
				{
					MessageBox(hDlg, TEXT("Couldn't allocate memory for thread handles!"), TEXT("Error"), MB_OK | MB_ICONERROR);
					EndDialog(hDlg, 0);
					return TRUE;
				}				
				hThreads[0] = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START)CreateListThreadProc<bUseExceptions>, NULL, 0, NULL);
			}
			return TRUE;
		case IDC_CALCULATE_LIST_SIZE:		//����������� ���������� ��������� � ������ �� ����������
		case IDC_CALCULATE_AVERAGE:			//����������� �������� �������� �������� � ������ �� ����������
			if (bOneLinked)
				CalculateListSizeAndAverage(listOneLinked, true);
			else
				CalculateListSizeAndAverage(listTwoLinked, true);
			return TRUE;
		case IDC_FIND_NODE:					//����� ���� �� ��������
			{
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_SEARCH_VALUE, szTextBuffer, c_usiBufferSize);	//�������� �������� ����
				unsigned __int64 u64SearchValue = 0;
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &u64SearchValue);				
				if (bOneLinked)				//����������� ������
				{
					auto time = GetTickCount();					//������ ������ �������
					auto listElement = listOneLinked.FindElementByValue(u64SearchValue);
					if (listElement)
					{
						try
						{
							unsigned int uiElementNumber = listOneLinked.FindElementNumber(listElement);		//����� ��������
							double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
							_stprintf_s(szTextBuffer, TEXT("%u"), uiElementNumber);
							SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, szTextBuffer);
							_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
							SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
							SetDlgItemText(hDlg, IDC_STATUS, TEXT("Found a list element containing the value requested."));
						}
						catch (typename ListErrors::NotPartOfList)
						{
							SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, TEXT("already deleted"));
							SetDlgItemText(hDlg, IDC_TIME, TEXT(""));
							SetDlgItemText(hDlg, IDC_STATUS, TEXT("The located list element with the value specified was deleted before this message."));
						}
					}
					else
					{
						SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, TEXT("not found"));
						SetDlgItemText(hDlg, IDC_TIME, TEXT(""));
						SetDlgItemText(hDlg, IDC_STATUS, TEXT("List element with the value requested was not found."));
					}
				}
				else				//���������� ������
				{
					auto time = GetTickCount();					//������ ������ �������
					auto listElement = listTwoLinked.FindElementByValue(u64SearchValue);
					if (listElement)
					{
						try
						{
							unsigned int uiElementNumber = listTwoLinked.FindElementNumber(listElement);		//����� ��������
							double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
							_stprintf_s(szTextBuffer, TEXT("%u"), uiElementNumber);
							SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, szTextBuffer);
							_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
							SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
							SetDlgItemText(hDlg, IDC_STATUS, TEXT("Found a list element containing the value requested."));
						}
						catch (typename ListErrors::NotPartOfList)
						{
							SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, TEXT("already deleted"));
							SetDlgItemText(hDlg, IDC_TIME, TEXT(""));
							SetDlgItemText(hDlg, IDC_STATUS, TEXT("The located list element with the value specified was deleted before this message."));
						}
					}
					else
					{
						SetDlgItemText(hDlg, IDC_SEARCH_NODE_NUMBER, TEXT("not found"));
						SetDlgItemText(hDlg, IDC_TIME, TEXT(""));
						SetDlgItemText(hDlg, IDC_STATUS, TEXT("List element with the value requested was not found."));
					}
				}
			}
			return TRUE;
		case IDC_STOP_MULTITHREADING:		//������������� ������������� ������ �� �������
			EnableWindow(GetDlgItem(hDlg, IDC_STOP_MULTITHREADING), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_START_MULTITHREADING), TRUE);
			ResetEvent(hStopEvent);
			return TRUE;
		case IDC_START_MULTITHREADING:		//��������� ������������� ������ �� �������
			EnableWindow(GetDlgItem(hDlg, IDC_STOP_MULTITHREADING), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_START_MULTITHREADING), FALSE);
			SetEvent(hStopEvent);
			return TRUE;
		case IDC_READ:						//������ �������� ���� �� ���������� ������
			{
				unsigned long long ullElementNumber = 0;	//����� ����
				unsigned __int64 u64Value = 0;				//�������� �������� ����
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_NODE_NUMBER, szTextBuffer, c_usiBufferSize);	//�������� �������� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &ullElementNumber);

				bool bReadSuccess = true;
				auto time = GetTickCount();			//������ ������ �������
				if (bOneLinked)				//����������� ������
					bReadSuccess = ReadListElementByNumber<decltype(listOneLinked)>(listOneLinked, ullElementNumber, u64Value);
				else
					bReadSuccess = ReadListElementByNumber<decltype(listTwoLinked)>(listTwoLinked, ullElementNumber, u64Value);
				double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
				_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
				SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
				if (bReadSuccess)
				{
					_stprintf_s(szTextBuffer, TEXT("%I64u"), u64Value);
					SetDlgItemText(hDlg, IDC_NODE_VALUE, szTextBuffer);
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("Read the element value at specified index."));
				}
				else
				{
					SetDlgItemText(hDlg, IDC_NODE_VALUE, TEXT("wrong index"));
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("There is no element with the index specified."));
				}				
			}
			return TRUE;
		case IDC_WRITE:						//���������� �������� ���� �� ���������� ������
			{
				unsigned long long ullElementNumber = 0;	//����� ����
				unsigned __int64 u64Value = 0;				//������������ �������� ����
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_NODE_NUMBER, szTextBuffer, c_usiBufferSize);	//�������� ����� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &ullElementNumber);
				GetDlgItemText(hDlg, IDC_NODE_VALUE, szTextBuffer, c_usiBufferSize);	//�������� �������� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &u64Value);

				bool bWriteSuccess = true;
				auto time = GetTickCount();				//������ ������ �������
				if (bOneLinked)				//����������� ������
					bWriteSuccess = WriteListElementByNumber<decltype(listOneLinked)>(listOneLinked, ullElementNumber, u64Value);
				else
					bWriteSuccess = WriteListElementByNumber<decltype(listTwoLinked)>(listTwoLinked, ullElementNumber, u64Value);
				double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������					
				_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
				SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
				if (bWriteSuccess)
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("Wrote the value element at the specified index."));
				else
				{
					SetDlgItemText(hDlg, IDC_NODE_VALUE, TEXT("wrong index"));
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("There is no element with the index specified."));
				}
			}
			return TRUE;
		case IDC_CREATE:						//������ ������� ������
			{
				unsigned long long ullElementNumber = 0;	//����� ����
				unsigned __int64 u64Value = 0;				//������������ �������� ����
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_NODE_NUMBER, szTextBuffer, c_usiBufferSize);	//�������� ����� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &ullElementNumber);
				GetDlgItemText(hDlg, IDC_NODE_VALUE, szTextBuffer, c_usiBufferSize);	//�������� �������� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &u64Value);

				bool bCreateSuccess = true;
				auto time = GetTickCount();				//������ ������ �������
				if (bOneLinked)				//����������� ������
					bCreateSuccess = CreateListElementByNumber<decltype(listOneLinked)>(listOneLinked, ullElementNumber, u64Value);
				else
					bCreateSuccess = CreateListElementByNumber<decltype(listTwoLinked)>(listTwoLinked, ullElementNumber, u64Value);
				double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
				_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
				SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
				if (bCreateSuccess)
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("The new list element was created at specified index."));
				else
				{
					SetDlgItemText(hDlg, IDC_NODE_VALUE, TEXT("wrong index"));
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("There is no element with the index specified."));
				}
			}
			return TRUE;
		case IDC_DELETE:						//������� ������� ������
			{
				unsigned long long ullElementNumber = 0;	//����� ����
				TCHAR szTextBuffer[c_usiBufferSize];		//��������������� ��������� �����
				GetDlgItemText(hDlg, IDC_NODE_NUMBER, szTextBuffer, c_usiBufferSize);	//�������� ����� ����
				_stscanf_s(szTextBuffer, TEXT("%I64u"), &ullElementNumber);

				bool bDeleteSuccess = true;
				auto time = GetTickCount();				//������ ������ �������
				if (bOneLinked)				//����������� ������
					bDeleteSuccess = DeleteListElementByNumber<decltype(listOneLinked)>(listOneLinked, ullElementNumber);
				else
					bDeleteSuccess = DeleteListElementByNumber<decltype(listTwoLinked)>(listTwoLinked, ullElementNumber);
				double dTime = ((double)GetTickCount() - (double)time) / 1000.0;	//�������� ����� ��������
				
				_stprintf_s(szTextBuffer, TEXT("%lf �"), dTime);
				SetDlgItemText(hDlg, IDC_TIME, szTextBuffer);
				if (bDeleteSuccess)
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("Deleted list element at the specified index."));
				else
				{
					SetDlgItemText(hDlg, IDC_NODE_VALUE, TEXT("wrong index"));
					SetDlgItemText(hDlg, IDC_STATUS, TEXT("There is no element with the index specified.."));
				}
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			timeKillEvent(idMMTimer);
			return TRUE;
		}
	}
	return FALSE;
}

//������� �������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//�������������� �������� ������ �����������
	{
		INITCOMMONCONTROLSEX* lpInitCtrls = new INITCOMMONCONTROLSEX;
		lpInitCtrls->dwSize = sizeof(INITCOMMONCONTROLSEX);
		lpInitCtrls->dwICC = ICC_PROGRESS_CLASS;

		if (!InitCommonControlsEx(lpInitCtrls))
		{
			MessageBox(NULL, TEXT("Couldn't initialize common controls!"), TEXT("Error"), MB_OK | MB_ICONERROR);
			return 0;
		}

		delete lpInitCtrls;
	}

	InitializeCriticalSectionAndSpinCount(&csUIUpdate, 4000);		//�������������� ����������� ������
	{
		//���������� ���������� ���������� �������, ������� ����� ��������� ��������� � ������� ������������
		SYSTEM_INFO si = { 0 };		//��������� � ��������� ����������
		GetSystemInfo(&si);
		usiNumThreads = (unsigned short int)si.dwNumberOfProcessors;
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);		//�������� ������� ���������� ����
	
	//����������� �������
	DeleteCriticalSection(&csUIUpdate);
	if (hStopEvent)
		CloseHandle(hStopEvent);
	InterlockedExchange8(reinterpret_cast<volatile CHAR *>(&bExit), true);		//������ �������� � ���������� ������ �������
	if (hThreads)
	{
		WaitForMultipleObjects(usiNumThreads, hThreads + 1, TRUE, INFINITE);
		for (unsigned short int i = 0; i < usiNumThreads; i++)
			CloseHandle(hThreads[i]);
		delete hThreads;
		hThreads = nullptr;
	}

	if (ce_bPerformanceMeasure == true && bMeasureFinished == true)
	{
		//��������� ���� ��� ������
		std::ofstream file("PerformanceMeasure.txt", std::ios::trunc);

		//���� ������?
		if (file)
		{
			//��������� � ������� ��������� ���������
			//std::sort(vPerformanceMarks.begin(), vPerformanceMarks.end(), [](std::pair<unsigned long long, unsigned long>& pMsr1, std::pair<unsigned long long, unsigned long>& pMsr2)
			//	{
			//		return pMsr1.first < pMsr2.first;
			//	});
			//vPerformanceMarks.erase(std::unique(vPerformanceMarks.begin(), vPerformanceMarks.end(), [](const std::pair<unsigned long long, unsigned long>& pMsr1, const std::pair<unsigned long long, unsigned long>& pMsr2)
			//	{
			//		return pMsr1.first == pMsr2.first;
			//	}), vPerformanceMarks.end());

			//��������� ��������� ������������������
			for (unsigned int i = 0; i < vPerformanceMarks.size(); i++)
			{
				auto& [ullNumElements, ulNumCycles] = vPerformanceMarks[i];
				file << std::setw(5) << ullNumElements << ':' << ulNumCycles << std::endl;
			}
			MessageBox(NULL, TEXT("Program completed after reaching the specified number of list elements or after measure time limit exceeding. If this is not "
				"an expecting behaviour, change the flag ce_bPerformanceMeasure. Measure data were successfully saved."), TEXT("Measure finishing"),
				MB_OK | MB_ICONINFORMATION);
		}
		else
			MessageBox(NULL, TEXT("Program completed after reaching the specified number of list elements or after measure time limit exceeding. Couldn't "
				"open the file to save measure data!"), TEXT("Measure finishing"), MB_OK | MB_ICONEXCLAMATION);
	}
}