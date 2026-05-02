#include"HdQAntennaMIMO.h"

#include"DxQTransmittingAntennaDatabase.h"

namespace AntennaMIMOStd {
    
    
    AntennaMIMO::AntennaMIMO()
    {
    }
    AntennaMIMO::~AntennaMIMO()
    {
    }
    void UpdataTransmittingAntennaDatabase(const AntennaMIMOStd::AntennaMIMO& antennaMIMO)
    {
        for (int i = 0;i< antennaMIMO.antennaSIMOs.size();++i) {
            TransmittingAntennaDatabaseStd::Add(antennaMIMO.antennaSIMOs[i].transmittingAntenna);
        }
    }
}