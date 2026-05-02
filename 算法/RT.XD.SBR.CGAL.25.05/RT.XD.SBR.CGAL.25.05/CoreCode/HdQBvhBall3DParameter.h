#pragma once



namespace BvhBall3DParameterStd {

	/// <summary>
	/// BvhBall3DParameter表示BvhBall3D的参数
	/// </summary>
	class BvhBall3DParameter
	{
	public:
		/// <summary>
		/// 最大层数
		/// </summary>
		int maxLevel;

		BvhBall3DParameter();
		BvhBall3DParameter(int maxLevel);
		~BvhBall3DParameter();

	private:

	};


}