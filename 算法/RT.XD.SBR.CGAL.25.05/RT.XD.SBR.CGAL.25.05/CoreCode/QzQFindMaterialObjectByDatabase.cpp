
#include"QzQFindMaterialObjectByDatabase.h"
#include"LxQMaterialObjectDatabase.h"

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
        MaterialObjectStd::MaterialObject& materialObject_down) {
        if (!MaterialObjectDatabaseStd::Find(upObjectType, frequency, materialObject_up)) {
            return false;
        }
        if (!MaterialObjectDatabaseStd::Find(downObjectType, frequency, materialObject_down)) {
            return false;
        }
        return true;
    }
}