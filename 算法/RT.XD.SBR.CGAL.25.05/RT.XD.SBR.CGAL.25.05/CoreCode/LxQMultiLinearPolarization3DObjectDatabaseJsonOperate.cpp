
#include"LxQMultiLinearPolarization3DObjectDatabaseJsonOperate.h"
#include"QzQJsonFileOperateBase.h"
#include"QzQFileBase.h"
#include"LxQMultiLinearPolarization3DDatabase.h"


namespace MultiLinearPolarization3DObjectDatabaseJsonOperateStd {



    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void ReadMultiLinearPolarization3DObjectDatabaseJsonByJsonFile(const char* path, MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson& object) {
        if (!FileOperateStd::ExistFile(path)) {
            return;
        }
        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, object);
        ifs.close();
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteMultiLinearPolarization3DObjectDatabaseJsonToJsonFile(const char* path, const MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson& object) {
        nlohmann::json jf;
        to_json(jf, object);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }

    void WriteMultiLinearPolarization3DDatabaseToJsonFile(const std::string& multiLinearPolarization3DDatabaseFileName) {

        MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase2
            = MultiLinearPolarization3DDatabaseStd::ToMultiLinearPolarization3DObjectDatabase();
        MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson multiLinearPolarization3DObjectDatabaseJson;
        multiLinearPolarization3DObjectDatabaseJson.database = multiLinearPolarization3DObjectDatabase2.database;
        MultiLinearPolarization3DObjectDatabaseJsonOperateStd::WriteMultiLinearPolarization3DObjectDatabaseJsonToJsonFile(
            multiLinearPolarization3DDatabaseFileName.c_str(), multiLinearPolarization3DObjectDatabaseJson);

    }

    void InitMultiLinearPolarization3DDatabaseByJsonFile(const std::string& multiLinearPolarization3DDatabaseFileName) {

        MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson multiLinearPolarization3DObjectDatabaseJson;
        MultiLinearPolarization3DObjectDatabaseJsonOperateStd::ReadMultiLinearPolarization3DObjectDatabaseJsonByJsonFile(
            multiLinearPolarization3DDatabaseFileName.c_str(), multiLinearPolarization3DObjectDatabaseJson);

        MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase1;
        multiLinearPolarization3DObjectDatabase1.AddAll(multiLinearPolarization3DObjectDatabaseJson.database);
        MultiLinearPolarization3DDatabaseStd::InitDatabaseByMultiLinearPolarization3DObjectDatabase(multiLinearPolarization3DObjectDatabase1);

    }

}