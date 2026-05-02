
#include "HdQCornerAccelerateStructDatabase.h"

#include "LxQProjectDependencies.h"

namespace CornerAccelerateStructDatabaseStd {

	const int CornerAccelerateStructArrayMaxSize = 1500000;

	CornerAccelerateStructDatabase::CornerAccelerateStructDatabase()
	{
		this->cornerAccelerateStructArray = NULL;
		this->cornerAccelerateStructArraySize = 0;
		this->cornerAccelerateStructArray = (CornerAccelerateStructStd::CornerAccelerateStruct*)malloc(CornerAccelerateStructArrayMaxSize * sizeof(CornerAccelerateStructStd::CornerAccelerateStruct));
	}
	CornerAccelerateStructDatabase::~CornerAccelerateStructDatabase()
	{
		delete this->cornerAccelerateStructArray;
		this->cornerAccelerateStructArray = NULL;
		this->cornerAccelerateStructArraySize = 0;
	}

	bool InitCornerAccelerateStructDatabaseByScenarioObject(const ScenarioObjectStd::ScenarioObject& scenarioAccelerateStruct, CornerAccelerateStructDatabase& cornerAccelerateStructDatabase)
	{
		if (scenarioAccelerateStruct.scenarioCorner3DIndex.size() < 1) {

			{
				std::ostringstream oss;
				oss << "输入的边数量为0" << std::endl;
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}

			return false;
		}


		if ((int)scenarioAccelerateStruct.scenarioCorner3DIndex.size() > CornerAccelerateStructArrayMaxSize) {

			{
				std::ostringstream oss;
				oss << "当前版本支持的最大边数量为 " << CornerAccelerateStructArrayMaxSize << std::endl;
				oss << "实际值为  " << scenarioAccelerateStruct.scenarioCorner3DIndex.size() << std::endl;
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}

			return false;
		}

		cornerAccelerateStructDatabase.cornerAccelerateStructArraySize = (int)scenarioAccelerateStruct.scenarioCorner3DIndex.size();

		auto location_points = scenarioAccelerateStruct.scenarioPoint3D;


		for (int loop = 0; loop < scenarioAccelerateStruct.scenarioCorner3DIndex.size(); ++loop) {

			Point3DStd::Point3D p1 = location_points[scenarioAccelerateStruct.scenarioCorner3DIndex[loop].P1Index];
			Point3DStd::Point3D p2 = location_points[scenarioAccelerateStruct.scenarioCorner3DIndex[loop].P2Index];

			cornerAccelerateStructDatabase.cornerAccelerateStructArray[loop].start_location = p1;
			cornerAccelerateStructDatabase.cornerAccelerateStructArray[loop].end_location = p2;
			cornerAccelerateStructDatabase.cornerAccelerateStructArray[loop].triangle_1_index = scenarioAccelerateStruct.scenarioCorner3DIndex[loop].Face0Index;
			cornerAccelerateStructDatabase.cornerAccelerateStructArray[loop].triangle_2_index = scenarioAccelerateStruct.scenarioCorner3DIndex[loop].FaceNIndex;

		}

		return true;
	}
}