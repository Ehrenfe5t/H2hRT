
#include"QzQFileBase.h"
#include"QzQJsonFileOperateBase.h"


namespace JsonFileOperateBaseStd {


    void WriteJsonStringToJsonFile(const char* path, const nlohmann::json& jf) {
        if (FileOperateStd::ExistFile(path)) {
            remove(path);
        }

        std::ofstream fileOpen; //定义ofstream 对象
        //fileOpen.open(path, std::ofstream::app);//追加
        fileOpen.open(path);
        if (!fileOpen)
        {
            std::cout << ("[" + std::string(path) + "]文件没有打开!") << std::endl;
            fileOpen.close();
            return;
        }
        
        //std::cout << jf;
        fileOpen << jf.dump(4);
        fileOpen.close();
    }

}