
#include"DxQStringToStructure.h"
#include <sstream>

namespace StringToStructureStd {


    int StringToInt(const std::string& str)
    {
        std::istringstream iss(str);
        int num;
        iss >> num;
        return num;
    }


    float StringToFloat(const std::string& str)
    {
        std::istringstream iss(str);
        float num;
        iss >> num;
        return num;
    }

    double StringToDouble(const std::string& str)
    {
        std::istringstream iss(str);
        double num;
        iss >> num;
        return num;
    }

}



