
#include"DxQStructureToString.h"
#include <sstream>
#include <iomanip>

namespace StructureToStringStd {



    std::string DoubleToStringByStringStream(double value, int doubleSize)
    {
        std::ostringstream stream;
        stream << std::setprecision(doubleSize) << value;
        return stream.str();
    }

    std::string FloatToStringByStringStream(double value, int floatSize)
    {
        std::ostringstream stream;
        stream << std::setprecision(floatSize) << value;
        return stream.str();
    }

    std::string IntToStringByStringStream(int value)
    {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }
    std::string Size_tToStringByStringStream(size_t value)
    {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }

    std::string BoolToStringByStringStream(bool value)
    {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }



    std::string CharArrayToString(const char* charArray, int startIndex, int endIndex) {
        std::string res;
        for (int i = startIndex; i < endIndex; ++i) {
            res.push_back(charArray[i]);
        }
        return res;
    }

}