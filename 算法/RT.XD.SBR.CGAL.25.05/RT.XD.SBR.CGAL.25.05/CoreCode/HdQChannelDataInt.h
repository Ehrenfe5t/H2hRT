#pragma once

#include"LxQPoint3D.h"
#include<vector>
namespace ChannelDataIntStd {

	class ChannelDataInt
	{
	public:
		int id_tx;
		int id_rx;
		Point3DStd::Point3D location_tx;
		Point3DStd::Point3D location_rx;
		int value;
		ChannelDataInt();
		~ChannelDataInt();

	private:

	};


	std::string ChannelDataDouble_To_string(const std::vector<ChannelDataIntStd::ChannelDataInt>& bigChannelParameters);

}