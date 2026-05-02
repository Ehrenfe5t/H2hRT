#pragma once

#include"LxQPoint3D.h"
#include"LxQPropagationType.h"

namespace GeometryRTMultiPathBaseNodeStd {

	class GeometryRTMultiPathBaseNode
	{
	public:

		Point3DStd::Point3D location;
		// 该语句使编译器为 Root 类生成一个默认构造函数
		GeometryRTMultiPathBaseNode();
		// 该函数计算并返回该书籍的销售额
		virtual PropagationTypeStd::PropagationType GetPropagationType() const;
		// 析构函数
		~GeometryRTMultiPathBaseNode();

	private:

	};

	void Free(std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& path);

	void Free(std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void DeleteSamePath(std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

}