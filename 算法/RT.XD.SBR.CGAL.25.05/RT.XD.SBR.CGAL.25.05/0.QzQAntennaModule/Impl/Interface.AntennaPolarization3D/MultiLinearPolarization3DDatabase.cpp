
#include"../Input.h"

namespace MultiLinearPolarization3DDatabaseStd {


	const int MultiLinearPolarization3DDatabaseSize = 20;

	class MultiLinearPolarization3DDatabase
	{
	public:

		MultiLinearPolarization3DStd::MultiLinearPolarization3D multiLinearPolarization3DDatabase[MultiLinearPolarization3DDatabaseSize];

		MultiLinearPolarization3DDatabase();
		~MultiLinearPolarization3DDatabase();

	private:

	};

	MultiLinearPolarization3DDatabase::MultiLinearPolarization3DDatabase()
	{
	}

	MultiLinearPolarization3DDatabase::~MultiLinearPolarization3DDatabase()
	{
	}




	MultiLinearPolarization3DDatabase Database;


	void InitDatabaseByMultiLinearPolarization3DObjectDatabase(
		const MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase& multiLinearPolarization3DObjectDatabase)
	{
		int count = 0;

		for (int i = 0;i<multiLinearPolarization3DObjectDatabase.database.size();++i) {
			MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject = 
				multiLinearPolarization3DObjectDatabase.database[i];
			MultiLinearPolarization3DStd::MultiLinearPolarization3D multiLinearPolarization3D = 
				MultiLinearPolarization3DStd::ToMultiLinearPolarization3D(multiLinearPolarization3DObject);
			if (multiLinearPolarization3D.GetLegal()) {
				if (count >= MultiLinearPolarization3DDatabaseSize) {
					{
						std::ostringstream oss;
						oss << "当前不支持超过 " << MultiLinearPolarization3DDatabaseSize << " 个极化模式.";
						ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
					}
				}
				Database.multiLinearPolarization3DDatabase[count] = multiLinearPolarization3D;
				count++;
			}
		}

	}


	void InitLinearPolarization3DZ() {
		Database.multiLinearPolarization3DDatabase[0].legal = true;
		Database.multiLinearPolarization3DDatabase[0].polarization3DModelId = 1;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].legal = true;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].linearPolarization3DObject.phi0 = 0.0;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].linearPolarization3DObject.vec.x = 0.0;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].linearPolarization3DObject.vec.y = 0.0;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].linearPolarization3DObject.vec.z = 0.1;
		Database.multiLinearPolarization3DDatabase[0].multiLinearPolarization3D[0].weight = 1.0;
	}

	bool FindMultiLinearPolarization3D(
		int polarization3DModelId,
		MultiLinearPolarization3DStd::MultiLinearPolarization3D& multiLinearPolarization3D)
	{
		for (int i = 0; i < MultiLinearPolarization3DDatabaseSize;++i) {
			if (polarization3DModelId == Database.multiLinearPolarization3DDatabase[i].polarization3DModelId) {
				if (Database.multiLinearPolarization3DDatabase[i].GetLegal()) {
					multiLinearPolarization3D = Database.multiLinearPolarization3DDatabase[i];
					return true;
				}
			}
		}

		//找不到时返回线极化
		multiLinearPolarization3D = Database.multiLinearPolarization3DDatabase[0];
		return false;
	}


	MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase ToMultiLinearPolarization3DObjectDatabase() {
		MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase;

		for (int i = 0; i < MultiLinearPolarization3DDatabaseSize; ++i) {
			if (Database.multiLinearPolarization3DDatabase[i].GetLegal()) {

				MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject =
					MultiLinearPolarization3DStd::ToMultiLinearPolarization3DObject(Database.multiLinearPolarization3DDatabase[i]);

				multiLinearPolarization3DObjectDatabase.Add(multiLinearPolarization3DObject);
			}
		}

		return multiLinearPolarization3DObjectDatabase;
	}

}