#include "HdQAntennaPatternDatabaseObject.h"
#include "LxQProjectDependencies.h"

namespace AntennaPatternDatabaseObjectStd {
	AntennaPatternDatabaseObject::AntennaPatternDatabaseObject()
	{
		this->legal = false;
	}
	AntennaPatternDatabaseObject::~AntennaPatternDatabaseObject()
	{
	}

	void AntennaPatternDatabaseObject::SetAntennaPatternObject(
		const AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject)
	{
		if (antennaPatternObject.radiationPatternId < 1) {

			{
				std::ostringstream oss;
				oss << "radiationPatternId²»ÄÜ<1!";
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}
			this->legal = false;
			return;
		}
		this->legal = true;
		this->antennaPatternObject.radiationPatternId = antennaPatternObject.radiationPatternId;
		for (int i = 0; i < AntennaPatternObjectStd::radiationPattern_rows; ++i) {
			for (int j = 0; j < AntennaPatternObjectStd::radiationPattern_cols; ++j) {
				this->antennaPatternObject.radiationPattern[i][j] = antennaPatternObject.radiationPattern[i][j];
			}
		}
	}
}