#pragma once

#include"HdQComplex.h"


namespace Polarization3DStd {


	class Polarization3D
	{
	public:
		ComplexStd::Complex ex;
		ComplexStd::Complex ey;
		ComplexStd::Complex ez;
		Polarization3D();
		Polarization3D(const ComplexStd::Complex& ex, const ComplexStd::Complex& ey, const ComplexStd::Complex& ez);
		~Polarization3D();

	private:

	};

	Polarization3D MulComplexPolarization3D(const ComplexStd::Complex& c, const Polarization3D& e);
}
