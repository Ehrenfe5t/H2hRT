
#include"LxQOneLinearPolarization3D.h"

namespace OneLinearPolarization3DStd {


	OneLinearPolarization3D::OneLinearPolarization3D()
	{
		this->legal = false;
		this->weight = 0.0;
	}
	OneLinearPolarization3D::OneLinearPolarization3D(double weight, const LinearPolarization3DObjectStd::LinearPolarization3DObject& linearPolarization3DObject)
	{
		this->legal = true;
		this->weight = weight;
		if (weight <= 0) {
			this->legal = false;
			return;
		}
		this->linearPolarization3DObject = linearPolarization3DObject;
	}

	OneLinearPolarization3D::~OneLinearPolarization3D()
	{
	}

	bool OneLinearPolarization3D::GetLegal() const
	{
		return this->legal;
	}

	LinearPolarization3DObjectStd::LinearPolarization3DObject OneLinearPolarization3D::GetLinearPolarization3DObject() const
	{
		return this->linearPolarization3DObject;
	}

	void OneLinearPolarization3D::SetLinearPolarization3DObject(double weight, const LinearPolarization3DObjectStd::LinearPolarization3DObject& linearPolarization3DObject)
	{
		this->legal = true;
		this->weight = weight;
		if (weight <= 0) {
			this->legal = false;
			return;
		}
		this->linearPolarization3DObject = linearPolarization3DObject;
	}

	double OneLinearPolarization3D::GetWeight() const
	{
		return this->weight;
	}

	/// <summary>
	/// 从json字符串获取点
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void from_json(const nlohmann::json& j, OneLinearPolarization3D& obj) {
		{
			auto jsonObject = j.at("weight");
			if (!jsonObject.is_null()) {
				jsonObject.get_to(obj.weight);
			}
		}
		{
			auto jsonObject = j.at("linearPolarization3DObject");
			if (!jsonObject.is_null()) {
				jsonObject.get_to(obj.linearPolarization3DObject);
			}
		}
		if (obj.weight <= 0) {
			obj.legal = false;
		}
		else {
			obj.legal = true;
		}
	}
	/// <summary>
	/// 将点对象转化为json字符串
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void to_json(nlohmann::json& j, const OneLinearPolarization3D& obj) {
		j["weight"] = obj.weight;
		j["linearPolarization3DObject"] = obj.linearPolarization3DObject;
	}

}