#pragma once

#include"../Input.h"

namespace TransmittingAntennaDatabaseObjectStd {

	class TransmittingAntennaDatabaseObject
	{
	public:

		/// <summary>
		/// true表示合法，false表示非法
		/// </summary>
		bool legal;

		TransmittingAntennaStd::TransmittingAntenna transmittingAntenna;

		TransmittingAntennaDatabaseObject();
		~TransmittingAntennaDatabaseObject();

		void SetTransmittingAntenna(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna);
	private:

	};

}