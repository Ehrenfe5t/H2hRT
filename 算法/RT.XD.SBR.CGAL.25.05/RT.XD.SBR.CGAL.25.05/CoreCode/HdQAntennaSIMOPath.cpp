#include "HdQAntennaSIMOPath.h"


#include <mutex>

namespace AntennaSIMOPathStd {


	std::mutex mtx;

	AntennaSIMOPath::AntennaSIMOPath()
	{
		this->transmittingAntennaId = -1;
	}

	AntennaSIMOPath::AntennaSIMOPath(int transmittingAntennaId, std::vector<AntennaSISOPathStd::AntennaSISOPath>& paths)
	{
		this->transmittingAntennaId = transmittingAntennaId;
		this->paths = paths;
	}

	AntennaSIMOPathStd::AntennaSIMOPath::~AntennaSIMOPath()
	{
	}

	int AntennaSIMOPath::FindIndex(int receiverAntennaId)
	{
		std::lock_guard<std::mutex> lock(mtx);
		for (int i = 0; i < this->paths.size(); ++i) {
			if (receiverAntennaId == this->paths[i].receiverAntennaId) {
				return i;
			}
		}
		return -1;
	}

	void AntennaSIMOPath::AddPath(int receiverAntennaId, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path)
	{
		int index = FindIndex(receiverAntennaId);
		if (index == -1) {
			return;
		}
		std::lock_guard<std::mutex> lock(mtx);
		this->paths[index].AddPath(path);
	}

	void AntennaSIMOPath::AddPathByLoop(int index, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path)
	{
		std::lock_guard<std::mutex> lock(mtx);
		this->paths[index].AddPath(path);
	}



}