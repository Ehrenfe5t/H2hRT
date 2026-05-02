
#include"QzQIntArray.h"

#include<iostream>

namespace IntArrayStd {


	void IntArray::Free()
	{
		if (this->size == 0) {
			this->size = 0;
			this->value = NULL;
			return;
		}
		delete this->value;
		this->size = 0;
		this->value = NULL;
	}

	bool IntArray::Init(const std::vector<int>& values)
	{

		if (values.size() > 0) {

			Free();
			this->size = values.size();
			this->value = (int*)malloc(this->size * sizeof(int));
			if (this->value == NULL) {
				return false;
			}

			for (int i = 0; i < this->size; ++i) {
				this->value[i] = values[i];
			}

			return true;
		}

		return false;
	}

	IntArray::IntArray()
	{
		this->size = 0;
		this->value = NULL;
	}

	IntArray::~IntArray()
	{
		Free();
	}


}