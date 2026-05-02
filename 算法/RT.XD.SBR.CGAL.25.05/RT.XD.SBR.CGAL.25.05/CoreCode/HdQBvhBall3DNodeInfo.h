#pragma once

#include<set>
#include"HdQBoundingBox3D.h"
#include"HdQBall3D.h"

namespace BvhBall3DNodeInfoStd {

	/// <summary>
	/// bvh树结点信息
	/// </summary>
	class BvhBall3DNodeInfo
	{
	public:

		/// <summary>
		/// box3D 的外接球
		/// </summary>
		Ball3DStd::Ball3D ball3D;

		void ReBuildByBox3D(const BoundingBox3DStd::BoundingBox3D& box3D);

		BvhBall3DNodeInfo();
		BvhBall3DNodeInfo(int level, const BoundingBox3DStd::BoundingBox3D& box3D);
		~BvhBall3DNodeInfo();

		int GetLevel() const; 
		void SetLevel(int level);
		int GetId();
		BoundingBox3DStd::BoundingBox3D GetBox() const;
	private:
		int id;
		int level;
		BoundingBox3DStd::BoundingBox3D box3D;

	};


}