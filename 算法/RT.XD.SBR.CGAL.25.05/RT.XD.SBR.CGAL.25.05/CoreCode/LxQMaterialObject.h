#pragma once

#include"HdQComplex.h"
#include<string>
#include<vector>
#include <algorithm>// 包含std::copy
namespace MaterialObjectStd {
	/// <summary>
	/// 一种材质,材质类型不能为负数，
	/// </summary>
	class MaterialObject {
	public:
		/// <summary>
		/// 材质类型
		/// </summary>
		int typeNumber;
		/// <summary>
		/// 频率
		/// </summary>
		long long frequency;
		/// <summary>
		/// 相对介电常数
		/// </summary>
		double relativePermittivity;
		/// <summary>
		/// 电导率
		/// </summary>
		double conductivity;
		/// <summary>
		/// 相对磁导率
		/// </summary>
		double relativePermeability;
		/// <summary>
		/// 导磁率
		/// </summary>
		double magnetoconductivity;

		/// <summary>
		/// 名称
		/// </summary>
		char materialName[180];

		MaterialObject();
		MaterialObject(const MaterialObject&obj);
		~MaterialObject();
	};


	int ContainMaterialObjectInVectorMaterialObject(const MaterialObject& value,
		const std::vector<MaterialObject>& list);

	int FindIndexOfListMaterialParamByObjectTypeAndF(int typeNumber, long long frequency,
		const std::vector<MaterialObjectStd::MaterialObject>& materialObjects);

	ComplexStd::Complex GetK(const MaterialObjectStd::MaterialObject& materialObject);

	double GetTime(const MaterialObjectStd::MaterialObject& materialObject, double distance);

	/// <summary>
	/// 计算材质阻抗
	/// </summary>
	/// <param name="materialObject"></param>
	/// <returns></returns>
	double GetZ(const MaterialObjectStd::MaterialObject& materialObject);
}
