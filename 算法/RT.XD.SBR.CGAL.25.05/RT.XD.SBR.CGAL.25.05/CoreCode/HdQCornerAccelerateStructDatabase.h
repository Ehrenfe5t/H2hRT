#pragma once
#include"HdQCornerAccelerateStruct.h"
#include"DxQScenarioObject.h"

namespace CornerAccelerateStructDatabaseStd {


    class CornerAccelerateStructDatabase
    {
    public:
        int cornerAccelerateStructArraySize;
        CornerAccelerateStructStd::CornerAccelerateStruct* cornerAccelerateStructArray;

        CornerAccelerateStructDatabase();
        ~CornerAccelerateStructDatabase();

    private:

    };

    bool InitCornerAccelerateStructDatabaseByScenarioObject(
        const ScenarioObjectStd::ScenarioObject& scenarioAccelerateStruct,
        CornerAccelerateStructDatabase& cornerAccelerateStructDatabase);

}