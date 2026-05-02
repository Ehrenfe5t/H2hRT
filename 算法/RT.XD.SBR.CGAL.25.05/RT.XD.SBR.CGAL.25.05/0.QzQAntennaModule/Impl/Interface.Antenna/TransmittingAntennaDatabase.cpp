#include "TransmittingAntennaDatabaseObject.h"



namespace TransmittingAntennaDatabaseStd {


    const int TransmittingAntennaDatabaseSize = 10;

    class TransmittingAntennaDatabase
    {
    public:

        TransmittingAntennaDatabaseObjectStd::TransmittingAntennaDatabaseObject transmittingAntennaDatabaseObject[TransmittingAntennaDatabaseSize];

        TransmittingAntennaDatabase();
        ~TransmittingAntennaDatabase();

    private:

    };

    TransmittingAntennaDatabase::TransmittingAntennaDatabase()
    {
    }

    TransmittingAntennaDatabase::~TransmittingAntennaDatabase()
    {
    }

    TransmittingAntennaDatabase database;


    int FindFirstIllegalIndex() {
        for (int i = 0; i < TransmittingAntennaDatabaseSize; ++i) {
            if (!database.transmittingAntennaDatabaseObject[i].legal) {
                return i;
            }
        }
        return -1;
    }

    int IndexOf(int transmittingAntennaId) {
        for (int i = 0; i < TransmittingAntennaDatabaseSize; ++i) {
            if (database.transmittingAntennaDatabaseObject[i].legal) {
                if (transmittingAntennaId == database.transmittingAntennaDatabaseObject[i].transmittingAntenna.transmittingAntennaId) {
                    return i;
                }
            }
        }
        return -1;
    }

    bool FindTransmittingAntenna(int transmittingAntennaId, TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna)
    {
        int index = IndexOf(transmittingAntennaId);
        if (index == -1) {
            return false;
        }
        transmittingAntenna = database.transmittingAntennaDatabaseObject[index].transmittingAntenna;
        return true;
    }


    void Add(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna)
    {
        TransmittingAntennaDatabaseObjectStd::TransmittingAntennaDatabaseObject transmittingAntennaDatabaseObject;
        transmittingAntennaDatabaseObject.SetTransmittingAntenna(transmittingAntenna);
        if (transmittingAntennaDatabaseObject.legal) {
            int index = IndexOf(transmittingAntennaDatabaseObject.transmittingAntenna.transmittingAntennaId);
            if (index == -1) {
                int firstIllegalIndex = FindFirstIllegalIndex();
                if (firstIllegalIndex == -1) {
                    //数据库已经满了无法添加了
                    return;
                }
                else {
                    //将数据添加到合适位置
                    database.transmittingAntennaDatabaseObject[firstIllegalIndex].SetTransmittingAntenna(transmittingAntenna);
                }
            }
            else {
                //将数据添加到合适位置，这里覆盖了数据
                database.transmittingAntennaDatabaseObject[index].SetTransmittingAntenna(transmittingAntenna);
            }
        }
        else {
            //传入的无效数据
        }
    }

    void AddRange(const std::vector<TransmittingAntennaStd::TransmittingAntenna>& transmittingAntennas)
    {
        for (int i = 0; i < transmittingAntennas.size(); ++i) {
            Add(transmittingAntennas[i]);
        }
    }


}