#pragma once

#include"HdQAntennaPatternObject.h"

namespace AntennaPatternDatabaseObjectStd {

	class AntennaPatternDatabaseObject
	{
	public:
		/// <summary>
		/// true表示合法，false表示非法
		/// </summary>
		bool legal;

		AntennaPatternObjectStd::AntennaPatternObject antennaPatternObject;

		AntennaPatternDatabaseObject();
		~AntennaPatternDatabaseObject();

		void SetAntennaPatternObject(const AntennaPatternObjectStd::AntennaPatternObject& antennaPatternObject);

	private:

	};


}

