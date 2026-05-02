


#include"HdQChannelDataInt.h"

#include"DxQStructureToString.h"

namespace ChannelDataIntStd {



	ChannelDataInt::ChannelDataInt()
	{
        this->id_tx = -1;
        this->id_rx = -1;
        this->value = -1;
	}

	ChannelDataInt::~ChannelDataInt()
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

    std::string ChannelDataDouble_To_string(const std::vector<ChannelDataIntStd::ChannelDataInt>& bigChannelParameters) {

        std::string context;
        for (int i = 0; i < (int)bigChannelParameters.size() - 1; i++) {
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_tx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_tx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_rx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_rx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].value)); context.append(",");

            context.append("\n");
        }
        if (bigChannelParameters.size() > 0) {
            int i = (int)bigChannelParameters.size() - 1;
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_tx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_tx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].id_rx)); context.append(",");
            context.append(Point3DToString(bigChannelParameters[i].location_rx, ",", "", 16)); context.append(",");
            context.append(StructureToStringStd::IntToStringByStringStream(bigChannelParameters[i].value));
        }

        return context;
    }
}