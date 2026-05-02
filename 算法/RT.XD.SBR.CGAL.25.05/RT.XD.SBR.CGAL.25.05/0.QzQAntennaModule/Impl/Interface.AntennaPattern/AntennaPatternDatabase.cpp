
#include"../Input.h"

namespace AntennaPatternDatabaseStd {


	const int AntennaPatternDatabaseSize = 10;
	class AntennaPatternDatabase
	{
	public:

		AntennaPatternDatabaseObjectStd::AntennaPatternDatabaseObject antennaPatternDatabaseObjects[AntennaPatternDatabaseSize];

		AntennaPatternDatabase();
		~AntennaPatternDatabase();

	private:

	};

	AntennaPatternDatabase::AntennaPatternDatabase() {

	}
	AntennaPatternDatabase::~AntennaPatternDatabase() {

	}

	AntennaPatternDatabaseStd::AntennaPatternDatabase database;

	int FindFirstIllegalIndex() {
		for (int i = 0; i < AntennaPatternDatabaseSize; ++i) {
			if (!database.antennaPatternDatabaseObjects[i].legal) {
				return i;
			}
		}
		return -1;
	}


	int IndexOf(int radiationPatternId) {
		for (int i = 0;i< AntennaPatternDatabaseSize;++i) {
			if (database.antennaPatternDatabaseObjects[i].legal) {
				if (radiationPatternId== database.antennaPatternDatabaseObjects[i].antennaPatternObject.radiationPatternId) {
					return i;
				}
			}
		}
		return -1;
	}


	void Clear() {
		for (int i = 0; i < AntennaPatternDatabaseSize; ++i) {
			database.antennaPatternDatabaseObjects[i].legal = false;
		}
	}


	bool FindAntennaPatternObject(int radiationPatternId, AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject) {
		int index = IndexOf(radiationPatternId);
		if (index == -1) {
			return false;
		}
		antennaPatternObject = database.antennaPatternDatabaseObjects[index].antennaPatternObject;
		return true;
	}

	void AddAntennaPatternObject(const AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject) {
		AntennaPatternDatabaseObjectStd::AntennaPatternDatabaseObject antennaPatternDatabaseObject;
		antennaPatternDatabaseObject.SetAntennaPatternObject(antennaPatternObject);
		if (antennaPatternDatabaseObject.legal) {
			int index = IndexOf(antennaPatternDatabaseObject.antennaPatternObject.radiationPatternId);
			if (index == -1) {
				int firstIllegalIndex = FindFirstIllegalIndex();
				if (firstIllegalIndex == -1) {
					//数据库已经满了无法添加了
					return;
				}
				else {
					//将数据添加到合适位置
					database.antennaPatternDatabaseObjects[firstIllegalIndex].SetAntennaPatternObject(antennaPatternObject);
				}
			}
			else {
				//将数据添加到合适位置，这里覆盖了数据
				database.antennaPatternDatabaseObjects[index].SetAntennaPatternObject(antennaPatternObject);
			}
		}
		else {
			//传入的无效数据
		}
		
	}

	void AddRangeAntennaPatternObject(const std::vector<AntennaPatternObjectStd::AntennaPatternObject>& antennaPatternObjects) {
		for (int i = 0; i < antennaPatternObjects.size(); ++i) {
			AddAntennaPatternObject(antennaPatternObjects[i]);
		}
	}

	std::vector<AntennaPatternObjectStd::AntennaPatternObject> ToAntennaPatternObjectVector() {
		std::vector<AntennaPatternObjectStd::AntennaPatternObject> antennaPatternObjects;
		for (int i = 0; i < AntennaPatternDatabaseSize; ++i) {
			if (database.antennaPatternDatabaseObjects[i].legal) {
				antennaPatternObjects.emplace_back(database.antennaPatternDatabaseObjects[i].antennaPatternObject);
			}
		}
		return antennaPatternObjects;
	}

	double GetRadiationPatternByVector(int radiationPatternId, const Point3DStd::Point3D& vector)
	{
		if (radiationPatternId < 1) {
			return 0.0;
		}
		AntennaPatternObjectStd::AntennaPatternObject antennaPatternObject;
		if (FindAntennaPatternObject(radiationPatternId, antennaPatternObject)) {
			return AntennaPatternObjectStd::GetRadiationPatternByVector(antennaPatternObject, vector);
		}
		else {
			ProjectDependenciesStd::DisplayPromptOrReason("找不到该天线方向图.将按照没有天线方向图计算", false, __FILE__, __LINE__);
		}
		return 0.0;
	}

	double CalCoefficientByVector(int radiationPatternId, const Point3DStd::Point3D& vector) {
		double radiationPattern = GetRadiationPatternByVector(radiationPatternId, vector);
		return sqrt(pow(10.0, radiationPattern / 10.0));
	}

}