

#include"LxQMultiLinearPolarization3DObject.h"

namespace MultiLinearPolarization3DObjectStd {

	MultiLinearPolarization3DObject::MultiLinearPolarization3DObject()
	{
		this->polarization3DModelId = -1;
	}

	MultiLinearPolarization3DObject::~MultiLinearPolarization3DObject()
	{
	}


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, MultiLinearPolarization3DObject& obj) {
        {
            auto jsonObject = j.at("polarization3DModelId");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.polarization3DModelId);
            }
        }
        {
            for (auto& object : j["multiLinearPolarization3D"]) {
                OneLinearPolarization3DStd::OneLinearPolarization3D value;
                from_json(object, value);
                if (!value.legal) {
                    continue;
                }
                obj.multiLinearPolarization3D.emplace_back(value);
            }
        }
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const MultiLinearPolarization3DObject& obj) {
        j["polarization3DModelId"] = obj.polarization3DModelId;
        j["multiLinearPolarization3D"];
        for (auto& value : obj.multiLinearPolarization3D) {
            nlohmann::json jf;
            to_json(jf, value);
            j["multiLinearPolarization3D"].push_back(jf);
        }
    }

}

