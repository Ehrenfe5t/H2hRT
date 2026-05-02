#pragma once

#include "../0.Ray3DIntersectGeometry3DElementsModule.Impl.Input.h"

namespace Ray3DIntersectGeometry3DElementsAabbBvhTree4Std {

	void InitializeAabbBvhTree(const Scenario3D& scenario, bool switchCorner, double cornerRadius);

	void CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result);

}


