

#include"Ray3DIntersectGeometry3DElementsAabbBvhTree.h"


#include"Ray3DIntersectGeometry3DElementsAabbBvhTree1.h"
#include"Ray3DIntersectGeometry3DElementsAabbBvhTree2.h"
#include"Ray3DIntersectGeometry3DElementsAabbBvhTree3.h"
#include"Ray3DIntersectGeometry3DElementsAabbBvhTree4.h"

#include<iostream>
#include<chrono>

namespace Ray3DIntersectGeometry3DElementsAabbBvhTreeStd {

	const int model = 4;

	void InitializeAabbBvhTree(const Scenario3D& scenario, bool switchCorner, double cornerRadius) {


		switch (model)
		{
		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTree1Std::InitializeAabbBvhTree(scenario, switchCorner, cornerRadius);
			break;
		}
		case 2: {
			//加速结构初始化耗时: 25492.4 ms.
			Ray3DIntersectGeometry3DElementsAabbBvhTree2Std::InitializeAabbBvhTree(scenario, switchCorner, cornerRadius);
			break;
		}
		case 3: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree3Std::InitializeAabbBvhTree(scenario, switchCorner, cornerRadius);
			break;
		}
		case 4: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree4Std::InitializeAabbBvhTree(scenario, switchCorner, cornerRadius);
			break;
		}

		default:
			std::cout << "Invalid model: " << model << std::endl;
			break;
		}
	}

	void CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result) {

		switch (model)
		{
		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTree1Std::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
				ray_origin, ray_direction, result);
			break;
		}
		case 2: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree2Std::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
				ray_origin, ray_direction, result);
			break;
		}
		case 3: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree3Std::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
				ray_origin, ray_direction, result);
			break;
		}
		case 4: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree4Std::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
				ray_origin, ray_direction, result);
			break;
		}

		default:
			std::cout << "Invalid model: " << model << std::endl;
			break;
		}

	}

	void CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result) {

		switch (model)
		{
		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTree1Std::CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
				ray_origin, ray_direction, cornerRadius, result);
			break;
		}
		case 2: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree2Std::CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
				ray_origin, ray_direction, cornerRadius, result);
			break;
		}
		case 3: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree3Std::CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
				ray_origin, ray_direction, cornerRadius, result);
			break;
		}
		case 4: {

			Ray3DIntersectGeometry3DElementsAabbBvhTree4Std::CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
				ray_origin, ray_direction, cornerRadius, result);
			break;
		}

		default:
			std::cout << "Invalid model: " << model << std::endl;
			break;
		}

	}

}