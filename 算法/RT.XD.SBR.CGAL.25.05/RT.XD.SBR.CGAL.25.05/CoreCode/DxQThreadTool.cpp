

#include"DxQThreadTool.h"

namespace ThreadToolStd {


	void GetRunningThread(std::vector <std::thread>& threads) {

		for (int i = 0; i < threads.size(); ++i) {
			if (threads[i].joinable()) {
				threads[i].join();
			}
		}
	}
}