#include "DxQTriangleAccelerateStructDatabase.h"


#include"LxQProjectDependencies.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Normalization.h"
#include"QzQGeometry3DOperate.Create.h"

namespace TriangleAccelerateStructDatabaseStd {



	const int TriangleAccelerateStructDatabaseSize = 3000000;

	TriangleAccelerateStructDatabase::TriangleAccelerateStructDatabase()
	{
		this->triangle_real_size = 0;
		this->triangleAccelerateStructs = NULL;
		this->triangleAccelerateStructs = (TriangleAccelerateStructStd::TriangleAccelerateStruct*)malloc(TriangleAccelerateStructDatabaseSize*sizeof(TriangleAccelerateStructStd::TriangleAccelerateStruct));
		
	}

	TriangleAccelerateStructDatabase::~TriangleAccelerateStructDatabase()
	{
		delete this->triangleAccelerateStructs;
		this->triangleAccelerateStructs = NULL;
	}

	bool InitTriangleAccelerateStructDatabaseByScenarioObject(
		const ScenarioObjectStd::ScenarioObject& scenarioAccelerateStruct,
		TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase)
	{
		if ((int)scenarioAccelerateStruct.scenarioTriangle3DIndex.size() > TriangleAccelerateStructDatabaseSize) {

			{
				std::ostringstream oss;
				oss << "当前版本支持的最大面元数量为 " << TriangleAccelerateStructDatabaseSize << std::endl;
				oss << "实际值为  " << scenarioAccelerateStruct.scenarioTriangle3DIndex.size() << std::endl;
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}

			return false;
		}
		triangleAccelerateStructDatabase.triangle_real_size = (int)scenarioAccelerateStruct.scenarioTriangle3DIndex.size();

		for (int loop = 0; loop < scenarioAccelerateStruct.scenarioTriangle3DIndex.size(); ++loop) {
			Point3DStd::Point3D p1 = scenarioAccelerateStruct.scenarioPoint3D[scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].TriangleP1Index];
			Point3DStd::Point3D p2 = scenarioAccelerateStruct.scenarioPoint3D[scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].TriangleP2Index];
			Point3DStd::Point3D p3 = scenarioAccelerateStruct.scenarioPoint3D[scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].TriangleP3Index];
			Point3DStd::Point3D n = Geometry3DOperateStd::Normalization_Point3D(Geometry3DOperateStd::CrossPoint3DPoint3D(
				Geometry3DOperateStd::SubPoint3DPoint3D(p2, p1), Geometry3DOperateStd::SubPoint3DPoint3D(p3, p1)));
			if (Geometry3DOperateStd::DotPoint3DPoint3D(n, scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].n) < 0.0) {
				auto temp = p2;
				p2 = p3;
				p3 = temp;
			}

			Point3DStd::Point3D E1 = Geometry3DOperateStd::SubPoint3DPoint3D(p1, p2);
			Point3DStd::Point3D E2 = Geometry3DOperateStd::SubPoint3DPoint3D(p1, p3);
			Point3DStd::Point3D E1E2 = Geometry3DOperateStd::CrossPoint3DPoint3D(E1, E2);
			Point3DStd::Point3D E2E1 = Geometry3DOperateStd::CrossPoint3DPoint3D(E2, E1);

			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].upTypeNumber = scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].UpTypeNumber;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].downTypeNumber = scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].DownTypeNumber;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1 = p1;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP2 = p2;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP3 = p3;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleN = scenarioAccelerateStruct.scenarioTriangle3DIndex[loop].n;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1 = E1;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2 = E2;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1E2 = E1E2;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2E1 = E2E1;
			triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].triangleBox = Geometry3DOperateStd::CreateBoundingBox3DBy3Point3D(p1, p2, p3);

		}

		return true;

	}

}