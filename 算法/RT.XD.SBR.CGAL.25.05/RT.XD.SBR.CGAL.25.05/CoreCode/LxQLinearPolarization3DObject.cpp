


#include"LxQLinearPolarization3DObject.h"
#include"QzQGeometry3DOperate.Normalization.h"
#include"QzQGeometry3DOperate.Point3D.h"

namespace LinearPolarization3DObjectStd {




	LinearPolarization3DObject::LinearPolarization3DObject(double phi0, const Point3DStd::Point3D& vec)
	{
		this->phi0 = phi0;
		this->vec = vec;
	}

	Point3DStd::Point3D LinearPolarization3DObject::GetOneOthervec() const
	{
		Point3DStd::Point3D other(vec.z, vec.x, vec.y);
		return Geometry3DOperateStd::Normalization_Point3D(Geometry3DOperateStd::CrossPoint3DPoint3D(vec, other));
	}

	LinearPolarization3DObject::LinearPolarization3DObject()
	{
		this->phi0 = 0.0;
		this->vec.z = 1.0;
	}

	LinearPolarization3DObject::~LinearPolarization3DObject()
	{
	}


	/// <summary>
	/// 从json字符串获取点
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void from_json(const nlohmann::json& j, LinearPolarization3DObject& obj) {
		{
			auto jsonObject = j.at("phi0");
			if (!jsonObject.is_null()) {
				jsonObject.get_to(obj.phi0);
			}
		}
		{
			auto jsonObject = j.at("vec");
			if (!jsonObject.is_null()) {
				jsonObject.get_to(obj.vec);
			}
		}
	}
	/// <summary>
	/// 将点对象转化为json字符串
	/// </summary>
	/// <param name="j"></param>
	/// <param name="p"></param>
	void to_json(nlohmann::json& j, const LinearPolarization3DObject& obj) {
		j["phi0"] = obj.phi0;
		j["vec"] = obj.vec;
	}

}