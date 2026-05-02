#pragma once

#include <iostream>
#include <vector>

#include"MatrixObject.h"
#include"MatrixOperator.h"

namespace TestStd {

	int demo01()
	{
		std::cout << "*********************" << std::endl;
		double mb[5][7] = { {1, 2, 3, 4, 5, 6, 7}, { 8, 9, 10, 11, 12, 13, 14 }, {15, 16, 17, 18, 19, 20, 21}, { 22, 23, 24, 25, 26, 27, 28 }, { 29, 30, 31, 32, 33, 34, 35 } };
		MatrixObjectStd::MatrixObject mB;
		mB = MatrixObjectStd::Init((double*)mb, 5, 7);
		std::cout << "Ô­¾ØÕóA1:" << std::endl;
		MatrixObjectStd::print(mB);
		MatrixObjectStd::MatrixObject pinv_mB = MatrixOperatorStd::pinv(mB);
		std::cout << "¾ØÕóA1µÄ¹ãÒåÄæ¾ØÕó:" << std::endl;
		MatrixObjectStd::print(pinv_mB);
		std::cout << "*********************" << std::endl;
		double ma[6][4] = { { 11,12,13,14 },{ 15,16,17,18 },{ 19,20,21,22 },{ 23,24,25,26 },{ 27,28,29,30 },{ 31,32,33,34 } };
		MatrixObjectStd::MatrixObject mA;
		mA = MatrixObjectStd::Init((double*)ma, 6, 4);
		std::cout << "Ô­¾ØÕóA2:" << std::endl;
		MatrixObjectStd::print(mA);
		MatrixObjectStd::MatrixObject pinv_mA = MatrixOperatorStd::pinv(mA);
		std::cout << "¾ØÕóA2µÄ¹ãÒåÄæ¾ØÕó:" << std::endl;
		MatrixObjectStd::print(pinv_mA);
		return 0;
	}

}