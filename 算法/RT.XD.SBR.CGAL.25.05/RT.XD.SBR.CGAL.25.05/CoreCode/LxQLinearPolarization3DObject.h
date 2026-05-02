#pragma once

#include"LxQPoint3D.h"

namespace LinearPolarization3DObjectStd {

	class LinearPolarization3DObject
	{
	public:
		/// <summary>
		/// 初始相位
		/// </summary>
		double phi0;

		/// <summary>
		/// 天线极化方向
		/// </summary>
		Point3DStd::Point3D vec;


		LinearPolarization3DObject(double phi0, const Point3DStd::Point3D& vec);

		Point3DStd::Point3D GetOneOthervec() const;

		LinearPolarization3DObject();
		~LinearPolarization3DObject();

	private:

	};

	/// <summary>
	/// 从json字符串获取点
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void from_json(const nlohmann::json& j, LinearPolarization3DObject& obj);
	/// <summary>
	/// 将点对象转化为json字符串
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void to_json(nlohmann::json& j, const LinearPolarization3DObject& obj);

}