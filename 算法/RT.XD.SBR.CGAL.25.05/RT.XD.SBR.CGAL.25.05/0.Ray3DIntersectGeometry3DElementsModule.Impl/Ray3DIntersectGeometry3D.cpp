
#include"0.Ray3DIntersectGeometry3DElementsModule.Impl.Input.h"

#include<iostream>
#include<algorithm>
#include<chrono>

#include"AabbBvhTree/Ray3DIntersectGeometry3DElementsAabbBvhTree.h"
#include"Exhaust/Ray3DIntersectGeometry3DElementsExhaust.h"
#include"Base/Ray3DIntersectGeometry3DElementsBase.h"



namespace Ray3DIntersectGeometry3DStd {

	int global_type = 0;

	void Initialize(int type, const Scenario3D& scenario, bool switchCorner, double cornerRadius) {

		auto start = std::chrono::high_resolution_clock::now(); // 记录开始时间点

		global_type = type;
		switch (global_type) {
		case 0:
		{
			Ray3DIntersectGeometry3DElementsExhaustStd::InitializeExhaust(scenario, switchCorner);
			break;
		}

		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTreeStd::InitializeAabbBvhTree(scenario, switchCorner, cornerRadius);
			break;
		}
			
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::Initialize, unsupported type: " << global_type << std::endl;
			break;
		}

		auto end = std::chrono::high_resolution_clock::now(); // 记录结束时间点
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		//std::cout << "加速结构初始化耗时: " << duration.count() / 1000.0 << " ms." << std::endl;
	}



	void CalculateRay3DIntersectTriangle3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result) {

		switch (global_type) {

		//0表示暴力求解
		case 0:
		{
			Ray3DIntersectGeometry3DElementsExhaustStd::CalculateRay3DIntersectTriangle3DExhaustFirst(
				ray_origin,
				ray_direction,
				result);
			break;
		}
			
		//AabbBvhTree
		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTreeStd::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
				ray_origin,
				ray_direction,
				result);
			break;
		}
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectTriangle3D, unsupported type: " << global_type << std::endl;
			break;
		}

	}



	void CalculateRay3DIntersectCorner3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result) {

		switch (global_type) {

			//0表示暴力求解
		case 0:
		{
			Ray3DIntersectGeometry3DElementsExhaustStd::CalculateRay3DIntersectCorner3DExhaustFirst(
				ray_origin,
				ray_direction,
				cornerRadius,
				result);
			break;
		}
		case 1:
		{
			Ray3DIntersectGeometry3DElementsAabbBvhTreeStd::CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
				ray_origin,
				ray_direction,
				cornerRadius,
				result);
			break;
		}
			
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectTriangle3D, unsupported type: " << global_type << std::endl;
			break;
		}

	}


}

namespace Ray3DIntersectGeometry3DStd {

	void CalculateRay3DIntersectTriangle3DSortFirst(
		int result_num,
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results) {

		CalculateRay3DIntersectTriangle3D(ray_origin, ray_direction, results);

		int k = std::min(result_num, (int)results.results.size());
		std::partial_sort(results.results.begin(), results.results.begin() + k, results.results.end(), [](const Ray3DIntersectGeometry3DElementResult& a, const Ray3DIntersectGeometry3DElementResult& b) {
			return a.distance < b.distance;
			});


	}

	void CalculateRay3DIntersectTriangle3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results) {

		switch (global_type) {
		case 0:
			Ray3DIntersectGeometry3DElementsExhaustStd::CalculateRay3DIntersectTriangle3DExhaust(
				ray_origin,
				ray_direction,
				results);
			break;
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectTriangle3D, unsupported type: " << global_type << std::endl;
			break;
		}

	}

	void CalculateRay3DIntersectCorner3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results) {

		switch (global_type) {
		case 0:
			Ray3DIntersectGeometry3DElementsExhaustStd::CalculateRay3DIntersectCorner3DExhaust(
				ray_origin,
				ray_direction,
				cornerRadius,
				results);
			break;
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectCorner3D, unsupported type: " << global_type << std::endl;
			break;
		}
	}


	void CalculateRay3DIntersectTriangle3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results) {

		CalculateRay3DIntersectTriangle3D(ray_origin, ray_direction, results);

		std::sort(results.results.begin(), results.results.end(), [](const Ray3DIntersectGeometry3DElementResult& a, const Ray3DIntersectGeometry3DElementResult& b) {
			return a.distance < b.distance;
			});


	}
}


namespace Ray3DIntersectGeometry3DStd {

	void InitializeDefault(const Scenario3D& scenario, bool switchCorner, double cornerRadius) {
		Initialize(0, scenario,  switchCorner,  cornerRadius);
	}

	void CalculateRay3DIntersectCorner3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results) {

		CalculateRay3DIntersectCorner3D(ray_origin, ray_direction, cornerRadius, results);

		std::sort(results.results.begin(), results.results.end(), [](const Ray3DIntersectGeometry3DElementResult& a, const Ray3DIntersectGeometry3DElementResult& b) {
			return a.distance < b.distance;
			});
	}


	/// <summary>
	/// 射线和整个场景的相交检测，暴力方法
	/// </summary>
	/// <param name="ray_origin">射线的起点</param>
	/// <param name="ray_direction">射线的方向，这里是归一化的</param>
	/// <param name="cornerRadius">边的半径</param>
	/// <param name="scenario">环境的几何信息</param>
	/// <param name="results">计算结果</param>
	void CalculateRay3DIntersectScenario3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results) {

		switch (global_type) {
		case 0:
			Ray3DIntersectGeometry3DElementsExhaustStd::CalculateRay3DIntersectScenario3DExhaust(
				ray_origin,
				ray_direction,
				cornerRadius,
				results);
			break;
		default:
			std::cout << "Error: Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectScenario3D, unsupported type: " << global_type << std::endl;
			break;
		}

	}


	void CalculateRay3DIntersectScenario3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results) {

		CalculateRay3DIntersectScenario3D(ray_origin, ray_direction, cornerRadius, results);

		std::sort(results.results.begin(), results.results.end(), [](const Ray3DIntersectGeometry3DElementResult& a, const Ray3DIntersectGeometry3DElementResult& b) {
			return a.distance < b.distance;
			});

	}


	void CalculateRay3DIntersectScenario3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result) {

		result.elementIndex = -1;
		result.distance = Ray3DIntersectGeometry3DElementsBaseStd::RayMaxMovingDistance;

		Ray3DIntersectGeometry3DElementsResults results;
		CalculateRay3DIntersectScenario3DSort(ray_origin, ray_direction, cornerRadius, results);

		if (!results.results.empty()) {
			result = results.results.front();
		}
	}
}