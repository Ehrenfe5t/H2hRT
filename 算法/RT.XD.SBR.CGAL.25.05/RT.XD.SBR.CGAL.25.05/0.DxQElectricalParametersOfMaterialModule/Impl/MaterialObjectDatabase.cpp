
#include"Input.h"

#include "OneMaterialObject.h"

namespace MaterialObjectDatabaseStd {


	const int MaterialObjectDatabaseSize = 200;

	class MaterialObjectDatabase
	{
	public:

		int size;

		OneMaterialObjectStd::OneMaterialObject materialObjects[MaterialObjectDatabaseSize];
		MaterialObjectDatabase();
		~MaterialObjectDatabase();

	private:

	};


	MaterialObjectDatabase::MaterialObjectDatabase()
	{
		this->size = 0;
	}
	MaterialObjectDatabase::~MaterialObjectDatabase()
	{
	}



	MaterialObjectDatabase Database;

	std::vector<MaterialObjectStd::MaterialObject> GetAllMaterialObject() {
		std::vector<MaterialObjectStd::MaterialObject> materialObjects;
		for (int i = 0; i < Database.size; ++i) {
			materialObjects.emplace_back(Database.materialObjects[i].materialObject);
		}
		return materialObjects;
	}

	int IndexOf(int typeNumber, long long frequency) {

		for (int i = 0; i < Database.size; ++i) {
			if (typeNumber == Database.materialObjects[i].materialObject.typeNumber) {
				if (frequency == Database.materialObjects[i].materialObject.frequency) {
					return i;
				}
			}
		}
		
		return -1;
	}
	/// <summary>
	/// 可能会覆盖
	/// </summary>
	/// <param name="materialObject"></param>
	/// <returns></returns>
	bool Add(const MaterialObjectStd::MaterialObject& materialObject) {

		if (Database.size< MaterialObjectDatabaseSize) {

			int index = IndexOf(materialObject.typeNumber, materialObject.frequency);
			if (index == -1) {
				Database.materialObjects[Database.size].legal = true;
				Database.materialObjects[Database.size].materialObject.conductivity = materialObject.conductivity;
				Database.materialObjects[Database.size].materialObject.frequency = materialObject.frequency;
				Database.materialObjects[Database.size].materialObject.magnetoconductivity = materialObject.magnetoconductivity;

				std::string str(materialObject.materialName);
				std::copy(str.begin(), str.end(), Database.materialObjects[Database.size].materialObject.materialName);
				Database.materialObjects[Database.size].materialObject.materialName[str.length()] = '\0';

				Database.materialObjects[Database.size].materialObject.relativePermeability = materialObject.relativePermeability;
				Database.materialObjects[Database.size].materialObject.relativePermittivity = materialObject.relativePermittivity;
				Database.materialObjects[Database.size].materialObject.typeNumber = materialObject.typeNumber;
				Database.size = Database.size + 1;
				return true;
			}
			else {
				Database.materialObjects[index].materialObject.conductivity = materialObject.conductivity;
				Database.materialObjects[index].materialObject.frequency = materialObject.frequency;
				Database.materialObjects[index].materialObject.magnetoconductivity = materialObject.magnetoconductivity;

				std::string str(materialObject.materialName);
				std::copy(str.begin(), str.end(), Database.materialObjects[index].materialObject.materialName);
				Database.materialObjects[index].materialObject.materialName[str.length()] = '\0';

				Database.materialObjects[index].materialObject.relativePermeability = materialObject.relativePermeability;
				Database.materialObjects[index].materialObject.relativePermittivity = materialObject.relativePermittivity;
				Database.materialObjects[index].materialObject.typeNumber = materialObject.typeNumber;
			}
			
		}
		return false;
	}

	/// <summary>
	/// 可能会覆盖
	/// </summary>
	/// <param name="materialObjects"></param>
	void AddRange(const std::vector<MaterialObjectStd::MaterialObject>& materialObjects) {
		for (int i = 0; i < materialObjects.size(); ++i) {
			Add(materialObjects[i]);
		}
	}


	void Init(const std::vector<MaterialObjectStd::MaterialObject>& materialObjects) {
		Clear();
		AddRange(materialObjects);
	}

	bool RemoveAt(int typeNumber, long long frequency) {
		int index = IndexOf(typeNumber, frequency);
		if (index == -1) {
			return false;
		}
		std::vector<MaterialObjectStd::MaterialObject> materialObjects = GetAllMaterialObject();
		materialObjects.erase(materialObjects.begin()+index);
		Init(materialObjects);
		return true;
	}

	void Clear() {
		for (int i = 0; i < MaterialObjectDatabaseSize; ++i) {
			Database.materialObjects[i].legal = false;
		}
		Database.size = 0;
	}

	bool Find(int typeNumber, long long frequency, MaterialObjectStd::MaterialObject& materialObject) {
		int index = IndexOf(typeNumber, frequency);
		if (index == -1) {
			{
				{
					std::ostringstream oss;
					oss << std::endl << std::endl;
					oss << "没有找到材质 ";
					ProjectDependenciesStd::DisplayPromptOrReason(oss.str(),true,__FILE__,__LINE__);
				}
			}
			return false;
		}
		materialObject = Database.materialObjects[index].materialObject;
		return true;
	}



}