#pragma once



namespace RayScenarioIntersectTypeStd {


	enum class RayScenarioIntersectType
	{
		/// <summary>
		/// 不划分，暴力求解，已测试
		/// </summary>
		Null,
		/// <summary>
		/// 体素化和距离场结合运用
		/// </summary>
		Pixel3D_SDF,
		/// <summary>
		/// 体素化，不支持
		/// </summary>
		Pixel3D,
		/// <summary>
		/// BVH，不支持
		/// </summary>
		BvhBall3D
	};


}