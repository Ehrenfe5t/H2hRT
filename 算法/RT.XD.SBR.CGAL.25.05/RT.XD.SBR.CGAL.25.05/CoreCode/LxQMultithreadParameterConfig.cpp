#include "LxQMultithreadParameterConfig.h"




namespace MultithreadParameterConfigStd {
	
	MultithreadParameterConfig::MultithreadParameterConfig()
	{
		this->multithreadConfigSwitchOfMultithread = true;
		this->multithreadConfigThreadNum = 8;
		this->multithreadConfigThreadOneCpuCalNum = 50;
	}
	MultithreadParameterConfig::~MultithreadParameterConfig()
	{
	}
}