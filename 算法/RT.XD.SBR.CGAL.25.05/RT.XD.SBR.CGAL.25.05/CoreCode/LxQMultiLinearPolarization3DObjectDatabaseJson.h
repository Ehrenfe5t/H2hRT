#pragma once

#include"LxQMultiLinearPolarization3DObjectDatabase.h"

/// <summary>
/// 本地交互的json文件
/// </summary>
namespace MultiLinearPolarization3DObjectDatabaseJsonStd {

	class MultiLinearPolarization3DObjectDatabaseJson
	{
	public:
		std::string a1_describe;
		std::string a2_describe;
		std::string a3_describe;
		std::string a4_describe;
		std::string a5_describe;
		std::string a6_describe;
		std::string a7_describe;

		std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject> database;

		MultiLinearPolarization3DObjectDatabaseJson();
		~MultiLinearPolarization3DObjectDatabaseJson();

	private:

	};


	void from_json(const nlohmann::json& j, MultiLinearPolarization3DObjectDatabaseJson& obj);
	void to_json(nlohmann::json& j, const MultiLinearPolarization3DObjectDatabaseJson& obj);


}