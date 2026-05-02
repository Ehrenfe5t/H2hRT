#pragma once


#include"DxQScenarioObject.h"
#include"DxQScenarioDiffuseScatteringFace.h"
#include"DxQTriangleAccelerateStructDatabase.h"
#include"HdQCornerAccelerateStructDatabase.h"
/// <summary>
/// 场景和加速结构
/// </summary>
namespace ScenarioDataInformationStd {

	class ScenarioDataInformation
	{
	public:
		/// <summary>
		/// 输入场景,多线程并发访问不安全
		/// </summary>
		ScenarioObjectStd::ScenarioObject scenarioObject;

		/// <summary>
		/// 射线碰撞加速结构，多线程并发访问安全
		/// </summary>
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase triangleAccelerateStructDatabase;

		/// <summary>
		/// 射线碰撞加速结构，多线程并发访问安全
		/// </summary>
		CornerAccelerateStructDatabaseStd::CornerAccelerateStructDatabase cornerAccelerateStructDatabase;

		/// <summary>
		/// 漫散射面元
		/// </summary>
		std::vector<ScenarioDiffuseScatteringFaceStd::ScenarioDiffuseScatteringFace> scenarioDiffuseScatteringFace;
		ScenarioDataInformation();
		~ScenarioDataInformation();

	private:

	};
	void from_json(const nlohmann::json& j, ScenarioDataInformation& obj);
	void to_json(nlohmann::json& j, const ScenarioDataInformation& obj);

	/// <summary>
	/// 仅仅初始化 点、三角形和边信息
	/// </summary>
	/// <param name="scenarioObject"></param>
	/// <returns></returns>
	bool InitScenarioDataInformationByScenarioObject(
		bool corner,
		const ScenarioObjectStd::ScenarioObject& scenarioObject,
		ScenarioDataInformation& scenarioDataInformation);

}