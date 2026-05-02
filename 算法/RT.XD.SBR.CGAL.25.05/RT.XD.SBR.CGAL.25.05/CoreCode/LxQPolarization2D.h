#pragma once

#include"HdQComplex.h"


namespace Polarization2DStd {
	/// <summary>
	/// 二维极化分量
	/// </summary>
	class Polarization2D
	{
	public:
		/// <summary>
		/// 水平极化,TM
		/// </summary>
		ComplexStd::Complex hh;
		/// <summary>
		/// 垂直极化,TE
		/// </summary>
		ComplexStd::Complex vv;
		Polarization2D(const ComplexStd::Complex& hh, const ComplexStd::Complex& vv);
		Polarization2D();
		~Polarization2D();

	private:

	};

	Polarization2DStd::Polarization2D MulDoublePolarization2D(double t, const Polarization2DStd::Polarization2D& c2);
}