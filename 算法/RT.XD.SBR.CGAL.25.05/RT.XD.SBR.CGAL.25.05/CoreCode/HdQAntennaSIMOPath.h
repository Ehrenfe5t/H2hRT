#pragma once

#include"HdQAntennaSISOPath.h"
#include"LxQMultiPathNodeInfo.h"

namespace AntennaSIMOPathStd {

	class AntennaSIMOPath
	{
	public:
		int transmittingAntennaId;
		std::vector<AntennaSISOPathStd::AntennaSISOPath> paths;
		AntennaSIMOPath();
		AntennaSIMOPath(int transmittingAntennaId,std::vector<AntennaSISOPathStd::AntennaSISOPath>& paths);
		~AntennaSIMOPath();

		int FindIndex(int receiverAntennaId);
		void AddPath(int receiverAntennaId, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path);
		void AddPathByLoop(int index, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path);
	private:

	};


}