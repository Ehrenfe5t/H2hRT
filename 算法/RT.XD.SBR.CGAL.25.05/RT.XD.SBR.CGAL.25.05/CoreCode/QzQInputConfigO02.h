#pragma once


namespace InputConfigO02Std {

	class InputConfigO02
	{
	public:
		bool isLos;
		bool isDiffractionReflection;
		bool rebuildEdge;
		int airSubstanceType;
		int electricFieldCalculationMode;
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
		InputConfigO02();
		~InputConfigO02();

	private:

	};


}