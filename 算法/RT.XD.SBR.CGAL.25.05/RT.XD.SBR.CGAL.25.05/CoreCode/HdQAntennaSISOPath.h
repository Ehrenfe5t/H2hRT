#pragma once

#include"HdQAntennaSISOOnePath.h"

namespace AntennaSISOPathStd {

	class AntennaSISOPath
	{
	public:
		int receiverAntennaId;

		Point3DStd::Point3D rx_location;

		std::vector<AntennaSISOOnePathStd::AntennaSISOOnePath> paths;
		AntennaSISOPath();
		~AntennaSISOPath();
		AntennaSISOPath(int receiverAntennaId, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path);

		void AddPath(const AntennaSISOOnePathStd::AntennaSISOOnePath& path);
	private:

	};

}