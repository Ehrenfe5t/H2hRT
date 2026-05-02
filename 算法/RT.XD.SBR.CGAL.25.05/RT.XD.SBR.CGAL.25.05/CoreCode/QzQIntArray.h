#pragma once

#include<vector>

namespace IntArrayStd {

	class IntArray
	{
	public:

		size_t size;
		int* value;

		bool Init(const std::vector<int>& values);

		IntArray();
		~IntArray();

	private:
		void Free();
	};

}