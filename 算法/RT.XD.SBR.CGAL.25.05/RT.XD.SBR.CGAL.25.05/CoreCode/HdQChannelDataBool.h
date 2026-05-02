#pragma once


#include"LxQPoint3D.h"
#include<vector>

namespace ChannelDataBoolStd {

	class ChannelDataBool
	{
	public:
		int id_tx;
		int id_rx;
		Point3DStd::Point3D location_tx;
		Point3DStd::Point3D location_rx;
		bool value;
		ChannelDataBool();
		~ChannelDataBool();

	private:

	};


	std::string ChannelDataBool_To_string(const std::vector<ChannelDataBoolStd::ChannelDataBool>& bigChannelParameters);



}