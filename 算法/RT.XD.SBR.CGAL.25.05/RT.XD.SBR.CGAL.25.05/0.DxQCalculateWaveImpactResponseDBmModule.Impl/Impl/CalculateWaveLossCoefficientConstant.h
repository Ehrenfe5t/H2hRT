#pragma once


namespace CalculateWaveLossCoefficientStd {

	//约0.1度
	const double MIN_ARG = 0.0016;

	const double C = 299792458;

	const double PI = 3.14159265358979323846;
	const double TWO_PI = 2.0 * 3.14159265358979323846;

	const double EPSILON = 1e-6;

	//最大路径损耗(dBm)
	const double MAX_PATH_LOSS = 327.0;

	//真空介电常数
	const double EPSILON_0 = 8.854187817620389e-12;

	//真空磁导率
	const double MU_0 = 4.0e-7 * PI;

}