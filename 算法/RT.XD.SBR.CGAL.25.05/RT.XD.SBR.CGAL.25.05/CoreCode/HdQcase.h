#pragma once

/*


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, DiffuseScatteringParameterStd::DiffuseScatteringParameter& obj) {
        {
            auto jsonObject = j.at("a1_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a1_describe);
            }
        }
        {
            auto jsonObject = j.at("a2_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a2_describe);
            }
        }
        {
            auto jsonObject = j.at("a3_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a3_describe);
            }
        }
        {
            auto jsonObject = j.at("a4_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a4_describe);
            }
        }
        {
            auto jsonObject = j.at("a5_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a5_describe);
            }
        }
        {
            auto jsonObject = j.at("a6_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a6_describe);
            }
        }
        {
            auto jsonObject = j.at("a7_describe");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.a7_describe);
            }
        }

        {
            for (auto& object : j["database"]) {
                MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject value;
                from_json(object, value);
                obj.database.emplace_back(value);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const DiffuseScatteringParameterStd::DiffuseScatteringParameter& obj) {

        j["a1_describe"] = obj.a1_describe;
        j["a2_describe"] = obj.a2_describe;
        j["a3_describe"] = obj.a3_describe;
        j["a4_describe"] = obj.a4_describe;
        j["a5_describe"] = obj.a5_describe;
        j["a6_describe"] = obj.a6_describe;
        j["a7_describe"] = obj.a7_describe;

        j["database"];
        for (auto& value : obj.database) {
            nlohmann::json jf;
            to_json(jf, value);
            j["database"].emplace_back(jf);
        }
    }



*/