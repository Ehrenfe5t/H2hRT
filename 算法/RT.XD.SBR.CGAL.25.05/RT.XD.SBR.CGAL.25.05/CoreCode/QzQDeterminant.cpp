
#include"QzQDeterminant.h"

namespace DeterminantStd {


	Determinant::Determinant()
	{
		this->m = 0;
		this->n = 0;
	}

	Determinant::Determinant(int m, int n)
	{
		this->m = m;
		this->n = n;
		determinant.resize(m, std::vector<double>(n));
	}

	Determinant::~Determinant()
	{
	}

}