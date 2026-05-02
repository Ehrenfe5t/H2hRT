#pragma once

#include"DxQTriangleAccelerateStruct.h"

#include"DxQScenarioObject.h"

namespace TriangleAccelerateStructDatabaseStd {


	class TriangleAccelerateStructDatabase
	{
	public:
		int triangle_real_size;
		TriangleAccelerateStructStd::TriangleAccelerateStruct* triangleAccelerateStructs;

		TriangleAccelerateStructDatabase();
		~TriangleAccelerateStructDatabase();

	private:

	};

	bool InitTriangleAccelerateStructDatabaseByScenarioObject(
		const ScenarioObjectStd::ScenarioObject& scenarioAccelerateStruct,
		TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase);

}
