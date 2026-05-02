


#include"HdQComplexOperate.h"
#include"LxQPolarization2D.h"

namespace Polarization2DStd {

	Polarization2D::Polarization2D(const ComplexStd::Complex& hh, const ComplexStd::Complex& vv)
	{
		this->hh = hh;
		this->vv = vv;
	}

	Polarization2D::Polarization2D()
	{
	}

	Polarization2D::~Polarization2D()
	{
	}

	Polarization2DStd::Polarization2D MulDoublePolarization2D(double t, const Polarization2DStd::Polarization2D& c2)
	{
		ComplexStd::Complex hh = ComplexStd::MulDoubleComplex(t, c2.hh);
		ComplexStd::Complex vv = ComplexStd::MulDoubleComplex(t, c2.vv);
		Polarization2DStd::Polarization2D c3(hh, vv);
		return c3;
	}

}