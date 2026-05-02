

#include"DxQRtIm3DParameterConfigJsonOperate.h"
#include"HdQCommonParameterConfigJsonOperate.h"
#include"QzQDataInputCsvFileParameterConfigJsonOperate.h"
#include"QzQDataOutputParameterConfigJsonOperate.h"
#include"QzQGeometricSpaceAccelerateParameterConfigJsonOperate.h"
#include"LxQMultithreadParameterConfigJsonOperate.h"
#include"LxQNumericalMethodParameterConfigJsonOperate.h"
#include"DxQRayEjectionParameterConfigJsonOperate.h"
#include"DxQRtIm3DPrivateParameterConfigJsonOperate.h"

namespace RtIm3DParameterConfigStd {



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DParameterConfigStd::RtIm3DParameterConfig& obj) {
        {
            auto jsonObject = j.at("commonParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.commonParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("dataInputCsvFileParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.dataInputCsvFileParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("dataOutputParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.dataOutputParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("geometricSpaceAccelerateParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.geometricSpaceAccelerateParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("multithreadParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.multithreadParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("numericalMethodParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.numericalMethodParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("rayEjectionParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.rayEjectionParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("rtIm3DPrivateParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.rtIm3DPrivateParameterConfig);
            }
        }


    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtIm3DParameterConfigStd::RtIm3DParameterConfig& obj) {

        j["commonParameterConfig"] = obj.commonParameterConfig;
        j["dataInputCsvFileParameterConfig"] = obj.dataInputCsvFileParameterConfig;
        j["dataOutputParameterConfig"] = obj.dataOutputParameterConfig;
        j["geometricSpaceAccelerateParameterConfig"] = obj.geometricSpaceAccelerateParameterConfig;
        j["multithreadParameterConfig"] = obj.multithreadParameterConfig;
        j["numericalMethodParameterConfig"] = obj.numericalMethodParameterConfig;
        j["rayEjectionParameterConfig"] = obj.rayEjectionParameterConfig;
        j["rtIm3DPrivateParameterConfig"] = obj.rtIm3DPrivateParameterConfig;

    }



}
