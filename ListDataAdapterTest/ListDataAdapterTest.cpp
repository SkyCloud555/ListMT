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

//Test program in creation of list data adapters making ListMT be alike std:list.

#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include "..\List\ListDataExample.h"
#include "..\List\SearchContainerElement.h"	//стратегии поиска элемента

using namespace ListMT;
using namespace std;

List_TwoLinked_DataAdapter<int, MemoryPolicy, ThreadLocking_STDMutex, DirectSearch, false> list0;
List_TwoLinked_DataAdapter<double, SmartSharedPointer, ThreadLocking_STDMutex, DirectSearch, false> list00;
List_TwoLinked_DataAdapter<double> list1;

int main()
{
	list0.AddLast(nullptr, false, false, 0);
	list0.AddLast(nullptr, false, false, 1);
	list0.AddLast(nullptr, false, false, 2);
	list0.AddLast(nullptr, false, false, 3);

	int x = *list0.GetLast();			//тип приходится указывать явно, иначе через auto будет выведен тип ListElementCompound_TwoLinked_CP<...>
	cout << "x = " << x << endl << endl;

	constexpr unsigned int ce_uiElementsMax = 10;
	for (unsigned int i = 0; i < ce_uiElementsMax; i++)
		list1.AddLast(false, false, sin(static_cast<double>(i) / static_cast<double>(ce_uiElementsMax) * 2.0 * 3.14159265358));

	unsigned int i = 0;
	for (auto& dElem : list1)
		cout << i++ << ": " << dElem << endl;

	transform(list1.begin(), list1.end(), list1.begin(), [](double& dElem1)
		{
			return dElem1 * dElem1;
		});
	cout << endl;
	cout << "Transformed: " << endl;
	for (auto& dElem : list1)
		cout << i++ << ": " << dElem << endl;

	vector<double> v(list1.cbegin(), list1.cend());
	cout << endl;
	cout << "Vector, copied from list: " << endl;
	for (auto& dElem : v)
		cout << i++ << ": " << dElem << endl;

	cout << endl;
	cout << "Direct copy: " << endl;
	v.clear();
	copy(v.begin(), v.end(), list1.begin());
	
	list<double> l;
	auto y = 3.0;
	l.push_front(y);
	l.push_front(8);
	list1.push_back(5);

	decltype(list1) list2;
	copy(list1.cbegin(), list1.cend(), back_inserter(list2));
	cout << endl;
	cout << "list2 = list1 through std::copy algorithm with insertion: " << endl;
	for (auto& dElem : list2)
		cout << i++ << ": " << dElem << endl;

	auto list3 = list00 + list1;
}