

#include"HdQChannelDataDouble.h"
#include"DxQStructureToString.h"

#include"QzQFileBase.h"
namespace ChannelDataDoubleStd {


	ChannelDataDouble::ChannelDataDouble()
	{
		this->id_tx = -1;
		this->id_rx = -1;
		this->value = 1e7;
	}

	ChannelDataDouble::~ChannelDataDouble()
	{
	}





    /// <summary>
    /// 点转化为字符串
    /// </summary>
    /// <param name="p"></param>
    /// <param name="midGap"></param>
    /// <param name="endGap"></param>
    /// <returns></returns>
    std::string Point3DToString(const Point3DStd::Point3D& p, const std::string& midGap, const std::string& endGap, int doubleSize) {
        std::string res = "";
        res.append(StructureToStringStd::DoubleToStringByStringStream(p.x, doubleSize)); res.append(midGap);
        res.append(StructureToStringStd::DoubleToStringByStringStream(p.y, doubleSize)); res.append(midGap);
        res.append(StructureToStringStd::DoubleToStringByStringStream(p.z, doubleSize)); res.append(endGap);
        return res;
    }



    std::string ChannelDataDouble_To_string_MOMI(
        const std::vector<int>& receiverAntennaIds,
        const std::vector<Point3DStd::Point3D>& rx_locations,
        const std::vector<double>& bigChannelParameters) {

        std::string context;
        for (int i = 0; i < (int)bigChannelParameters.size() - 1; i++) {
            context.append(StructureToStringStd::IntToStringByStringStream(receiverAntennaIds[i])); context.append(",");
            context.append(Point3DToString(rx_locations[i], ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters[i], 16)); context.append(",");

            context.append("\n");
        }
        if (bigChannelParameters.size() > 0) {
            int i = (int)bigChannelParameters.size() - 1;
            context.append(StructureToStringStd::IntToStringByStringStream(receiverAntennaIds[i])); context.append(",");
            context.append(Point3DToString(rx_locations[i], ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters[i], 16)); context.append(",");
        }

        return context;
    }

    std::string ChannelDataDouble_To_string(const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string context;
        for (int i = 0; i < (int)bigChannelParameters.size() - 1; i++) {
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_tx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_tx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_rx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_rx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters[i].value, 16)); context.append(",");

            context.append("\n");
        }
        if (bigChannelParameters.size() > 0) {
            int i = (int)bigChannelParameters.size() - 1;
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_tx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_tx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_rx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_rx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters[i].value, 16));
        }

        return context;
    }


    std::vector<ChannelDataDoubleStd::ChannelDataDouble> GetVector(
        const std::vector<int>& transmittingAntennaIds,
        const std::vector<Point3DStd::Point3D>& tx_locations,
        const std::vector<int>& receiverAntennaIds,
        const std::vector<Point3DStd::Point3D>& rx_locations,
        const std::vector<double>& bigChannelParameters) {

        std::vector<ChannelDataDoubleStd::ChannelDataDouble> channelDataDoubles(transmittingAntennaIds.size());
        for (int i = 0;i< transmittingAntennaIds.size();++i) {
            channelDataDoubles[i].id_tx = transmittingAntennaIds[i];
            channelDataDoubles[i].location_tx = tx_locations[i];
            channelDataDoubles[i].id_rx = receiverAntennaIds[i];
            channelDataDoubles[i].location_rx = rx_locations[i];
            channelDataDoubles[i].value = bigChannelParameters[i];
        }
        return channelDataDoubles;
    }

    void WriteToTxtFile(const std::string& fileNameWithoutEnd,
        const  std::string& excelHeader, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& channelDataDoubles) {

        std::string context = ChannelDataDouble_To_string(channelDataDoubles);

        std::string csv_fileName = fileNameWithoutEnd + ".csv";
        std::string txt_fileName = fileNameWithoutEnd + ".txt";

        FileOperateStd::WriteStringToTxtFile(csv_fileName.c_str(), excelHeader + context);
        FileOperateStd::WriteStringToTxtFile(txt_fileName.c_str(), context);
    }


}