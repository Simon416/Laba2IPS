#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

/// ������� ReducerMaxTest() ���������� ������������ ������� �������,
/// ����������� �� � �������� ���������, � ��� �������
/// mass_pointer - ��������� �������� ������ ����� �����
/// size - ���������� ��������� � �������
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	cout << "Maximal element = " << maximum->get_reference() << " has index = " << maximum->get_index_reference() << endl;
}


/// ������� ReducerMinTest() ���������� ����������� ������� �������
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	cout << "Minimal element = " << minimum->get_reference() << " has index = " << minimum->get_index_reference() << endl;
	cout << endl;
}

/// ������� ParallelSort() ��������� ������ � ������� �����������
/// begin - ��������� �� ������ ������� ��������� �������
/// end - ��������� �� ��������� ������� ��������� �������
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

/// ������� CompareForAndCilk_For() ������� ������ �������� �� ������� ����� ������ ������������ ����� for, 
/// � ������� ����������� ���������� ���������� � ����� ������ ������������� ����� cilk_for 

int *CompareForAndCilk_For(size_t sz)
{
	cout << "The result of the function CompareForAndCilk_For()" << endl;

	duration<double> duration, /// ��������� ��� ��������� ������� ������ ����� for
		durationCilk; /// ��������� ��� ��������� ������� ������ ����� cilk_for
	cilk::reducer<cilk::op_vector<int>>red_vec; /// ����������� ������ � ����� cilk_for
	vector <int> vec;/// ����������� ������ � ����� for

	/// �������� ����� ������ ����� for
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < sz; ++i)
	{
		vec.push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	// �������� ����� ������ ����� cilk_for
	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(long i = 0; i != sz; ++i)
	{
		red_vec->push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	durationCilk = (t4 - t3);

	/// ������� ��������� ��������� �������
	cout << "Duration is: " << duration.count() << " seconds (the standard for loop)" << endl;
	cout << "Duration is: " << durationCilk.count() << " seconds (cilk_for loop)" << endl;
	cout << endl;

	return 0;
}

int main()
{
	srand((unsigned)time(0));

	// ������������� ���������� ���������� ������� = 4
	__cilkrts_set_param("nworkers", "4");

	/// ������ ����������� �������
	size_t sz;
	cout << "Specify the size of the array: ";
	cin >> sz;

	int *mass_begin, *mass_end;
	int *mass = new int[sz];
	mass_begin = mass;
	mass_end = mass_begin + sz;
	for (long i = 0; i < sz; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}
	

	duration<double> duration; /// ��������� ��� ��������� �������

	ReducerMaxTest(mass, sz); /// ����� ������������� �������� �������
	ReducerMinTest(mass, sz); /// ����� ������������ �������� �������


	// ��������� ������ ������� ParallelSort
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end); /// ���������� �������
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);

	// ����� ������������ �������� ����� ���������� �������
	ReducerMaxTest(mass, sz); /// ����� ������������� �������� �������
	ReducerMinTest(mass, sz); /// ����� ������������ �������� �������
	/// ������� ����� ������ ������� ParallelSort()
	cout << "Duration is: " << duration.count() << " seconds (function ParallelSort())" << endl;
	cout << endl;

	/// ������� - ��������� ������� ������ ������ for � cilk_for
	CompareForAndCilk_For(sz);

	delete[]mass;
	system("pause");
	return 0;
}
