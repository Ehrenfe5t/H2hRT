#include"HdQAntennaPatternObject.h"

#include"QzQGlobalConstant.h"
#include"LxQPoint3D.h"

#include <mutex>
namespace AntennaPatternObjectStd {

	std::vector<double**> All_radiationPattern_ptr;
	void Free_All_radiationPattern_ptr() {

		while (!All_radiationPattern_ptr.empty())
		{
			auto ptr = All_radiationPattern_ptr.back();
			All_radiationPattern_ptr.pop_back();
			for (int i = 0; i < radiationPattern_rows; ++i) {
				delete ptr[i];
			}
			delete ptr;
		}

	}

	std::mutex mtx_node_all;
	AntennaPatternObject::AntennaPatternObject()
	{
		this->radiationPatternId = -1;

		this->radiationPattern = (double**)malloc(radiationPattern_rows*sizeof(double*));
		if (this->radiationPattern == NULL) {
			return;
		}
		for (int i = 0; i < radiationPattern_rows; ++i) {
			this->radiationPattern[i] = (double*)malloc(radiationPattern_cols * sizeof(double));
			if (this->radiationPattern[i] == NULL) {
				return;
			}
		}

		for (int i = 0; i < radiationPattern_rows; ++i) {
			for (int j = 0; j < radiationPattern_cols; ++j) {
				this->radiationPattern[i][j] = 0.0;
			}
		}
		std::lock_guard<std::mutex> lock(mtx_node_all);
		//All_radiationPattern_ptr.emplace_back(this->radiationPattern);
	}
	AntennaPatternObject::~AntennaPatternObject()
	{

	}



	void AntennaPatternObject::UpdateData(const AntennaPatternObject& antennaPatternObject)
	{
		this->radiationPatternId = antennaPatternObject.radiationPatternId;
		for (int i = 0; i < radiationPattern_rows; ++i) {
			for (int j = 0; j < radiationPattern_cols; ++j) {
				this->radiationPattern[i][j] = antennaPatternObject.radiationPattern[i][j];
			}
		}
	}


	void AntennaPatternObject::UpdateData1(int radiationPatternId, const std::vector<std::vector<double>>& radiationPattern)
	{
		this->radiationPatternId = radiationPatternId;
		for (int i = 0; i < radiationPattern_rows; ++i) {
			for (int j = 0; j < radiationPattern_cols; ++j) {
				this->radiationPattern[i][j] = radiationPattern[i][j];
			}
		}
	}

	void AntennaPatternObject::UpdateData2(int radiationPatternId, double* radiationPattern)
	{
		this->radiationPatternId = radiationPatternId;
		int c = 0;
		for (int i = 0; i < radiationPattern_rows; ++i) {
			for (int j = 0; j < radiationPattern_cols; ++j) {
				this->radiationPattern[i][j] = radiationPattern[i * 181 + j];
			}
		}
	}






	void CalThetaAndPhi(const Point3DStd::Point3D& unit_vec, double& theta, double& phi) {

		theta = std::acos(unit_vec.z);
		if (unit_vec.x == 0.0 && unit_vec.y == 0.0) {
			phi = 0.0;
		}
		else {
			phi = std::atan2(unit_vec.y, unit_vec.x);
			if (phi < 0) {
				phi = phi + 2 * GlobalConstantStd::Pi;
			}
		}

	}

	double GetRadiationPatternByVector(const AntennaPatternObject& antennaPatternObject, const Point3DStd::Point3D& vector)
	{
		//计算出来的范围[0,GlobalConstantStd::Pi]
		double theta;
		//计算出来的范围[0,2.0*GlobalConstantStd::Pi)
		double phi;
		CalThetaAndPhi(vector, theta, phi);
		phi = phi * 180.0 / GlobalConstantStd::Pi;
		theta = theta * 180.0 / GlobalConstantStd::Pi;
		int indexLeft = (int)(phi);
		int indexRight = (indexLeft + 1) % 360;
		int indexTop = (int)(theta);
		int indexDown = indexTop + 1;
		if (indexDown > 180) {
			indexDown = 180;
		}
		if (indexLeft < 0 || indexLeft>359) {
			return 0.0;
		}
		if (indexTop < 0 || indexTop>359) {
			return 0.0;
		}
		double value1 = antennaPatternObject.radiationPattern[indexLeft][indexTop];
		double area1 = abs((phi - indexLeft) * (theta - indexTop));
		if (area1 == 0.0) {
			return value1;
		}
		double value2 = antennaPatternObject.radiationPattern[indexRight][indexTop];
		double area2 = abs((indexLeft + 1 - phi) * (theta - indexTop));
		if (area2 == 0.0) {
			return value2;
		}
		double value3 = antennaPatternObject.radiationPattern[indexLeft][indexDown];
		double area3 = abs((phi - indexLeft) * (indexTop + 1 - theta));
		if (area3 == 0.0) {
			return value3;
		}
		double value4 = antennaPatternObject.radiationPattern[indexRight][indexDown];
		double area4 = abs((indexLeft + 1 - phi) * (indexTop + 1 - theta));
		if (area4 == 0.0) {
			return value4;
		}
		double area = area1 + area2 + area3 + area4;
		double value = value1 * area1 / area + value2 * area2 / area +
			value3 * area3 / area + value4 * area4 / area;
		return value;
	}



}
