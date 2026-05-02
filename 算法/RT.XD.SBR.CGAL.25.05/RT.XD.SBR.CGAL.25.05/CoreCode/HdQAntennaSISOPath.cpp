

#include"HdQAntennaSISOPath.h"


#include <mutex>

namespace AntennaSISOPathStd {


	std::mutex mtx;

	AntennaSISOPath::AntennaSISOPath()
	{
		this->receiverAntennaId = -1;
	}

	AntennaSISOPath::~AntennaSISOPath()
	{
	}

	AntennaSISOPath::AntennaSISOPath(int receiverAntennaId, std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path)
	{
		this->receiverAntennaId = receiverAntennaId;
		std::lock_guard<std::mutex> lock(mtx);
		this->paths.emplace_back(path);
	}

	void AntennaSISOPath::AddPath(const AntennaSISOOnePathStd::AntennaSISOOnePath& path)
	{
		bool flag = true;
		for (int i = 1; i < (int)path.path.size() - 1; ++i) {

			if (path.path[i - 1]->type != PropagationTypeStd::PropagationType::Transmission) {
				if (path.path[i]->type == PropagationTypeStd::PropagationType::Transmission) {
					if (path.path[i + 1]->type != PropagationTypeStd::PropagationType::Transmission) {
						flag = false;
						break;
					}
				}
			}
		}
		if (flag) {
			std::lock_guard<std::mutex> lock(mtx);
			this->paths.emplace_back(path);
		}
	}


}