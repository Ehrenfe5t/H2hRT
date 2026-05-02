#pragma once



#include"LxQPoint3D.h"



namespace Ball3DStd {


	class Ball3D
	{
	public:

		/// <summary>
		/// °ë¾¶
		/// </summary>
		double radius;

		/// <summary>
		/// ÖÐÐÄ
		/// </summary>
		Point3DStd::Point3D center;

		Ball3D(double radius, const Point3DStd::Point3D& center);
		Ball3D();
		~Ball3D();

		bool IsPoint3DInBox(const Point3DStd::Point3D& obj);
		bool IsTriangle3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3);
		bool IsLineSegment3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);


		bool IsColliding(const Ball3D& ball);
	private:

	};

}