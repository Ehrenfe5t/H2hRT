#pragma once

#include"HdQConfig.h"
#include"LxQMaterialObject.h"

namespace MaterialObjectDatabaseStd {


	INTERFACE_API std::vector<MaterialObjectStd::MaterialObject> GetAllMaterialObject();

	INTERFACE_API int IndexOf(int typeNumber, long long frequency);
	/// <summary>
	/// 可能会覆盖
	/// </summary>
	/// <param name="materialObject"></param>
	/// <returns></returns>
	INTERFACE_API bool Add(const MaterialObjectStd::MaterialObject& materialObject);

	/// <summary>
	/// 可能会覆盖
	/// </summary>
	/// <param name="materialObjects"></param>
	INTERFACE_API void AddRange(const std::vector<MaterialObjectStd::MaterialObject>& materialObjects);


	INTERFACE_API void Init(const std::vector<MaterialObjectStd::MaterialObject>& materialObjects);

	INTERFACE_API bool RemoveAt(int typeNumber, long long frequency);

	INTERFACE_API void Clear();

	INTERFACE_API bool Find(int typeNumber, long long frequency, MaterialObjectStd::MaterialObject& materialObject);

}
