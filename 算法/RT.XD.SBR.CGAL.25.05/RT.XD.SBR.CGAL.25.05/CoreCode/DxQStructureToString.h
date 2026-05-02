#pragma once



#include<string>

namespace StructureToStringStd {



    std::string DoubleToStringByStringStream(double value, int doubleSize);

    std::string FloatToStringByStringStream(double value, int floatSize);

    std::string IntToStringByStringStream(int value);
    std::string Size_tToStringByStringStream(size_t value);

    std::string BoolToStringByStringStream(bool value);

    std::string CharArrayToString(const char* charArray, int startIndex, int endIndex);
    

}

