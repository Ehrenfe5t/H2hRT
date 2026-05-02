


#include"HdQBvhBall3DNodeInfo.h"

namespace BvhBall3DNodeInfoStd {

	static int startBvhBall3DNodeInfoId = 0;


	void BvhBall3DNodeInfo::ReBuildByBox3D(const BoundingBox3DStd::BoundingBox3D& box3D)
	{
		this->box3D = box3D;
		this->ball3D.center.x = 0.5 * (box3D.min.x + box3D.max.x);
		this->ball3D.center.y = 0.5 * (box3D.min.y + box3D.max.y);
		this->ball3D.center.z = 0.5 * (box3D.min.z + box3D.max.z);
		auto dx = box3D.max.x - box3D.min.x;
		auto dy = box3D.max.y - box3D.min.y;
		auto dz = box3D.max.z - box3D.min.z;
		this->ball3D.radius = sqrt(dx * dx + dy * dy + dz * dz);
	}

	BvhBall3DNodeInfo::BvhBall3DNodeInfo()
	{
		this->id = startBvhBall3DNodeInfoId++;
		this->level = 0;
	}

	BvhBall3DNodeInfo::BvhBall3DNodeInfo(int level, const BoundingBox3DStd::BoundingBox3D& box3D)
	{
		this->id = startBvhBall3DNodeInfoId++;
		this->level = level;

		this->box3D = box3D;
		this->ball3D.center.x = 0.5 * (box3D.min.x + box3D.max.x);
		this->ball3D.center.y = 0.5 * (box3D.min.y + box3D.max.y);
		this->ball3D.center.z = 0.5 * (box3D.min.z + box3D.max.z);
		auto dx = box3D.max.x-box3D.min.x;
		auto dy = box3D.max.y-box3D.min.y;
		auto dz = box3D.max.z-box3D.min.z;
		this->ball3D.radius = sqrt(dx * dx + dy * dy + dz * dz);
	}

	BvhBall3DNodeInfo::~BvhBall3DNodeInfo()
	{
	}


	void BvhBall3DNodeInfo::SetLevel(int level) {
		this->level = level;
	}
	int BvhBall3DNodeInfo::GetLevel()  const
	{
		return this->level;
	}

	int BvhBall3DNodeInfo::GetId()
	{
		return this->id;
	}
	BoundingBox3DStd::BoundingBox3D BvhBall3DNodeInfo::GetBox() const
	{
		return this->box3D;
	}
}