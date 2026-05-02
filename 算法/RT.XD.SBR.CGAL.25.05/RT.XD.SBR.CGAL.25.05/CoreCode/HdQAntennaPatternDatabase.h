#pragma once

#include"HdQAntennaPatternDatabaseObject.h"
#include"HdQConfig.h"
#include<vector>

namespace AntennaPatternDatabaseStd {


	INTERFACE_API void Clear();

	INTERFACE_API bool FindAntennaPatternObject(int radiationPatternId, AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject);

	INTERFACE_API void AddAntennaPatternObject(const AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject);

	INTERFACE_API void AddRangeAntennaPatternObject(const std::vector<AntennaPatternObjectStd::AntennaPatternObject>& antennaPatternObjects);

	INTERFACE_API std::vector<AntennaPatternObjectStd::AntennaPatternObject> ToAntennaPatternObjectVector();


	INTERFACE_API double GetRadiationPatternByVector(int radiationPatternId, const Point3DStd::Point3D& vector);

	INTERFACE_API double CalCoefficientByVector(int radiationPatternId, const Point3DStd::Point3D& vector);

}