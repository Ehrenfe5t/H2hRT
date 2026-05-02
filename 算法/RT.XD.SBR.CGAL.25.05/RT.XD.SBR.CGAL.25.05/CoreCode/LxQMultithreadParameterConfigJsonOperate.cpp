

#include"LxQMultithreadParameterConfigJsonOperate.h"


namespace MultithreadParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, MultithreadParameterConfigStd::MultithreadParameterConfig& obj) {
        {
            auto jsonObject = j.at("multithreadConfigSwitchOfMultithread");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.multithreadConfigSwitchOfMultithread);
            }
        }
        {
            auto jsonObject = j.at("multithreadConfigThreadNum");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.multithreadConfigThreadNum);
            }
        }
        {
            auto jsonObject = j.at("multithreadConfigThreadOneCpuCalNum");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.multithreadConfigThreadOneCpuCalNum);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const MultithreadParameterConfigStd::MultithreadParameterConfig& obj) {

        j["multithreadConfigSwitchOfMultithread"] = obj.multithreadConfigSwitchOfMultithread;
        j["multithreadConfigThreadNum"] = obj.multithreadConfigThreadNum;
        j["multithreadConfigThreadOneCpuCalNum"] = obj.multithreadConfigThreadOneCpuCalNum;

    }



}


