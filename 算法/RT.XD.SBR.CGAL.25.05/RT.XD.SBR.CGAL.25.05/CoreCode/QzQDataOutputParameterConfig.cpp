#include "QzQDataOutputParameterConfig.h"


namespace DataOutputParameterConfigStd {


    DataOutputParameterConfig::DataOutputParameterConfig()
    {
        this->outPutDirectoryPathName = "OutPutDirectory";
        this->outPutLogTxtFileName = "Log.txt";
        this->switchOfBigChannelParameterInfo = true;
        this->switchOfPathInfo = true;
        this->switchOfSmallChannelParameterInfo = true;
        this->switchOfStatisticChannelParameterInfo = true;
        this->switchOfMultipleSignalSourceSuperposition = false;
    }

    DataOutputParameterConfig::~DataOutputParameterConfig()
    {
    }

}