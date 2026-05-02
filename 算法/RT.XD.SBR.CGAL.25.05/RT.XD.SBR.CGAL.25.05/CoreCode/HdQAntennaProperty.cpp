
#include"HdQAntennaProperty.h"

namespace AntennaPropertyStd {

    static int AntennaID_count = 0;

	AntennaProperty::AntennaProperty()
	{
        this->radiationPatternId = -1;
        this->polarization3DModelId = -1;
        this->frequencys.emplace_back(1e9);
	}


    AntennaProperty::~AntennaProperty()
    {
    }

}