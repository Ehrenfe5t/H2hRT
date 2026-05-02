#pragma once

#include<string>

namespace DataOutputParameterConfigStd {

	class DataOutputParameterConfig
	{
	public:

        /// <summary>
        /// 输出文件夹名称
        /// </summary>
        std::string outPutDirectoryPathName;

        /// <summary>
        /// 输出日志文件名
        /// </summary>
        std::string outPutLogTxtFileName;

        /// <summary>
        /// 是否输出大尺度信息(包括接收功率、路径损耗)
        /// </summary>
        bool switchOfBigChannelParameterInfo;

        /// <summary>
        /// 多信号源叠加
        /// </summary>
        bool switchOfMultipleSignalSourceSuperposition;

        /// <summary>
        /// 是否输出路径信息,仅几何路径，该路径可支持重新计算场
        /// </summary>
        bool switchOfPathInfo;

        /// <summary>
        /// 小尺度信息配置
        /// </summary>
        bool switchOfSmallChannelParameterInfo;

        /// <summary>
        /// 是否输出统计信息(包括频率、发射与及接收机的直线距离、打穿面元数量(发射与及接收机的直线)、接收总功率、平均附加时延、均方根时延扩展)
        /// </summary>
        bool switchOfStatisticChannelParameterInfo;

        

		DataOutputParameterConfig();
		~DataOutputParameterConfig();

	private:

	};


}