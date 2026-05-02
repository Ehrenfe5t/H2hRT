#pragma once

#include"LxQLinearPolarization3DObject.h"

namespace OneLinearPolarization3DStd {

	class OneLinearPolarization3D
	{
	public:

		OneLinearPolarization3D();
		OneLinearPolarization3D(double weight,const LinearPolarization3DObjectStd::LinearPolarization3DObject& linearPolarization3DObject);
		~OneLinearPolarization3D();

		bool GetLegal() const;
		LinearPolarization3DObjectStd::LinearPolarization3DObject GetLinearPolarization3DObject() const;
		void SetLinearPolarization3DObject(double weight,const LinearPolarization3DObjectStd::LinearPolarization3DObject& linearPolarization3DObject);
		double GetWeight() const;

		/// <summary>
		/// 权重，如果是0则没有值，同属于一个模式下的两个线极化A,B的权重分别是2，3，则发射功率w分给A：2/5；分给B：3/5.
		/// </summary>
		double weight;
		LinearPolarization3DObjectStd::LinearPolarization3DObject linearPolarization3DObject;


		/// <summary>
		/// true表示合法，false表示非法
		/// </summary>
		bool legal;
	private:
	};

	/// <summary>
	/// 从json字符串获取点
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void from_json(const nlohmann::json& j, OneLinearPolarization3D& obj);

	/// <summary>
	/// 将点对象转化为json字符串
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void to_json(nlohmann::json& j, const OneLinearPolarization3D& obj);
}