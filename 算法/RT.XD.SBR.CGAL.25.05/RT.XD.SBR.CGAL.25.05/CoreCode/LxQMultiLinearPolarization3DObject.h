#pragma once

#include"LxQOneLinearPolarization3D.h"

namespace MultiLinearPolarization3DObjectStd {

	class MultiLinearPolarization3DObject
	{
	public:
		/// <summary>
		/// 唯一编号
		/// </summary>
		int polarization3DModelId;

		std::vector<OneLinearPolarization3DStd::OneLinearPolarization3D> multiLinearPolarization3D;

		MultiLinearPolarization3DObject();
		~MultiLinearPolarization3DObject();

	private:

	};

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, MultiLinearPolarization3DObject& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
	void to_json(nlohmann::json& j, const MultiLinearPolarization3DObject& obj);

}