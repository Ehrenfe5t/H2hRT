#pragma once

#include<set>

namespace ArrayIntStd {

	static const int data_max_size = 1200;

	class ArrayInt
	{
	public:
		int size = 0;

		int data[data_max_size];
		ArrayInt();
		~ArrayInt();

		bool UpdateBySet(const std::set<int>& data_set);
	private:

	};

}