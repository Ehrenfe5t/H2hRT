#pragma once

#include"LxQMultiLinearPolarization3DObject.h"


/// <summary>
/// 和本地文件交互
/// </summary>
namespace MultiLinearPolarization3DObjectDatabaseStd {


	class MultiLinearPolarization3DObjectDatabase
	{
	public:
		std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject> database;
		MultiLinearPolarization3DObjectDatabase();
		~MultiLinearPolarization3DObjectDatabase();

		int IndexOf(int polarization3DModelId) const;
		void Add(const MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject& multiLinearPolarization3DObject);
		void AddAll(const std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject>& multiLinearPolarization3DObjects);

	private:

	};


	void BaseDescribeMultiLinearPolarization3DObjectDatabase1(
		std::string& describe);

	void BaseDescribeMultiLinearPolarization3DObjectDatabase3(
		std::string& a1_describe,
		std::string& a2_describe,
		std::string& a3_describe,
		std::string& a4_describe,
		std::string& a5_describe,
		std::string& a6_describe,
		std::string& a7_describe);

	void BaseDescribeMultiLinearPolarization3DObjectDatabase2(
		std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject>& multiLinearPolarization3DObjects);



}