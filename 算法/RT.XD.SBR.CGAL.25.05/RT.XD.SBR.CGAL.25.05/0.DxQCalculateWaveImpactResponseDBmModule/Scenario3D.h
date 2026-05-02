#pragma once

#include "Point3D.h"

#include <vector>

/// <summary>
/// 三维三角形结构体，用于表示三维空间中的一个三角形
/// </summary>
struct Triangle3D {

    /// <summary>
    /// 三角面元的上部类型编号
    /// </summary>
    short topMaterialTypeNumber;

    /// <summary>
    /// 三角面元的下部类型编号
    /// </summary>
    short bottomMaterialTypeNumber;

    /// <summary>
    /// 三角面元的第一个顶点索引
    /// </summary>
    int triangleP1Index;    

    /// <summary>
    /// 三角面元的第二个顶点索引
    /// </summary>
    int triangleP2Index;    

    /// <summary>
    /// 三角面元的第三个顶点索引
    /// </summary>
    int triangleP3Index;    

    /// <summary>
    /// 三角面元的粗糙度
    /// </summary>
    float roughness;
    float roughness_zw;

    /// <summary>
    /// 三角面元的法向量,单位化的
    /// </summary>
    Point3D n;
} ;


/// <summary>
/// 定义一个三维角点的结构体
/// </summary>
struct Corner3D {

    /// <summary>
    /// 角点的第一个顶点索引
    /// </summary>
    int p1Index;            

    /// <summary>
    /// 角点的第二个顶点索引
    /// </summary>
    int p2Index;            

    /// <summary>
    /// 构成三角面元1的第三个顶点索引
    /// </summary>
    int p3Face1Index;       

    /// <summary>
    /// 构成三角面元2的第三个顶点索引
    /// </summary>
    int p3Face2Index;        

    /// <summary>
    /// 第一个三角面元的索引
    /// </summary>
    int face1Index;         

    /// <summary>
    /// 第二个三角面元的索引
    /// </summary>
    int face2Index;         

    int face2Index_fq;
    /// <summary>
    /// 角点的半径
    /// </summary>
    float radius;            
};


/// <summary>
/// 表示三维场景的相关信息
/// </summary>
struct Scenario3D {

    int pointsCount;         // 对应 std::vector 的 size()
    int trianglesCount;
    int cornersCount;

    /// <summary>
    /// 占位符，避免c#无法对齐
    /// </summary>
    int cornersCount_fq;

    /// <summary>
    /// 场景中的所有三维点集合
    /// </summary>
    Point3D* scenario_point3d_set;

    /// <summary>
    /// 场景中的所有三维三角面元集合
    /// </summary>
    Triangle3D* scenario_triangle3d_set;

    /// <summary>
    /// 场景中的所有三维角点集合
    /// </summary>
    Corner3D* scenario_corner3d_set;
} ;