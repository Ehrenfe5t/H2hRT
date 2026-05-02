


#include"DxQTransmittingAntennaJsonFile.h"


namespace TransmittingAntennaJsonFileStd {


	TransmittingAntennaJsonFile::TransmittingAntennaJsonFile()
	{
		this->transmittingAntennaId = 0;
		this->radiationPatternId = -1;
		this->polarization3DModelId = 1;
		this->emissionPower = 0.001;
		this->center_location_x = 0;
		this->center_location_y = 0;
		this->center_location_z = 0;
		this->init_frequency = false;
		this->frequencys.emplace_back(3e9);
		this->inputReceivingAntennaCsvFileName = "ReceivingAntenna.csv";
	}

	TransmittingAntennaJsonFile::~TransmittingAntennaJsonFile()
	{
	}



	void TransmittingAntennaJsonFile::AddFrequency(double frequency_)
	{
		if (this->init_frequency) {
			this->frequencys.emplace_back(frequency_);
		}
		else {
			this->init_frequency = true;
			this->frequencys.clear();
			this->frequencys.emplace_back(frequency_);
		}
	}

	void TransmittingAntennaJsonFile::AddFrequencys(const std::vector<double>& frequencys_)
	{
		for (int i = 0; i < frequencys_.size(); ++i) {
			AddFrequency(frequencys_[i]);
		}
	}

}