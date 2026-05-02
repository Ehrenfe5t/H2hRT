#include "QzQGlobalConstant.h"

namespace GlobalConstantStd{


    int airSubstanceType = -999;
    int GetAirSubstanceType()
    {
        return airSubstanceType;
    }
    void SetAirSubstanceType(int type)
    {
        airSubstanceType = type;
    }
    
    double boundingBoxPadding = 0.7;
    
    double GetBoundingBoxPadding()
    {
        return boundingBoxPadding;
    }

    void SetBoundingBoxPadding(double padding)
    {
        boundingBoxPadding = padding;
    }

    double numberOfDiscreteBoundingBox = 3000;

    double GetNumberOfDiscreteBoundingBox()
    {
        return numberOfDiscreteBoundingBox;
    }

    void SetNumberOfDiscreteBoundingBox(double numberOfDiscreteBoundingBox_value)
    {
        numberOfDiscreteBoundingBox = numberOfDiscreteBoundingBox_value;
    }
}


