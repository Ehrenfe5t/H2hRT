

#include"LxQPolarization3D.h"

#include"HdQComplexOperate.h"

namespace Polarization3DStd {



	Polarization3D::Polarization3D()
	{
	}

	Polarization3D::Polarization3D(const ComplexStd::Complex& ex, const ComplexStd::Complex& ey, const ComplexStd::Complex& ez)
	{
		this->ex = ex;
		this->ey = ey;
		this->ez = ez;
	}

	Polarization3D::~Polarization3D()
	{
	}

	Polarization3D MulComplexPolarization3D(const ComplexStd::Complex& c, const Polarization3D& e)
	{
		Polarization3D res;
		res.ex = ComplexStd::MulComplexComplex(c, e.ex);
		res.ey = ComplexStd::MulComplexComplex(c, e.ey);
		res.ez = ComplexStd::MulComplexComplex(c, e.ez);
		return res;
	}

}