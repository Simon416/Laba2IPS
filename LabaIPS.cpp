#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <iostream>
#include <chrono>

using namespace std;
using namespace chrono;

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	cout << "Maximal element = " << maximum->get_reference() << " has index = " << maximum->get_index_reference() << endl;
}


/// Функция ReducerMinTest() определяет минимальный элемент массива
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

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
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

/// Функция CompareForAndCilk_For() функция должна выводить на консоль время работы стандартного цикла for, 
/// в котором заполняется случайными значениями и время работы параллельного цикла cilk_for 

int *CompareForAndCilk_For(size_t sz)
{
	cout << "The result of the function CompareForAndCilk_For()" << endl;

	duration<double> duration, /// Перемнная для измерения времени работы цикла for
		durationCilk; /// Перемнная для измерения времени работы цикла cilk_for
	cilk::reducer<cilk::op_vector<int>>red_vec; /// Заполняемый вектор в цикле cilk_for
	vector <int> vec;/// Заполняемый вектор в цикле for

	/// Измеряем время работы цикла for
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < sz; ++i)
	{
		vec.push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	// Измеряем время работы цикла cilk_for
	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(long i = 0; i != sz; ++i)
	{
		red_vec->push_back(rand() % 25000 + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	durationCilk = (t4 - t3);

	/// Выводим результат измерения времени
	cout << "Duration is: " << duration.count() << " seconds (the standard for loop)" << endl;
	cout << "Duration is: " << durationCilk.count() << " seconds (cilk_for loop)" << endl;
	cout << endl;

	return 0;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	/// Задаем размерность массива
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
	

	duration<double> duration; /// Перемнная для измерения времени

	ReducerMaxTest(mass, sz); /// Поиск максимального элемента массива
	ReducerMinTest(mass, sz); /// Поиск минимального элемента массива


	// измерение работы функции ParallelSort
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end); /// Сортировка массива
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration = (t2 - t1);

	// поиск минимального элемента после сортировки массива
	ReducerMaxTest(mass, sz); /// Поиск максимального элемента массива
	ReducerMinTest(mass, sz); /// Поиск минимального элемента массива
	/// Выводим время работы функции ParallelSort()
	cout << "Duration is: " << duration.count() << " seconds (function ParallelSort())" << endl;
	cout << endl;

	/// Задание - сравнение времени работы циклов for и cilk_for
	CompareForAndCilk_For(sz);

	delete[]mass;
	system("pause");
	return 0;
}
