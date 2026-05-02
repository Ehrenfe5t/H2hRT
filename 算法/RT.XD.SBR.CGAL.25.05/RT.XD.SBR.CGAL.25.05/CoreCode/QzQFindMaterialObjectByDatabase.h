#pragma once

#include"LxQMaterialObject.h"

namespace FindMaterialObjectByDatabaseStd {

    /// <summary>
    /// ”Î≤ƒ÷ ø‚Ωªª•
    /// </summary>
    /// <param name="upObjectType"></param>
    /// <param name="downObjectType"></param>
    /// <param name="frequency"></param>
    /// <param name="materialObject_up"></param>
    /// <param name="materialObject_down"></param>
    /// <returns></returns>
    bool FindMaterialObjectByDatabase(
        int upObjectType,
        int downObjectType,
        long long frequency,
        MaterialObjectStd::MaterialObject& materialObject_up,
        MaterialObjectStd::MaterialObject& materialObject_down);

}