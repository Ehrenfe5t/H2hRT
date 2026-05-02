#pragma once

#include <vector>

/// <summary>
/// 定义了材质的基本属性
/// </summary>
struct Material {


    /// <summary>
    /// 材质类型编号
    /// </summary> 
    short materialTypeNumber;

    /// <summary>
    /// 频率
    /// </summary> 
    long long frequency;

    /// <summary>
    /// 相对介电常数
    /// </summary>
    double relativePermittivity;

    /// <summary>
    /// 导电率
    /// </summary> 
    double conductivity; 

};



/// <summary>
/// 定义材料集合结构体，用于存储和管理不同类型的材料信息
/// </summary>
struct MaterialSet {
    int size;
    /// <summary>
    /// 材料集合，存储多个Material对象的向量
    /// </summary>
    Material* materials;
};


extern "C" _declspec(dllexport) void PrintMaterialSet(const MaterialSet & materialSet); 