#pragma once


namespace MultithreadParameterConfigStd {

	class MultithreadParameterConfig
	{
	public:
        /// <summary>
        /// 是否开启多线程。
        /// </summary>
        bool multithreadConfigSwitchOfMultithread;

        /// <summary>
        /// 多线程开启时的线程数量。
        /// </summary>
        int multithreadConfigThreadNum;

        /// <summary>
        /// 多线程开启时，单个线程单次分配的任务量。
        /// </summary>
        int multithreadConfigThreadOneCpuCalNum;

        MultithreadParameterConfig();
		~MultithreadParameterConfig();

	private:

	};


}