#pragma once

#include"DxQScenarioObject.h"
#include"HdQBvhBall3DParameter.h"
#include"HdQBvhBall3D.h"

/// <summary>
/// 构建一个bvh树结构
/// </summary>
namespace BuildBvhBall3DStd {

	/// <summary>
	/// 输入 点、三角形、边的信息构建一个bvh树
	/// </summary>
	/// <param name="bvhBall3DParameter"></param>
	/// <param name="scenarioObject"></param>
	/// <returns></returns>
	void BuildBvhBall3D(
		const BvhBall3DParameterStd::BvhBall3DParameter& bvhBall3DParameter,
		const ScenarioObjectStd::ScenarioObject& scenarioObject,
		BvhBall3DStd::BvhBall3D& bvhBall3D);

}