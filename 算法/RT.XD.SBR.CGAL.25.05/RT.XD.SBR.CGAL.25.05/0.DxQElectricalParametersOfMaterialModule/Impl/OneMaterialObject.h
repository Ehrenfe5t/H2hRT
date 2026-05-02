#pragma once

#include"Input.h"

namespace OneMaterialObjectStd {
	
	class OneMaterialObject
	{
	public:
		/// <summary>
		/// true表示合法，false表示非法
		/// </summary>
		bool legal;

		MaterialObjectStd::MaterialObject materialObject;

		OneMaterialObject();
		~OneMaterialObject();

	private:

	};


}