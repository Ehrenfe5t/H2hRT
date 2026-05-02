

#include"LxQMultiLinearPolarization3DObjectDatabase.h"
#include"QzQGlobalConstant.h"

namespace MultiLinearPolarization3DObjectDatabaseStd {



	MultiLinearPolarization3DObjectDatabase::MultiLinearPolarization3DObjectDatabase()
	{
		std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject> multiLinearPolarization3DObjects;
		BaseDescribeMultiLinearPolarization3DObjectDatabase2(multiLinearPolarization3DObjects);
		AddAll(multiLinearPolarization3DObjects);
	}

	MultiLinearPolarization3DObjectDatabase::~MultiLinearPolarization3DObjectDatabase()
	{
	}

	int MultiLinearPolarization3DObjectDatabase::IndexOf(int polarization3DModelId) const {
		for (int i = 0; i < database.size(); ++i) {
			if (polarization3DModelId == database[i].polarization3DModelId) {
				return i;
			}
		}
		return -1;
	}

	void MultiLinearPolarization3DObjectDatabase::Add(const MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject& multiLinearPolarization3DObject)
	{
		if (multiLinearPolarization3DObject.polarization3DModelId == -1) {
			return;
		}
		if (multiLinearPolarization3DObject.polarization3DModelId == 0) {
			return;
		}

		int index = IndexOf(multiLinearPolarization3DObject.polarization3DModelId);

		if (index == -1) {
			database.emplace_back(multiLinearPolarization3DObject);
		}
		else {

		}

	}

	void MultiLinearPolarization3DObjectDatabase::AddAll(const std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject>& multiLinearPolarization3DObjects)
	{
		for (int i = 0;i< multiLinearPolarization3DObjects.size();++i) {
			Add(multiLinearPolarization3DObjects[i]);
		}
	}


	//写一个描述，
	//这个文件描述了天线极化的数据库，
	//polarization3DModelId表示唯一编号
	//1. polarization3DModelId==-1表示非法，请不要设置它的值为-1
	//2. polarization3DModelId==0表示全极化，不需要设置初始相位和极化方向以及能量占比
	//3. polarization3DModelId==1表示默认的单一线极化,初始相位为0，极化方向为(0,0,1),能量占比为100%
	//4. polarization3DModelId==2表示默认的双线极化表示的一种左旋圆极化，
	//   第1个线极化的初始相位为0，极化方向为(0,0,1)，能量占比为50%
	//   第2个线极化的初始相位为0.5*pi，极化方向为(0,1,0)，能量占比为50%
	//<<polarization3DModelId不能设置为-1，0，1，2>>
	//

	void BaseDescribeMultiLinearPolarization3DObjectDatabase1(
		std::string& describe) {
		describe.clear();
		//describe.append("这个文件描述了天线极化的数据库\n");
		describe.append("(1).This document describes the database of antenna polarization. (2).");
		//describe.append("polarization3DModelId表示唯一编号,如果出现重复则后面的不起作用，只以第一次初始化为准\n");
		describe.append("The polarization3DModelId represents a unique identifier. If there is a repetition, the subsequent ones will be invalid, and only the first initialization will be taken as the standard. (3).");
		//describe.append("1. polarization3DModelId==-1表示非法，请不要设置它的值为-1\n");
		describe.append("If polarization3DModelId == -1, it indicates an illegal value. Please do not set its value to -1. (4).");
		//describe.append("2. polarization3DModelId==0表示全极化，不需要设置初始相位和极化方向以及能量占比\n");
		describe.append("When polarization3DModelId == 0, it indicates full polarization, and there is no need to set the initial phase, polarization direction, or energy proportion. (5).");
		//describe.append("3. polarization3DModelId==1表示默认的单一线极化,初始相位为0，极化方向为(0,0,1),能量占比为100%\n");
		describe.append("When polarization3DModelId == 1, it indicates the default single linear polarization, with an initial phase of 0, polarization direction of (0, 0, 1), and energy proportion of 100%. (6).");
		//describe.append("4. polarization3DModelId==2表示默认的双线极化表示的一种左旋圆极化，\n");
		//describe.append("\t第1个线极化的初始相位为0，极化方向为(0,0,1)，能量占比为50%\n");
		//describe.append("\t第2个线极化的初始相位为0.5*pi，极化方向为(0,1,0)，能量占比为50%\n");
		describe.append("When polarization3DModelId == 2, it indicates a default dual-linear polarization representation of a left-handed circular polarization. The initial phase of the first linear polarization is 0, with a polarization direction of (0, 0, 1) and an energy proportion of 50%. The initial phase of the second linear polarization is 0.5 * pi, with a polarization direction of (0, 1, 0) and an energy proportion of 50%. (7).");
		//describe.append("======polarization3DModelId不能设置为-1，0，1，2====\n");
		describe.append("In conclusion, the polarization3DModelId cannot be set to -1, 0, 1, or 2.");

	}


	void BaseDescribeMultiLinearPolarization3DObjectDatabase3(
		std::string& a1_describe,
		std::string& a2_describe,
		std::string& a3_describe,
		std::string& a4_describe,
		std::string& a5_describe,
		std::string& a6_describe,
		std::string& a7_describe) {

		a1_describe.clear();
		a2_describe.clear();
		a3_describe.clear();
		a4_describe.clear();
		a5_describe.clear();
		a6_describe.clear();
		a7_describe.clear();

		//describe.append("这个文件描述了天线极化的数据库\n");
		a1_describe.append("(1).This document describes the database of antenna polarization. ");
		//describe.append("polarization3DModelId表示唯一编号,如果出现重复则后面的不起作用，只以第一次初始化为准\n");
		a2_describe.append("(2).The polarization3DModelId represents a unique identifier. If there is a repetition, the subsequent ones will be invalid, and only the first initialization will be taken as the standard.");
		//describe.append("1. polarization3DModelId==-1表示非法，请不要设置它的值为-1\n");
		a3_describe.append("(3).If polarization3DModelId == -1, it indicates an illegal value. Please do not set its value to -1.");
		//describe.append("2. polarization3DModelId==0表示全极化，不需要设置初始相位和极化方向以及能量占比\n");
		a4_describe.append("(4).When polarization3DModelId == 0, it indicates full polarization, and there is no need to set the initial phase, polarization direction, or energy proportion.");
		//describe.append("3. polarization3DModelId==1表示默认的单一线极化,初始相位为0，极化方向为(0,0,1),能量占比为100%\n");
		a5_describe.append("(5).When polarization3DModelId == 1, it indicates the default single linear polarization, with an initial phase of 0, polarization direction of (0, 0, 1), and energy proportion of 100%. ");
		//describe.append("4. polarization3DModelId==2表示默认的双线极化表示的一种左旋圆极化，\n");
		//describe.append("\t第1个线极化的初始相位为0，极化方向为(0,0,1)，能量占比为50%\n");
		//describe.append("\t第2个线极化的初始相位为0.5*pi，极化方向为(0,1,0)，能量占比为50%\n");
		a6_describe.append("(6).When polarization3DModelId == 2, it indicates a default dual-linear polarization representation of a left-handed circular polarization. The initial phase of the first linear polarization is 0, with a polarization direction of (0, 0, 1) and an energy proportion of 50%. The initial phase of the second linear polarization is 0.5 * pi, with a polarization direction of (0, 1, 0) and an energy proportion of 50%.");
		//describe.append("======polarization3DModelId不能设置为-1，0，1，2====\n");
		a7_describe.append("In conclusion, the polarization3DModelId cannot be set to -1, 0, 1, or 2.");

	}

	void BaseDescribeMultiLinearPolarization3DObjectDatabase2(
		std::vector<MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject>& multiLinearPolarization3DObjects) {

		multiLinearPolarization3DObjects.clear();
		{
			MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject;
			multiLinearPolarization3DObject.polarization3DModelId = 1;
			LinearPolarization3DObjectStd::LinearPolarization3DObject linearPolarization3DObject1(0.0, Point3DStd::Point3D(0.0, 0.0, 1.0));
			OneLinearPolarization3DStd::OneLinearPolarization3D oneLinearPolarization3D1(1.0, linearPolarization3DObject1);
			multiLinearPolarization3DObject.multiLinearPolarization3D.emplace_back(oneLinearPolarization3D1);
			multiLinearPolarization3DObjects.emplace_back(multiLinearPolarization3DObject);
		}

		{
			MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject;
			multiLinearPolarization3DObject.polarization3DModelId = 2;
			LinearPolarization3DObjectStd::LinearPolarization3DObject linearPolarization3DObject1(0.0, Point3DStd::Point3D(0.0, 0.0, 1.0));
			LinearPolarization3DObjectStd::LinearPolarization3DObject linearPolarization3DObject2(0.5 * GlobalConstantStd::Pi, Point3DStd::Point3D(0.0, 1.0, 0.0));
			OneLinearPolarization3DStd::OneLinearPolarization3D oneLinearPolarization3D1(1.0, linearPolarization3DObject1);
			OneLinearPolarization3DStd::OneLinearPolarization3D oneLinearPolarization3D2(1.0, linearPolarization3DObject2);
			multiLinearPolarization3DObject.multiLinearPolarization3D.emplace_back(oneLinearPolarization3D1);
			multiLinearPolarization3DObject.multiLinearPolarization3D.emplace_back(oneLinearPolarization3D2);
			multiLinearPolarization3DObjects.emplace_back(multiLinearPolarization3DObject);
		}

	}

}