


#include"HdQArrayInt.h"

namespace ArrayIntStd {


	
	bool ArrayInt::UpdateBySet(const std::set<int>& data_set)
	{
		this->size = (int)data_set.size();
		if (this->size == 0) {
			return true;
		}


		int count = 0;
		for (auto ele : data_set) {
			this->data[count] = ele;
			count++;
		}
		return true;
	}
	ArrayInt::ArrayInt()
	{
		this->size = 0; 
		std::fill(this->data, this->data + data_max_size, -1);
	}

	ArrayInt::~ArrayInt()
	{
	}
}