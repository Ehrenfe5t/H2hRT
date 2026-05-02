#include "TransmittingAntennaDatabaseObject.h"

namespace TransmittingAntennaDatabaseObjectStd {
	TransmittingAntennaDatabaseObject::TransmittingAntennaDatabaseObject()
	{
		this->legal = false;
	}
	TransmittingAntennaDatabaseObject::~TransmittingAntennaDatabaseObject()
	{
	}
	void TransmittingAntennaDatabaseObject::SetTransmittingAntenna(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna)
	{
		if (transmittingAntenna.transmittingAntennaId < 0) {

			{
				std::ostringstream oss;
				oss << "transmittingAntennaId²»ÄÜ<0!";
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}

			this->legal = false;
			return;
		}
		this->legal = true;
		this->transmittingAntenna = transmittingAntenna;
	}
}
