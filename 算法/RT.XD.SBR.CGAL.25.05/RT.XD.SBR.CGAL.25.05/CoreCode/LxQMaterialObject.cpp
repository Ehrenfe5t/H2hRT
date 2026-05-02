

#include"LxQMaterialObject.h"
#include"QzQGlobalConstant.h"
#include"HdQComplexOperate.h"


namespace MaterialObjectStd {


	MaterialObject::MaterialObject() {
		this->typeNumber = GlobalConstantStd::GetAirSubstanceType();
		this->frequency = (long long)1e9;
		this->relativePermittivity = 1.0;
		this->conductivity = 0.0;
		this->relativePermeability = 1.0;
		this->magnetoconductivity = 0.0;

		std::string str = "No name";
		std::copy(str.begin(), str.end(), this->materialName);
		materialName[str.length()] = '\0';
		
	}

	MaterialObject::MaterialObject(const MaterialObject& obj)
	{
		this->typeNumber = obj.typeNumber;
		this->frequency = obj.frequency;
		this->relativePermittivity = obj.relativePermittivity;
		this->conductivity = obj.conductivity;
		this->relativePermeability = obj.relativePermeability;
		this->magnetoconductivity = obj.magnetoconductivity;
		//this->name = obj.name;
		std::copy(obj.materialName, obj.materialName + strlen(obj.materialName) + 1, this->materialName);

	}


	MaterialObject::~MaterialObject() {

	}


	



	int FindIndexOfListMaterialParamByObjectTypeAndF(int typeNumber, long long frequency, const std::vector<MaterialObjectStd::MaterialObject>& materialObjects) {
		for (int i = 0; i < materialObjects.size(); i++) {
			if (materialObjects[i].typeNumber == typeNumber) {
				if (materialObjects[i].frequency == frequency) {
					return i;
				}
			}
		}
		return -1;
	}

	int ContainMaterialObjectInVectorMaterialObject(const MaterialObject& value, const std::vector<MaterialObject>& list) {
		return FindIndexOfListMaterialParamByObjectTypeAndF(value.typeNumber,value.frequency, list);
	}

	double GetTime(const MaterialObjectStd::MaterialObject& materialObject,double distance) {
		double v = GlobalConstantStd::C / sqrt(materialObject.relativePermittivity);
		return distance / v;
	}


	double GetZ(const MaterialObjectStd::MaterialObject& materialObject) {

		//double uc = materialObject.relativePermeability * GlobalConstantStd::RelativePermeability_0;
		//double ec = materialObject.relativePermittivity * GlobalConstantStd::RelativePermittivity_0;
		//
		//return sqrt(uc / ec);
		double w = 2 * GlobalConstantStd::Pi * materialObject.frequency;
		ComplexStd::Complex uc(0.0, w * materialObject.relativePermeability * GlobalConstantStd::RelativePermeability_0);
		ComplexStd::Complex ec(materialObject.conductivity, w * materialObject.relativePermittivity * GlobalConstantStd::RelativePermittivity_0);
		ComplexStd::Complex div = ComplexStd::DivComplexComplex_unsafe(uc,ec);
		ComplexStd::Complex sqrt_d = ComplexStd::SqrtComplex(div);
		return  sqrt_d.real;
	}

	ComplexStd::Complex GetK2(const MaterialObjectStd::MaterialObject& materialObject) {
		double w = 2 * GlobalConstantStd::Pi * materialObject.frequency;

		double uc = materialObject.relativePermeability * GlobalConstantStd::RelativePermeability_0;
		double ec = materialObject.relativePermittivity * GlobalConstantStd::RelativePermittivity_0;

		double temp1 = w * ec;
		double temp2 = uc * ec;
		double temp3 = materialObject.conductivity / temp1;
		double temp4 = sqrt(1+ temp3* temp3);
		double k1 = w * sqrt((temp4 + 1) * 0.5 * temp2);
		double k2 = w * sqrt((temp4 - 1) * 0.5 * temp2);

		return ComplexStd::Complex(k1, k2);
	}

	ComplexStd::Complex GetK(const MaterialObjectStd::MaterialObject& materialObject) {
		double w = 2 * GlobalConstantStd::Pi * materialObject.frequency;
		//复数介电系数
		//ec = e0.*er + 1i.*detal. / w;
		ComplexStd::Complex ec(materialObject.relativePermittivity * GlobalConstantStd::RelativePermittivity_0, materialObject.conductivity / w);
		//复数导磁系数
		ComplexStd::Complex uc(materialObject.relativePermeability * GlobalConstantStd::RelativePermeability_0, materialObject.magnetoconductivity / w);

		ComplexStd::Complex temp1 = ComplexStd::MulComplexComplex(ec, uc);
		ComplexStd::Complex temp2 = ComplexStd::SqrtComplex_unsafe(temp1);

		ComplexStd::Complex k = ComplexStd::MulDoubleComplex(w, temp2);

		return k;
	}

}