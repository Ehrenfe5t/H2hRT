
#include "Input.h"

#include <iostream>

void PrintMaterialSet(const MaterialSet & materialSet) {
	std::cout << std::defaultfloat; // 取消固定小数
	std::cout << std::setprecision(16);
	//高精度打印数据
	for (int i = 0; i < materialSet.size; i++) {
		const Material & material = materialSet.materials[i];
		std::cout << "(" << material.materialTypeNumber << "," << material.frequency << "," << material.relativePermittivity << "," << material.conductivity << ")" << std::endl;
	}
	
}