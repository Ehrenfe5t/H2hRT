

#include"HdQAntennaSISOOnePath.h"

#include "LxQMultiPathNodeInfoOperate.Ptr.h"
#include "QzQGlobalConstant.h"
#include "DxQTransmittingAntennaDatabase.h"
#include "QzQGeometry3DOperate.Distance.h"

namespace AntennaSISOOnePathStd {


	AntennaSISOOnePath::AntennaSISOOnePath()
	{
		this->pathLength = GlobalConstantStd::BoundingBoxLength;
	}
	AntennaSISOOnePath::AntennaSISOOnePath(std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& nodes)
	{
		this->pathLength = GlobalConstantStd::BoundingBoxLength;
		this->path.clear();

		for (int loop = 0; loop < nodes.size(); ++loop) {
			this->path.emplace_back(nodes[loop]);
		}

	}

	AntennaSISOOnePath::~AntennaSISOOnePath()
	{
		//MultiPathNodeInfoOperateStd::FreeMultiPathNodeInfo_vector(this->path);
	}

	int AntennaSISOOnePath::GetTransmittingAntennaId() const
	{
		if (this->path.size() < 1) {
			return -1;
		}
		MultiPathNodeInfoStd::MultiPathNodeInfo* txNode = this->path[0];
		if (txNode->type != PropagationTypeStd::PropagationType::TransmittingAntenna) {
			return -1;
		}
		auto transmittingAntenna_ptr = MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmittingAntenna_ptr(txNode);

		return transmittingAntenna_ptr->baseStationAntennaID;
	}

	bool AntennaSISOOnePath::GetTransmittingAntenna(TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna) const
	{
		int transmittingAntennaId = GetTransmittingAntennaId();
		if (transmittingAntennaId == -1) {
			return false;
		}
		if (!TransmittingAntennaDatabaseStd::FindTransmittingAntenna(transmittingAntennaId, transmittingAntenna)) {
			return false;
		}
		return true;
	}

	void AntennaSISOOnePath::CalPathLength()
	{
		if (this->path.size()<2) {
			return;
		}
		this->pathLength = 0.0;
		for (int loop = 1; loop < this->path.size(); ++loop) {
			auto pre_location = this->path[loop - 1]->location;
			auto cur_location = this->path[loop]->location;
			this->pathLength += Geometry3DOperateStd::GetDistancePoint3DPoint3D(pre_location, cur_location);
		}
	}


}