#pragma once

#include"LxQOneLinearPolarization3D.h"
#include"LxQMultiLinearPolarization3DObject.h"

namespace MultiLinearPolarization3DStd {
	
	const int LinearPolarization3DNumber = 5;

	/// <summary>
	/// 这里将解决多个问题，包括单一的不同的线极化，多个不同的任意线极化
	/// </summary>
	class MultiLinearPolarization3D
	{
	public:
		/// <summary>
		/// 唯一编号
		/// </summary>
		int polarization3DModelId;

		MultiLinearPolarization3D();
		MultiLinearPolarization3D(int polarization3DModelId,
			const std::vector<OneLinearPolarization3DStd::OneLinearPolarization3D>& linearPolarization3Ds);
		~MultiLinearPolarization3D();

		/// <summary>
		/// 多线程时会频繁访问
		/// </summary>
		OneLinearPolarization3DStd::OneLinearPolarization3D multiLinearPolarization3D[LinearPolarization3DNumber];

		bool GetLegal() const;

		/// <summary>
		/// true表示合法，false表示非法
		/// </summary>
		bool legal;
	private:

	};

	

	MultiLinearPolarization3DStd::MultiLinearPolarization3D ToMultiLinearPolarization3D(
		const MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject& multiLinearPolarization3DObject);

	MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject ToMultiLinearPolarization3DObject(
		const MultiLinearPolarization3DStd::MultiLinearPolarization3D& multiLinearPolarization3D);

}