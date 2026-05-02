#pragma once

#include"QzQJson.hpp"

#include<fstream>

namespace JsonFileOperateBaseStd {


    void WriteJsonStringToJsonFile(const char* path, const nlohmann::json& jf);


}