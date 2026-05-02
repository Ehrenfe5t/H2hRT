#include"HdQAntennaMIMOPath.h"

#include"LxQMultiPathNodeInfoOperate.Ptr.h"


#include"QzQGeometry3DOperate.Distance.h"

namespace AntennaMIMOPathStd {
	AntennaMIMOPath::AntennaMIMOPath()
	{
	}
	AntennaMIMOPath::~AntennaMIMOPath()
	{
	}


	

	bool FindCenterMultipathByDeduplicateRadius_Path(
		const AntennaSISOOnePathStd::AntennaSISOOnePath& antennaSISOOnePath1,
		const AntennaSISOOnePathStd::AntennaSISOOnePath& antennaSISOOnePath2,
		double deduplicateRadius) {

		//这里不能删除长度小于2的路径
		if (antennaSISOOnePath1.path.size() < 2) {
			return false;
		}
		if (antennaSISOOnePath1.path.size() != antennaSISOOnePath2.path.size()) {
			return false;
		}

		for (int loop = 0; loop < antennaSISOOnePath1.path.size(); ++loop) {
			auto node_1 = antennaSISOOnePath1.path[loop];
			auto node_2 = antennaSISOOnePath2.path[loop];
			if (node_1->type != node_2->type) {
				return false;
			}

			auto location1 = node_1->location;
			auto location2 = node_2->location;
			double distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(location1, location2);
			if (distance > deduplicateRadius) {
				return false;
			}

		}

		return true;
	}

	void FindCenterMultipathByDeduplicateRadius_SISO(
		AntennaSISOPathStd::AntennaSISOPath& antennaSISOPath, 
		double deduplicateRadius) {

		for (int loop = 0; loop < antennaSISOPath.paths.size(); ++loop) {
			antennaSISOPath.paths[loop].CalPathLength();
		}

		//按照长度排序

		std::sort(antennaSISOPath.paths.begin(), antennaSISOPath.paths.end(), [](
			const AntennaSISOOnePathStd::AntennaSISOOnePath& a,
			const AntennaSISOOnePathStd::AntennaSISOOnePath& b) {
				if (a.pathLength <= b.pathLength) {
					return true;
				}
				return false;
			});

		if (antennaSISOPath.paths.size()<2) {
			return;
		}

		for (int j = (int)antennaSISOPath.paths.size() - 1; j > 0; j--) {
			bool state = false;
			for (int i = 0; i < j; ++i) {
				if (FindCenterMultipathByDeduplicateRadius_Path(antennaSISOPath.paths[j], antennaSISOPath.paths[i], deduplicateRadius)) {
					state = true;
					break;
				}
			}
			if (state) {

				antennaSISOPath.paths.erase(antennaSISOPath.paths.begin()+j);

			}

		}
		
	}

	void FindCenterMultipathByDeduplicateRadius_SIMO(AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath, double deduplicateRadius) {
		
		for (int i = 0; i < antennaSIMOPath.paths.size(); ++i) {
			FindCenterMultipathByDeduplicateRadius_SISO(antennaSIMOPath.paths[i], deduplicateRadius);
		}

	}

	void FindCenterMultipathByDeduplicateRadius(AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath, double deduplicateRadius) {

		for (int i = 0;i< antennaMIMOPath.paths.size();++i) {
			FindCenterMultipathByDeduplicateRadius_SIMO(antennaMIMOPath.paths[i], deduplicateRadius);
		}

	}
}
