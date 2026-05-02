#pragma once

#include"LxQMultiPathNodeInfo.h"
#include"DxQTransmittingAntenna.h"

namespace AntennaSISOOnePathStd {

	/// <summary>
	/// µ¥Ò»Â·¾¶
	/// </summary>
	class AntennaSISOOnePath
	{
	public:
		std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*> path;
		AntennaSISOOnePath();
		AntennaSISOOnePath(std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& nodes);
		~AntennaSISOOnePath();

		int GetTransmittingAntennaId() const;

		bool GetTransmittingAntenna(TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna) const;

		double pathLength;

		void CalPathLength();

	private:

	};





}