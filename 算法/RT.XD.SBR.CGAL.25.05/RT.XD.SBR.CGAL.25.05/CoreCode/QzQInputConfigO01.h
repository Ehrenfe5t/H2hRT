#pragma once


namespace InputConfigO01Std {

	class InputConfigO01
	{
	public:
		bool rebuildEdge;
		int airSubstanceType;
		int ejectionsMaxTotalNumber;
		int ejectionsOfDiffractionMaxNumber;
		int ejectionsOfReflectionMaxNumber;
		int ejectionsOfTransmissionMaxNumber;
		int multithreadConfigSwitchOfMultithread;
		int multithreadConfigThreadNum;
		int multithreadConfigThreadOneCpuCalNum;
		int spaceSubregionConfigType;
		double spaceSubregionConfigStepLength;
		double powerThreshold;
		InputConfigO01();
		~InputConfigO01();

	private:

	};


}