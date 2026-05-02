

#include"LxQMultiLinearPolarization3D.h"
#include"LxQProjectDependencies.h"

namespace MultiLinearPolarization3DStd {



	MultiLinearPolarization3D::MultiLinearPolarization3D()
	{
		this->legal = false;
		this->polarization3DModelId = -1;
	}

	MultiLinearPolarization3D::MultiLinearPolarization3D(
		int polarization3DModelId,
		const std::vector<OneLinearPolarization3DStd::OneLinearPolarization3D>& linearPolarization3Ds)
	{
		this->legal = false;
		this->polarization3DModelId = polarization3DModelId;
		int count = 0;
		for (int i = 0; i < linearPolarization3Ds.size(); ++i) {
			if (linearPolarization3Ds[i].GetLegal()) {
				if (count >= LinearPolarization3DNumber) {
					//当前不支持超过个线极化。
					{
						std::ostringstream oss;
						oss << "当前不支持超过 " << LinearPolarization3DNumber << " 个线极化.";
						ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
					}
					break;
				}

				multiLinearPolarization3D[count].SetLinearPolarization3DObject(
					linearPolarization3Ds[i].weight, linearPolarization3Ds[i].GetLinearPolarization3DObject());
				this->legal = true;
				count++;
			}
		}

	}

	MultiLinearPolarization3D::~MultiLinearPolarization3D()
	{
	}


	bool MultiLinearPolarization3D::GetLegal() const
	{
		return this->legal;
	}



	MultiLinearPolarization3DStd::MultiLinearPolarization3D ToMultiLinearPolarization3D(
		const MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject& multiLinearPolarization3DObject) {
		MultiLinearPolarization3DStd::MultiLinearPolarization3D multiLinearPolarization3D(
			multiLinearPolarization3DObject.polarization3DModelId,
			multiLinearPolarization3DObject.multiLinearPolarization3D);
		return multiLinearPolarization3D;
	}

	MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject ToMultiLinearPolarization3DObject(
		const MultiLinearPolarization3DStd::MultiLinearPolarization3D& multiLinearPolarization3D) {
		MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject;
		multiLinearPolarization3DObject.polarization3DModelId = multiLinearPolarization3D.polarization3DModelId;
		for (int i = 0;i<LinearPolarization3DNumber;++i) {
			if (multiLinearPolarization3D.multiLinearPolarization3D[i].GetLegal()) {
				multiLinearPolarization3DObject.multiLinearPolarization3D.emplace_back(multiLinearPolarization3D.multiLinearPolarization3D[i]);
			}
		}

		return multiLinearPolarization3DObject;
	}


}