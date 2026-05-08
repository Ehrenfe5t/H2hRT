# RT 3D电波传播射线追踪算法源码详细分析文档

## 目录

1. [系统概述](#一系统概述)
2. [数学理论基础](#二数学理论基础)
3. [核心数据结构](#三核心数据结构)
4. [从初始化到结果输出的完整流程](#四从初始化到结果输出的完整流程)
5. [射线生成模块](#五射线生成模块)
6. [空间加速结构模块](#六空间加速结构模块)
7. [射线与几何元相交检测模块](#七射线与几何元相交检测模块)
8. [射线追踪路径搜索模块](#八射线追踪路径搜索模块)
9. [波的传播损耗计算模块](#九波的传播损耗计算模块)
10. [材质参数模块](#十材质参数模块)
11. [天线模型模块](#十一天线模型模块)
12. [输出结果模块](#十二输出结果模块)
13. [关键技术细节](#十三关键技术细节)
14. [算法复杂度分析](#十四算法复杂度分析)

---

## 一、系统概述

### 1.1 算法定位

这是一个基于**SBR(射线发射法 Shooting-Based Ray tracing)**的**三维电波传播射线追踪算法系统**，主要用于：
- 无线通信电波覆盖预测
- MIMO信道建模
- 城市微蜂窝传播预测
- 复杂电磁环境仿真

### 1.2 核心技术特征

| 特性 | 说明 |
|------|------|
| 射线发射法 | 从发射天线向多个方向发射射线，追踪射线传播路径 |
| 多机制传播 | 支持反射、透射、绕射、散射 |
| 空间加速 | 使用AABB-BVH树加速射线-场景相交检测 |
| 多线程并行 | 支持多线程并行射线追踪 |
| MIMO支持 | 支持单发射多接收(SIMO)和多发射多接收(MIMO) |

### 1.3 项目结构

```
算法/
└── RT.XD.SBR.CGAL.25.05/
    └── RT.XD.SBR.CGAL.25.05/
        ├── main.cpp                          # 主入口
        ├── CoreCode/                       # 核心代码
        │   ├── XdQRtSbr3D.cpp            # 主RT引擎
        │   ├── DxQRay3D.cpp              # 射线类
        │   ├── DxQScenarioObject.cpp       # 场景对象
        │   └── ...
        ├── 0.LxQSbr3DRay3DGenerationModule/  # 射线生成
        │   └── Sbr3DRay3DGeneration/
        │       ├── SbrRayGeneratedByTransmittingAntenna.cpp
        │       ├── SbrRayGeneratedByReflection.cpp
        │       └── SbrRayGeneratedByTransmission.cpp
        ├── 0.Ray3DIntersectGeometry3DElementsModule.Impl/  # 相交检测
        │   ├── AabbBvhTree/
        │   │   ├── Ray3DIntersectGeometry3DElementsAabbBvhTree.cpp
        │   │   └── Ray3DIntersectGeometry3DElementsAabbBvhTree4.cpp
        │   └── Base/
        │       └── Ray3DIntersectGeometry3DElementsBase.cpp
        └── 0.DxQCalculateWaveImpactResponseDBmModule.Impl/   # 波传播损耗
            └── Impl/
                ├── CalculateReflectionWaveLoss.cpp
                ├── CalculateTransmissionWaveLoss.cpp
                ├── CalculateDiffractionWaveLoss.cpp
                └── CalculateDiffuseScatteringWaveLoss.cpp
```

---

## 二、数学理论基础

### 2.1 射线追踪基础理论

#### 2.1.1 射线光学近似

电波传播在高频极限下可用**几何光学(GO)**近似，射线满足：

$$\vec{r}(s) = \vec{r}_0 + s\vec{d}$$

其中：
- $\vec{r}_0$：射线原点
- $\vec{d}$：射线方向单位向量
- $s$：沿射线距离

#### 2.1.2 费马原理

射线传播路径满足**费马原理**：光在两点间传播所选取的路径是光程极值的路径。

$$\delta \int_{A}^{B} n(\vec{r}) ds = 0$$

在均匀介质中，射线沿直线传播；在非均匀介质中，按斯涅尔定律弯曲。

#### 2.1.3 斯涅尔定律(波折射定律)

当波从介质1进入介质2时：

$$\frac{\sin\theta_1}{\sin\theta_2} = \frac{n_2}{n_1} = \sqrt{\frac{\varepsilon_2}{\varepsilon_1}}$$

其中$\theta_1$为入射角，$\theta_2$为透射角。

### 2.2 向量代数基础

#### 2.2.1 叉积(外积)

$$\vec{a} \times \vec{b} = \begin{vmatrix} \vec{i} & \vec{j} & \vec{k} \\ a_x & a_y & a_z \\ b_x & b_y & b_z \end{vmatrix} = \begin{pmatrix} a_y b_z - a_z b_y \\ a_z b_x - a_x b_z \\ a_x b_y - a_y b_x \end{pmatrix}$$

代码实现(`Ray3DIntersectGeometry3DElementsBase.cpp:69-80`):
```cpp
Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2) {
    double l = p1.x, m = p1.y, n = p1.z;
    double o = p2.x, p = p2.y, q = p2.z;
    Point3D res = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);
    return res;
}
```

#### 2.2.2 点积(内积)

$$\vec{a} \cdot \vec{b} = a_x b_x + a_y b_y + a_z b_z$$

代码实现:
```cpp
double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2) {
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}
```

#### 2.2.3 向量减法

$$\vec{a} - \vec{b} = (a_x - b_x, a_y - b_y, a_z - b_z)$$

```cpp
Point3D SubPoint3DPoint3D(const Point3D& p1, const Point3D& p2) {
    return CreatePoint3D(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
}
```

#### 2.2.4 向量归一化

$$\hat{a} = \frac{\vec{a}}{|\vec{a}|}, \quad |\vec{a}| = \sqrt{\vec{a} \cdot \vec{a}}$$

```cpp
Point3D Normalization_Point3D(const Point3D& p) {
    double len = Length_Point3D(p);
    if (len < Eps * Eps) {
        return CreatePoint3D(0.0, 0.0, 0.0);
    }
    return MulDoublePoint3D(1 / len, p);
}
```

### 2.3 射线-三角形相交检测理论

#### 2.3.1 Möller–Trumbore算法

射线与三角形的相交检测使用**Möller–Trumbore算法**，基于参数化表示：

设射线为 $R = O + t\vec{D}$，三角形顶点为 $V_0, V_1, V_2$，定义边向量：

$$E_1 = V_1 - V_0, \quad E_2 = V_2 - V_0$$

计算：

$$S = O - V_0$$
$$P = \vec{D} \times E_2$$
$$Q = S \times E_1$$

参数方程：

$$t = \frac{P \cdot S}{P \cdot E_1}$$
$$u = \frac{Q \cdot E_2}{P \cdot E_1}$$
$$v = \frac{Q \cdot \vec{D}}{P \cdot E_1}$$

相交条件：
- $t > 0$ (在射线正方向)
- $u \geq 0, v \geq 0, u+v \leq 1$ (重心坐标)

代码实现(`Ray3DIntersectGeometry3DElementsBase.cpp:82-138`):
```cpp
bool Intersect_Ray3D_Triangle3D_plus(
    const Point3D& O,           // 射线原点
    const Point3D& rayVec,     // 射线方向
    const Point3D& V1,        // 三角形顶点
    const Point3D& E1,        // 边向量E1 = V1-V2
    const Point3D& E2,        // 边向量E2 = V1-V3
    const Point3D& E1E2,      // 预计算的叉积E1×E2
    const Point3D& E2E1,      // 预计算的叉积E2×E1
    double& distance,         // 输出交点距离
    Point3D& res) {          // 输出交点坐标
    
    Point3D D = CreatePoint3D(-rayVec.x, -rayVec.y, -rayVec.z);
    Point3D T = SubPoint3DPoint3D(V1, O);
    double f1 = DotPoint3DPoint3D(D, E2E1);
    if (IsZeroAbs(f1)) return false;  // 平行
    
    double f2 = DotPoint3DPoint3D(T, E1E2);
    double t = f2 / f1;
    if (t < Eps) return false;  // 反向
    
    Point3D P = CrossPoint3DPoint3D(D, E2);
    double f3 = DotPoint3DPoint3D(P, T);
    double u = f3 / f1;
    if (u < 0.0) return false;  // 在三角形外
    
    Point3D Q = CrossPoint3DPoint3D(T, E1);
    double f4 = DotPoint3DPoint3D(Q, D);
    double v = f4 / f1;
    if (v < 0.0 || u + v > 1.0f) return false;  // 超过三角形范围
    
    distance = t;
    res = O + t * rayVec;
    return true;
}
```

### 2.4 射线-墙角相交检测理论

#### 2.4.1 墙角建模

墙角由两条棱边定义：$P_1$和$P_2$，形成一条线段。

#### 2.4.2 射线-线段最短距离

计算射线到线段的距离，使用空间两直线的公垂线计算：

设空间两直线：
$$L_1: \vec{r} = \vec{a}_1 + s\vec{u}_1$$
$$L_2: \vec{r} = \vec{a}_2 + t\vec{u}_2$$

构建矩阵求解：

$$D = \begin{vmatrix} u_{1x} & -u_{2x} & (u_1 \times u_2)_x \\ u_{1y} & -u_{2y} & (u_1 \times u_2)_y \\ u_{1z} & -u_{2z} & (u_1 \times u_2)_z \end{vmatrix}$$

代码实现(`Ray3DIntersectGeometry3DElementsBase.cpp:333-403`):
```cpp
double GetDistanceLine3DLine3D_plus_unsafe(
    const Point3D& o1, const Point3D& d1,
    const Point3D& o2, const Point3D& d2,
    double& distance1, double& distance2,
    Point3D& res1, Point3D& res2) {
    Point3D d3 = CrossPoint3DPoint3D(d1, d2);
    // ... 矩阵求解 ...
}
```

绕射条件：射线与墙角棱边的距离小于绕射半径。

```cpp
bool Intersect_Ray3D_Corner3D(
    double radiusCorner,  // 绕射半径
    const Point3D& O,   // 射线原点
    const Point3D& rayVec,  // 射线方向
    const Point3D& seg_start,   // 线段起点
    const Point3D& seg_end,    // 线段终点
    double seg_length,         // 线段长度
    const Point3D& seg_unit_vec, // 线段单位向量
    double& rayLength,        // 输出交点距离
    Point3D& res) {        // 输出交点
    
    double pToSeg = GetDistancePoint3DLine3D_plus_unsafe(O, seg_start, seg_unit_vec);
    if (pToSeg < Eps) return false;
    
    double curSegSegDis = GetDistanceLine3DLine3D_plus_unsafe(O, rayVec, seg_start, seg_unit_vec, ...);
    if (curSegSegDis <= radiusCorner) {
        rayLength = GetDistancePoint3DPoint3D(O, res2);
        res = res2;
        return true;
    }
    return false;
}
```

### 2.5 反射理论

#### 2.5.1 镜面反射定律

入射波在介质表面发生反射，满足：
- 入射角等于反射角
- 入射波、反射波和法线共面

反射射线方向：

$$\vec{D}_r = \vec{D}_i - 2(\vec{D}_i \cdot \vec{n})\vec{n}$$

代码实现(`SbrRayGeneratedByReflection.cpp:17-49`):
```cpp
int BuildReflectionRay(
    const Point3D& a,        // 入射点
    const Point3D& b,        // 反射点
    const Point3D& n,         // 法线
    Ray3D& newRay,          // 输出反射射线
    double& thetai) {       // 输出入射角
    
    Point3D ab = b - a;
    double d1 = Dot(ab, n);       // ab·n
    double d2 = Length(ab);      // |ab|
    if (d2 < Eps) return 0;
    double d3 = d1 / d2;
    
    // 确保法线指向入射方向
    Point3D n2 = n;
    if (d1 < 0) {
        d3 = -d3;
    } else {
        n2 = -n;  // 反转法线
    }
    thetai = acos(d3);
    
    // 反射方向计算
    Point3D bc = d3 * n2;           // 镜面分量
    Point3D ac = ab/d2 + bc;        // 入射方向分解
    Point3D vec = ac + bc;          // 反射方向 = 入射+反射分量
    newRay.o = b;
    newRay.vec = vec;
    return 1;
}
```

### 2.6 透射(折射)理论

#### 2.6.1 斯涅尔定律

$$\frac{\sin\theta_t}{\sin\theta_i} = \frac{n_1}{n_2} = \sqrt{\frac{\varepsilon_1}{\varepsilon_2}}$$

其中$\theta_i$为入射角，$\theta_t$为透射角。

#### 2.6.2 复数介电常数

实际介质需要用复数介电常数描述：

$$\varepsilon_c = \varepsilon' - j\varepsilon'' = \varepsilon_r \varepsilon_0 - j\frac{\sigma}{\omega}$$

其中：
- $\varepsilon_r$：相对介电常数
- $\sigma$：电导率(S/m)
- $\omega = 2\pi f$：角频率

代码实现(`CalculateWaveLossCoefficientBase.cpp`):
```cpp
Complex CalculateComplexPermittivity(long long frequency, double relativePermittivity, double conductivity) {
    double omega = 2 * PI * frequency;
    double real = relativePermittivity * EPSILON_0;
    double imag = -conductivity / omega;
    return Complex(real, imag);
}
```

#### 2.6.3 复数折射率

$$n_c = \sqrt{\mu_r \varepsilon_c}$$

代码实现:
```cpp
Complex CalculateComplexRefractiveIndexOfMedium(const Complex& complexPermittivity) {
    double real = complexPermittivity.real;
    double imag = complexPermittivity.imag;
    double mag = sqrt(real*real + imag*imag);
    double phase = atan2(imag, real) / 2;
    double sqrt_mag = sqrt(mag);
    double real_out = sqrt_mag * cos(phase);
    double imag_out = sqrt_mag * sin(phase);
    return Complex(real_out, imag_out);
}
```

#### 2.6.4 透射角计算

从斯涅尔定律得：

$$\sin\theta_t = \frac{n_1}{n_2}\sin\theta_i$$

代码实现(`CalculateWaveLossCoefficientBase.cpp`):
```cpp
bool CalculateThetat(double thetai, const Complex& n1, const Complex& n2, double& thetat) {
    double sin_thetai = sin(thetai);
    Complex ratio = n1 / n2;
    double ratio_mag = sqrt(ratio.real*ratio.real + ratio.imag*ratio.imag);
    double sin_thetat = ratio_mag * sin_thetai;
    
    if (sin_thetat > 1.0) {
        // 全内反射
        return false;
    }
    thetat = asin(sin_thetat);
    return true;
}
```

### 2.7 绕射理论

#### 2.7.1 UTD绕射理论

使用**一致性衍射理论(Uniform Theory of Diffraction)**，绕射系数基于Keller的GTD并加入一致性项。

#### 2.7.2 Fresnel积分

绕射系数计算需要Fresnel积分：

$$F(x) = 2\sqrt{x}\exp(j\pi/4)\int_0^{\sqrt{x}} \exp(jt^2) dt$$

或等效形式：

$$F(x) = \sqrt{\pi x} \exp(j\pi/4) - 2x\exp(-j\pi/4)\dots$$

代码实现(`CalculateDiffractionWaveLoss.cpp:27-59`):
```cpp
Complex FresnelFunction(double x) {
    if (x < 0.0001) {
        // 小x近似
        Complex temp1 = Exp(PI / 4.0);
        Complex temp2 = Exp(-PI / 4.0);
        Complex temp3 = Exp(x + PI / 4.0);
        Complex Fx = (sqrt(PI * x) - temp1 * 2 * x - 2/3 * x * x * temp2) * temp3;
        return Fx;
    } else if (x >= 0.0001 && x <= 10) {
        // 数值积分
        double Fx_re = sqrt(PI * x) * cos(x + PI / 4);
        double Fx_im = sqrt(PI * x) * sin(x + PI / 4);
        double max_x_value = sqrt(x);
        double step = 1e-4;
        
        for (double dx = 0; dx <= max_x_value; dx += step) {
            Fx_re = Fx_re - 2 * max_x_value * cos(x + PI/2 - dx*dx) * step;
            Fx_im = Fx_im - 2 * max_x_value * sin(x + PI/2 - dx*dx) * step;
        }
        return Complex(Fx_re, Fx_im);
    } else {
        // 大x渐近
        double Fx_re = 1 - 3 / (4 * x * x) + 75 / (16 * x * x * x * x);
        double Fx_im = 1 / (2 * x) - 15 / (8 * x * x * x);
        return Complex(Fx_re, Fx_im);
    }
}
```

#### 2.7.3 UTD绕射系数

对于楔形绕射，绕射系数(D1-D4)计算：

$$D_0 = \frac{e^{-j\pi/4}}{2n\sqrt{2\pi k}\sin\beta_0}$$

$$a_1 = \frac{\pi - \phi_2}{2n}, a_2 = \frac{\pi + \phi_2}{2n}, a_3 = \frac{\pi - \phi_1}{2n}, a_4 = \frac{\pi + \phi_1}{2n}$$

$$D_1 = D_0 \cot(a_1) F(kL\sin^2 a_1)$$
$$D_2 = D_0 \cot(a_2) F(kL\sin^2 a_2)$$
$$D_3 = D_0 \cot(a_3) F(kL\sin^2 a_3)$$
$$D_4 = D_0 \cot(a_4) F(kL\sin^2 a_4)$$

总绕射系数(Holm+模型)：

$$\tau = D_1 + D_2 + r_0 D_3 + r_n D_4$$
$$\mu = D_1 + D_2 + r_0 D_3 + r_n D_4$$

代码实现(`CalculateDiffractionWaveLoss.cpp:162-205`):
```cpp
void CalD1234(double n, double k, double sin_beta0, double phi1, double phi2,
    Complex& D1, Complex& D2, Complex& D3, Complex& D4) {
    
    double phi1_add_phi2 = phi1 + phi2;
    double phi2_sub_phi1 = phi2 - phi1;
    double p_2_n = 2 * n;
    
    double a1 = (PI - phi2_sub_phi1) / p_2_n;
    double a2 = (PI + phi2_sub_phi1) / p_2_n;
    double a3 = (PI - phi1_add_phi2) / p_2_n;
    double a4 = (PI + phi1_add_phi2) / p_2_n;
    
    double L = sin_beta0 * sin_beta0;
    
    Complex Fx1 = FresnelFunction(sin(a1)*sin(a1)*L*k*p_2_n);
    Complex Fx2 = FresnelFunction(sin(a2)*sin(a2)*L*k*p_2_n);
    Complex Fx3 = FresnelFunction(sin(a3)*sin(a3)*L*k*p_2_n);
    Complex Fx4 = FresnelFunction(sin(a4)*sin(a4)*L*k*p_2_n);
    
    Complex D0 = Exp(-PI/4.0) / (p_2_n * sqrt(2*PI*k) * sin_beta0);
    
    D1 = D0 * Cot(a1) * Fx1;
    D2 = D0 * Cot(a2) * Fx2;
    D3 = D0 * Cot(a3) * Fx3;
    D4 = D0 * Cot(a4) * Fx4;
}
```

### 2.8 波传播损耗系数

#### 2.8.1 反射系数(Fresnel公式)

对于TE和TM极化：

$$Gamma_{TE} = \frac{n_1\cos\theta_i - n_2\cos\theta_t}{n_1\cos\theta_i + n_2\cos\theta_t}$$
$$Gamma_{TM} = \frac{n_2\cos\theta_i - n_1\cos\theta_t}{n_2\cos\theta_i + n_1\cos\theta_t}$$

代码实现(`CalculateReflectionWaveLoss.cpp:8-28`):
```cpp
void CalculateReflectionWaveCoefficientBase(
    double thetai, double thetat,
    const Complex& n1, const Complex& n2,
    Complex& te, Complex& tm) {
    
    double cos_i = cos(thetai);
    double cos_t = cos(thetat);
    
    Complex temp1 = n1 * cos_i;
    Complex temp2 = n2 * cos_i;
    Complex temp3 = n1 * cos_t;
    Complex temp4 = n2 * cos_t;
    
    te = (temp1 - temp4) / (temp1 + temp4);
    tm = (temp2 - temp3) / (temp2 + temp3);
}
```

#### 2.8.2 透射系数

$$\tau_{TE} = \frac{2n_1\cos\theta_i}{n_1\cos\theta_i + n_2\cos\theta_t}$$
$$\tau_{TM} = \frac{2n_1\cos\theta_i}{n_2\cos\theta_i + n_1\cos\theta_t}$$

代码实现(`CalculateTransmissionWaveLoss.cpp:9-28`):
```cpp
void CalculateTransmissionWaveCoefficientBase(
    double thetai, double thetat,
    const Complex& n1, const Complex& n2,
    Complex& te, Complex& tm) {
    
    double cos_i = cos(thetai);
    double cos_t = cos(thetat);
    
    te = (n1 * cos_i * 2.0) / (n1 * cos_t + n2 * cos_i);
    tm = (n1 * cos_i * 2.0) / (n2 * cos_t + n1 * cos_i);
}
```

### 2.9 路径损耗计算

#### 2.9.1 自由空间损耗

$$L_{fs}(dB) = 20\log_{10}(d) + 20\log_{10}(f) - 147.55$$

其中：
- $d$：距离(m)
- $f$：频率(Hz)

#### 2.9.2 路径电场

$$E = E_0 \prod_i \tau_i \cdot \prod_j \Gamma_j \cdot e^{-jkd}$$

其中$\tau_i$为各次透射系数，$\Gamma_j$为各次反射系数。

---

## 三、核心数据结构

### 3.1 几何元素

#### 3.1.1 点(Point3D)

```cpp
struct Point3D {
    double x, y, z;  // 三维坐标
};
```

#### 3.1.2 射线(Ray3D)

```cpp
struct Ray3D {
    Point3D o;      // 原点
    Point3D vec;    // 方向向量(单位向量)
};
```

#### 3.1.3 三角形(Triangle3D)

```cpp
struct Triangle3D {
    int triangleP1Index;    // 顶点1索引
    int triangleP2Index;   // 顶点2索引
    int triangleP3Index;  // 顶点3索引
    int topMaterialTypeNumber;    // 上表面材质类型
    int bottomMaterialTypeNumber; // 下表面材质类型
    float roughness;       // 粗糙度
    Point3D n;          // 法线向量
};
```

#### 3.1.4 墙角(Corner3D)

```cpp
struct Corner3D {
    int p1Index;         // 棱点1索引
    int p2Index;        // 棱点2索引
    int p3Face1Index;   // 面1的第三个点索引
    int p3Face2Index;  // 面2的第三个点索引
    int face1Index;    // 邻接面1索引
    int face2Index;   // 邻接面2索引
};
```

#### 3.1.5 场景(Scenario3D)

```cpp
struct Scenario3D {
    int pointsCount;
    Point3D* scenario_point3d_set;     // 点数组
    
    int trianglesCount;
    Triangle3D* scenario_triangle3d_set;  // 三角形数组
    
    int cornersCount;
    Corner3D* scenario_corner3d_set;   // 墙角数组
};
```

### 3.2 加速结构

#### 3.2.1 AABB包围盒

```cpp
struct AABB {
    double Surface;           // 表面积
    Point3D Min;            // 最小坐标
    Point3D Max;            // 最大坐标
    std::set<int> GeometryElementIndex;  // 包含的几何元素索引
};
```

#### 3.2.2 BVH节点

```cpp
struct AabbBvhTreeNode {
    bool IsLeaf;                    // 是否叶子节点
    double Radius;              // 包围球半径
    Point3D Center;           // 包围球中心
    std::unique_ptr<AabbBvhTreeNode> Left;   // 左子节点
    std::unique_ptr<AabbBvhTreeNode> Right;  // 右子节点
    AABB Bound;               // AABB包围盒
};
```

#### 3.2.3 加速结构预计算

```cpp
struct ExhaustTriangleAccelerateStruct {
    Point3D scenarioTriangleP1;   // 三角形顶点
    Point3D scenarioTriangleN;  // 法线
    Point3D scenarioE1;         // 边向量E1
    Point3D scenarioE2;        // 边向量E2
    Point3D scenarioE1E2;       // E1×E2
    Point3D scenarioE2E1;       // E2×E1
};

struct ExhaustCornerAccelerateStruct {
    Point3D start;             // 线段起点
    Point3D end;             // 线段终点
    Point3D unit_vec;        // 单位方向向量
    double length;          // 长度
};
```

### 3.3 材质参数

```cpp
struct MaterialObject {
    int typeNumber;           // 材质类型编号
    long long frequency;      // 频率(Hz)
    double relativePermittivity;  // 相对介电常数
    double conductivity;   // 电导率(S/m)
    char materialName[64]; // 材质名称
};
```

### 3.4 天线

#### 3.4.1 发射天线

```cpp
struct TransmittingAntenna {
    int transmittingAntennaId;
    Point3D location;              // 天线位置
    long long frequencys[10];      // 工作频率
    int frequencyBandwidthSize;
    double transmitPower;         // 发射功率
    int materialTypeNumber;        // 材质类型
    int polarization3DModelId;   // 极化模型ID
    int radiationPatternId;     // 辐射方向图ID
    
    int receiversCount;
    ReceiverAntenna* receivers;   // 接收天线数组
};
```

#### 3.4.2 接收天线

```cpp
struct ReceiverAntenna {
    int receiverAntennaId;
    AntennaProperty {
        Point3D location;
        // ...
    } antennaProperty;
};
```

### 3.5 射线追踪路径

```cpp
struct RayTracingGeometricPathNode {
    PropagationType type;     // 传播类型
    int objectIndex;    // 几何对象索引
    Point3D location; // 位置
};

enum PropagationType {
    TransmittingAntenna,
    ReceiverAntenna,
    Reflection,
    Transmission,
    Diffraction,
    DiffuseScattering
};
```

---

## 四、从初始化到结果输出的完整流程

### 4.1 主程序入口

文件：`main.cpp:14`

```cpp
int main(int args, char* argv[]) {
    std::cout << "start main." << std::endl;
    XdQRtSbr3DStd::XdQRtSbr3DStart();
    std::cout << "successful !" << std::endl;
    return 0;
}
```

### 4.2 详细流程步骤

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    XdQRtSbr3DStart()                            │
│                    主初始化函数                                   │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────��───────┐
│ Step 1: 材质初始化                                              │
│ 函数: RT_Init_MaterialSet()                                    │
│ 读入CSV材质表，构造MaterialSet                                 │
│ XdQRtSbr3D.cpp:113-132                                        │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ Step 2: 场景初始化                                             │
│ 函数: RT_Init_Scenario3D()                                      │
│ 读入场景几何数据:                                               │
│   - 点坐标CSV                                               │
│   - 三角形索引CSV                                           │
│   - 墙角索引CSV                                            │
│ XdQRtSbr3D.cpp:135-263                                       │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ Step 3: 发射天线初始化                                          │
│ 函数: RT_Init_TransmitterAntenna()                                │
│   - 读极化模型JSON                                         │
│   - 读天线方向图                                           │
│   - 读接收天线CSV                                         │
│ XdQRtSbr3D.cpp:344-427                                       │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ Step 4: 空间加速结构初始化                                      │
│ 函数: InitializeAabbBvhTree()                               │
│   - 构建三角形AABB-BVH树                                    │
│   - 构建墙角AABB-BVH树                                     │
│   - 预计算加速结构                                          │
│ Ray3DIntersectGeometry3DElementsAabbBvhTree4.cpp:1378-1391         │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ Step 5: 射线生成                                              │
│ 函数: InitSBRLaunchRayVec()                                    │
│ 使用Fibonacci球面分布生成均匀射线方向                             │
│ SbrRayGeneratedByTransmittingAntenna.cpp:156-199             │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
        ┌─────────────────────────────────────────────────────┐
        │          循环遍历每条射线                           │
        │  RtSbr3DForRay3DFindPathSingleThread_SIMO()      │
        │  XdQRtSbr3D.cpp:943-982                         │
        └─────────────────────────────────────────────────────┘
                                │
                                ▼
        ┌─────────────────────────────────────────────────────┐
        │ 5.1 射线-场景相交检测                            │
        │ 函数: CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst()
        │ 使用BVH树加速查找相交几何元素                         │
        └─────────────────────────────────────────────────────┘
                                │
                                ▼
        ┌─────────────────────────────────────────────────────┐
        │ 5.2 路径搜索(递归)                              │
        │ 函数: SbrFindGeometricPathSIMO()                 │
        │   - 检测反射                                   │
        │   - 检测透射                                 │
        │   - 检测绕射                                 │
        │   - 检测散射                                 │
        └─────────────────────────────────────────────────────┘
                                │
                                ▼
        ┌─────────────────────────────────────────────────────┐
        │ 5.3 检测是否到达接收天线                       │
        │ 函数: InteractionBetweenRayAndRx()                 │
        │ 使用锥形方法判断射线是否到达接收天线            │
        └─────────────────────────────────────────────────────┘
                                │
                                ▼
        ┌─────────────────────────────────────────────────────┐
        │ 5.4 保存路径                                   │
        │ 将找到的有效路径保存到路径库中                    │
        └─────────────────────────────────────────────────────┘
                                │
                                ▼
┌───────────────────────────────────────────────────────────��─��───────────────┐
│ Step 6: 波传播损耗计算                                         │
│ 函数: CalculateWaveImpactResponseDBm()                     │
│ 对每条路径计算电场衰减                                       │
└─────────────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│ Step 7: 结果输出                                              │
│ 输出路径文件、电场文件等                                     │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 五、射线生成模块

### 5.1 Fibonacci球面分布

为保证射线在球面上均匀分布，使用Fibonacci螺旋线方法。

#### 5.1.1 数学原理

在球面上均匀分布N个点，使用黄金角$\phi = \pi(3-\sqrt{5})$：

$$y = 1 - \frac{i}{N-1} \cdot 2$$
$$radius = \sqrt{1-y^2}$$
$$\theta = \phi \cdot i$$

$$x = \cos\theta \cdot radius$$
$$z = \sin\theta \cdot radius$$

#### 5.1.2 代码实现

文件：`SbrRayGeneratedByTransmittingAntenna.cpp:156-199`

```cpp
void InitSBRLaunchRayVec(size_t num, std::vector<Point3DStd::Point3D>& txRayVec) {
    
    txRayVec.resize(num);
    
    // 黄金角
    double phi = GlobalConstantStd::Pi * (3.0 - sqrt(5.0));
    double tx = 0.0, ty = 0.0, tz = 0.0;
    
    for (int i = 0; i < num; ++i) {
        // y坐标: 从1到-1均匀分布
        double y = 1.0 - (i / (num - 1.0)) * 2.0;
        
        // 当前高度的圆的半径
        double radius = sqrt(1.0 - y * y);
        
        // 黄金角螺旋
        double theta = phi * i;
        
        // 球面坐标转换
        double x = cos(theta) * radius + tx;
        double z = sin(theta) * radius + tz;
        y += ty;
        
        // 存储方向向量
        txRayVec[i].x = x;
        txRayVec[i].y = y;
        txRayVec[i].z = z;
    }
}
```

### 5.2 Marsaglia方法

另一种生成均匀球面分布的方法，使用拒绝采样。

#### 5.2.1 数学原理

```cpp
void GetUniformUnitVectorByMarsagliaMethod(int num, std::vector<Point3D>& vecPoints) {
    CRandom sita(z1, 0.0), pesi(z2, 0.0);
    
    for (; vecPoints.size() < num;) {
        double u = 2 * sita.random - 1.0;
        double v = 2 * pesi.random - 1.0;
        double r2 = u*u + v*v;
        
        if (r2 < 1) {
            // 拒绝采样
            double x = 2 * u * sqrt(1 - r2);
            double y = 2 * v * sqrt(1 - r2);
            double z = 1 - 2 * r2;
            vecPoints.push_back(Point3D(x, y, z));
        }
    }
}
```

---

## 六、空间加速结构模块

### 6.1 AABB包围盒

#### 6.1.1 基本定义

Axis-Aligned Bounding Box (AABB)是与坐标轴对齐的包围盒：

```cpp
struct AABB {
    double Surface;
    Point3D Min;           // 最小点
    Point3D Max;           // 最大点
    std::set<int> GeometryElementIndex;
};
```

#### 6.1.2 表面积计算

代码实现：

```cpp
double CalculateAABBSurfaceArea(const AABB& aabb) {
    double dx = aabb.Max.x - aabb.Min.x;
    double dy = aabb.Max.y - aabb.Min.y;
    double dz = aabb.Max.z - aabb.Min.z;
    
    dx = fmax(kAABBAccuracy, dx);
    dy = fmax(kAABBAccuracy, dy);
    dz = fmax(kAABBAccuracy, dz);
    
    double surface = 0.02 * (dx * dy + dy * dz + dz * dx);
    return surface;
}
```

### 6.2 BVH树

#### 6.2.1 SAH代价函数

Surface Area Heuristic (SAH)用于选择最优分割面：

$$Cost = N_L \cdot SA(B_L) + N_R \cdot SA(B_R)$$

其中：
- $N_L, N_R$：左右子树的三角形数量
- $SA(B_L), SA(B_R)$：左右子树的AABB表面积

#### 6.2.2 BVH构建算法

文件：`Ray3DIntersectGeometry3DElementsAabbBvhTree4.cpp:710-783`

```cpp
std::unique_ptr<AabbBvhTreeNode> BuildAabbBvhTreeRecursiveTriangle3D(
    int depth, int maxLeafSize, int maxDepth,
    const Scenario3D& scenario,
    const std::vector<Point3D>& center_points,
    std::vector<int>& indices) {
    
    auto root_node = std::make_unique<AabbBvhTreeNode>();
    root_node->IsLeaf = false;
    
    // 计算当前节点的AABB
    if (!GetAABBByTriangles(scenario, indices, root_node->Bound))
        return std::unique_ptr<AabbBvhTreeNode>();
    
    // 计算包围球中心
    root_node->Center = 0.5 * (Bound.Min + Bound.Max);
    
    // 计算包围球半径
    double dx = Bound.Max.x - Bound.Min.x;
    double dy = Bound.Max.y - Bound.Min.y;
    double dz = Bound.Max.z - Bound.Min.z;
    root_node->Radius = 0.5 * sqrt(dx*dx + dy*dy + dz*dz);
    
    // 终止条件：达到叶子节点
    if (indices.size() <= maxLeafSize || depth >= maxDepth) {
        root_node->IsLeaf = true;
        return root_node;
    }
    
    // 使用SAH找最优分割
    SahSplit split = FindBestSplitTriangle3D(scenario, center_points, indices);
    
    // 递归构建子树
    if (!split.LeftBoxesIndex.empty()) {
        root_node->Left = BuildAabbBvhTreeRecursiveTriangle3D(
            depth+1, maxLeafSize, maxDepth, scenario, center_points, split.LeftBoxesIndex);
    }
    if (!split.RightBoxesIndex.empty()) {
        root_node->Right = BuildAabbBvhTreeRecursiveTriangle3D(
            depth+1, maxLeafSize, maxDepth, scenario, center_points, split.RightBoxesIndex);
    }
    
    return root_node;
}
```

### 6.3 射线-AABB相交检测

使用Slab方法：

```cpp
bool Ray3DIntersectAabb(
    const Point3D& ray_origin,
    const Point3D& ray_direction,
    const AABB& aabb) {
    
    for (int axis = 0; axis < 3; ++axis) {
        double t1 = (GetAxis(aabb.Min, axis) - GetAxis(ray_origin, axis)) / GetAxis(ray_direction, axis);
        double t2 = (GetAxis(aabb.Max, axis) - GetAxis(ray_origin, axis)) / GetAxis(ray_direction, axis);
        
        double tmin = min(t1, t2);
        double tmax = max(t1, t2);
        
        t ENTER = max(t ENTER, tmin);
        t EXIT = min(t EXIT, tmax);
        
        if (t EXIT < t ENTER) return false;
    }
    return true;
}
```

### 6.4 射线-球体相交(用于BVH加速)

```cpp
bool Ray3DIntersectBall(
    const Point3D& ray_origin,
    const Point3D& ray_direction,
    const Point3D& center,
    double radius) {
    
    Point3D op = ray_origin - center;
    double length_op = |op|;
    
    if (length_op < radius) return true;
    
    double dot = op · ray_direction;
    if (dot > 0) return false;
    
    double h_radius = sqrt(length_op*length_op - dot*dot);
    if (h_radius < radius) return true;
    
    return false;
}
```

---

## 七、射线与几何元相交检测模块

### 7.1 射线-三角形相交

#### 7.1.1 预加速结构

为提高检测效率，预计算三角形的相关向量：

```cpp
void InitExhaustTriangleAccelerateStruct(
    const Scenario3D& scenario,
    std::vector<ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs) {
    
    for (int loop = 0; loop < scenario.trianglesCount; ++loop) {
        // 获取顶点
        Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
        Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
        Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
        
        // 计算边向量
        Point3D E1 = p1 - p2;
        Point3D E2 = p1 - p3;
        
        // 预计算叉积
        exhaustTriangleAccelerateStructs[loop].scenarioE1E2 = Cross(E1, E2);
        exhaustTriangleAccelerateStructs[loop].scenarioE2E1 = Cross(E2, E1);
        
        // 存储顶点
        exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1 = p1;
    }
}
```

#### 7.1.2 加速相交检测

```cpp
void CalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(
    const Point3D& ray_origin, const Point3D& ray_direction,
    const std::vector<ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
    const std::set<int>& triangleAccelerateStructIndex,
    Ray3DIntersectGeometry3DElementResult& result) {
    
    result.distance = RayMaxMovingDistance;
    result.elementIndex = -1;
    
    for (auto loop : triangleAccelerateStructIndex) {
        Point3D curRes;
        double curDis = RayMaxMovingDistance;
        
        if (Intersect_Ray3D_Triangle3D_plus(
            ray_origin, ray_direction,
            exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
            ...)) {
            
            if (result.distance > curDis) {
                result.type = 0;
                result.distance = curDis;
                result.elementIndex = loop;
                result.location = curRes;
            }
        }
    }
}
```

### 7.2 射线-墙角相交

#### 7.2.1 预加速结构

```cpp
void InitExhaustCornerAccelerateStruct(
    const Scenario3D& scenario,
    std::vector<ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs) {
    
    for (int loop = 0; loop < scenario.cornersCount; ++loop) {
        Point3D start = scenario.scenario_point3d_set[corner.p1Index];
        Point3D end = scenario.scenario_point3d_set[corner.p2Index];
        
        Point3D vec = end - start;
        double length = |vec|;
        Point3D unit_vec = vec / length;
        
        exhaustCornerAccelerateStructs[loop].start = start;
        exhaustCornerAccelerateStructs[loop].end = end;
        exhaustCornerAccelerateStructs[loop].unit_vec = unit_vec;
        exhaustCornerAccelerateStructs[loop].length = length;
    }
}
```

#### 7.2.2 墙角相交检测算法

使用两直线间公垂线距离：

```cpp
bool Intersect_Ray3D_Corner3D(
    double radiusCorner,
    const Point3D& O, const Point3D& rayVec,
    const Point3D& seg_start, const Point3D& seg_end,
    double seg_length, const Point3D& seg_unit_vec,
    double& rayLength, Point3D& res) {
    
    // 获取点到直线的距离
    double pToSeg = GetDistancePoint3DLine3D_plus_unsafe(O, seg_start, seg_unit_vec);
    if (pToSeg < Eps) return false;  // 在线段上
    
    // 获取两直线的公垂线
    double curSegSegDis = GetDistanceLine3DLine3D_plus_unsafe(O, rayVec, seg_start, seg_unit_vec, ...);
    
    if (curSegSegDis <= radiusCorner) {
        // 在绕射半径内
        rayLength = |O - res2|;
        res = res2;
        return true;
    }
    return false;
}
```

---

## 八、射线追踪路径搜索模块

### 8.1 主搜索函数

文件：`XdQRtSbr3D.cpp:794-938`

```cpp
void SbrFindGeometricPathSIMO(
    const RayEjectionParameterConfig& rayEjectionParameterConfig,
    const Ray3D& ray,
    std::vector<RayTracingGeometricPathNode>& root) {
    
    if (rayEjectionParameterConfig.ejectionsMaxTotalNumber < 1) return;
    
    // 1. 射线与场景求交
    Ray3DScenario3DIntersectResult rayScenarioIntersectResult;
    rayScenarioIntersectResult.ray3DTriangle3DIntersectResult =
        RayScenarioTriangle3DIntersect(ray, triangleAccelerateStructDatabase);
    
    // 2. 检测是否到达接收天线
    InteractionBetweenRayAndRx(ray, rayScenarioIntersectResult, root);
    
    // 3. 无相交则返回
    if (rayScenarioIntersectResult.index == -1) return;
    
    // 4. 处理反射
    if (reflect) {
        // 计算反射射线
        if (BuildReflectionRay(ray.o, intersectPoint, faceNormal, newRay, thetai)) {
            root.emplace_back(ReflectionNode);
            // 递归追踪反射射线
            SbrFindGeometricPathSIMO(newRayEjectionParameterConfig, newRay, root);
            root.pop_back();
        }
    }
    
    // 5. 处理透射
    if (transmission) {
        // 计算透射射线
        if (BuildTransmissionRay_RayTracingGeometricPathNode(...)) {
            root.emplace_back(TransmissionNode);
            // 递归追踪透射射线
            SbrFindGeometricPathSIMO(newRayEjectionParameterConfig, newRay, root);
            root.pop_back();
        }
    }
    
    // 6. 处理绕射
    if (ejectionsOfDiffraction > 0) {
        // 计算绕射射线
        SbrFindGeometricPathByDiffraction(...);
    }
}
```

### 8.2 路径搜索策略

| 参数 | 说明 |
|------|------|
| ejectionsMaxTotalNumber | 最大弹跳次数 |
| ejectionsOfReflectionMaxNumber | 最大反射次数 |
| ejectionsOfTransmissionMaxNumber | 最大透射次数 |
| ejectionsOfDiffractionMaxNumber | 最大绕射次数 |
| ejectionsOfDiffuseScatteringMaxNumber | 最大散射次数 |
| switchOfLos | 是否包含直达路径 |

---

## 九、波的传播损耗计算模块

### 9.1 反射损耗

文件：`CalculateReflectionWaveLoss.cpp`

```cpp
void CalculateReflectionWaveCoefficient(
    long long frequency,
    double thetai,
    double er1, double sigma1,  // 介质1
    double er2, double sigma2,  // 介质2
    double& te_real, double& te_imag,  // TE系数实部虚部
    double& tm_real, double& tm_imag) {  // TM系数实部虚部
    
    // 1. 计算复介电常数
    Complex ce1 = CalculateComplexPermittivity(frequency, er1, sigma1);
    Complex ce2 = CalculateComplexPermittivity(frequency, er2, sigma2);
    
    // 2. 计算复折射率
    Complex n1 = CalculateComplexRefractiveIndexOfMedium(ce1);
    Complex n2 = CalculateComplexRefractiveIndexOfMedium(ce2);
    
    // 3. 计算透射角
    double thetat;
    if (!CalculateThetat(thetai, n1, n2, thetat)) {
        te_real = 1.0; te_imag = 0.0;
        tm_real = 1.0; tm_imag = 0.0;
        return;
    }
    
    // 4. 计算反射系数
    Complex r_te, r_tm;
    CalculateReflectionWaveCoefficientBase(thetai, thetat, n1, n2, r_te, r_tm);
    
    te_real = r_te.real; te_imag = r_te.imag;
    tm_real = r_tm.real; tm_imag = r_tm.imag;
}
```

### 9.2 透射损耗

文件：`CalculateTransmissionWaveLoss.cpp`

```cpp
bool CalculateTransmissionWaveCoefficient(
    long long frequency,
    double thetai,
    double er1, double sigma1,
    double er2, double sigma2,
    double& te_real, double& te_imag,
    double& tm_real, double& tm_imag,
    double& thetat) {
    
    // 1. 复介电常数
    Complex ce1 = CalculateComplexPermittivity(frequency, er1, sigma1);
    Complex ce2 = CalculateComplexPermittivity(frequency, er2, sigma2);
    
    // 2. 复折射率
    Complex n1 = CalculateComplexRefractiveIndexOfMedium(ce1);
    Complex n2 = CalculateComplexRefractiveIndexOfMedium(ce2);
    
    // 3. 透射角
    if (!CalculateThetat(thetai, n1, n2, thetat)) {
        return false;
    }
    
    // 4. 透射系数
    Complex t_te, t_tm;
    CalculateTransmissionWaveCoefficientBase(thetai, thetat, n1, n2, t_te, t_tm);
    
    te_real = t_te.real; te_imag = t_te.imag;
    tm_real = t_tm.real; tm_imag = t_tm.imag;
    
    return true;
}
```

### 9.3 绕射损耗

文件：`CalculateDiffractionWaveLoss.cpp`

```cpp
bool CalculateDiffractionWaveLossHolmPlus(
    long long frequency,
    double beta0,    // 入射波与棱边的夹角
    double phi1,      // 入射面与棱边的夹角
    double phi2,      // 绕射线与棱边的夹角
    double phiE,     // 楔形角度
    double er, double sigma,
    Complex& te, Complex& tm) {
    
    // 参数有效性检查
    if (!CalculateDiffractionWaveLossCheck(beta0, phi1, phi2, phiE))
        return false;
    
    // 波数
    double k = 2 * PI * frequency / C;
    double sin_beta0 = sin(beta0);
    double n = phiE / PI;
    
    // 计算D1-D4绕射系数
    Complex D1, D2, D3, D4;
    CalD1234(n, k, sin_beta0, phi1, phi2, D1, D2, D3, D4);
    
    // 计算反射系数
    double thetai0 = 0.5 * PI - phi1;
    Complex r_0_te, r_0_tm, r_n_te, r_n_tm;
    CalculateReflectionWaveCoefficient(frequency, thetai0, 1.0, 0.0, er, sigma, ...);
    
    // 组合Holm+公式
    te = D1 + D2 + r_0_te * D3 + r_n_te * D4;
    tm = D1 + D2 + r_0_tm * D3 + r_n_tm * D4;
    
    return true;
}
```

---

## 十、材质参数模块

### 10.1 复介电常数

```cpp
Complex CalculateComplexPermittivity(long long frequency, double er, double sigma) {
    double omega = 2 * PI * frequency;
    double real = er * EPSILON_0;
    double imag = -sigma / omega;
    return Complex(real, imag);
}
```

其中：
- $\varepsilon_0 = 8.8541878128 \times 10^{-12}$ F/m
- $EPSILON_0$ 为真空介电常数

### 10.2 复折射率

```cpp
Complex CalculateComplexRefractiveIndexOfMedium(const Complex& ce) {
    double real = ce.real;
    double imag = ce.imag;
    double mag = sqrt(real*real + imag*imag);
    double phase = atan2(imag, real) / 2;
    double sqrt_mag = sqrt(mag);
    return Complex(sqrt_mag * cos(phase), sqrt_mag * sin(phase));
}
```

---

## 十一、天线模型模块

### 11.1 极化模型

支持多极化分量叠加：

```cpp
struct OneAntennaLinearPolarization3D {
    double weight;           // 权重
    LinearPolarization3DObject {
        double phi0;    // 初相
        Point3D vec;   // 极化方向
    } linearPolarization3DObject;
};
```

### 11.2 辐射方向图

从JSON文件读取三维辐射方向图：

- 分辨率：360° × 181°
- 存储为增益值数组

```cpp
struct AntennaRadiationPattern3DModel {
    int radiationPatternId;
    int rows;           // 360
    int columns;       // 181
    double* radiationPattern;  // 增益值[dB]
};
```

---

## 十二、输出结果模块

### 12.1 天线路径输出

每个MIMO路径输出：
- 发射天线ID
- 接收天线ID
- 路径点序列
- 各跳的类型和索引

### 12.2 电场结果输出

```cpp
struct AntennaMIMOPathElectric {
    int txId;             // 发射天线ID
    int rxId;             // 接收天线ID
    double pathLoss;     // 路径损耗[dB]
    Complex E;          // 电场复数
    double delay;       // 时延[ns]
    // ...
};
```

---

## 十三、关键技术细节

### 13.1 浮点数精度控制

```cpp
const double Eps = 1e-6;

bool IsZero(double d) {
    return d <= Eps;
}
```

### 13.2 向量归一化(安全版)

```cpp
Point3D Normalization_Point3D_safe(const Point3D& p) {
    double len = Length_Point3D(p);
    if (len < Eps * Eps) {
        std::cout << "Point3D Normalization非法参数" << std::endl;
        return CreatePoint3D(0.0, 0.0, 0.0);
    }
    return p / len;
}
```

### 13.3 全内反射检测

```cpp
bool CalculateThetat(double thetai, const Complex& n1, const Complex& n2, double& thetat) {
    double sin_thetai = sin(thetai);
    Complex ratio = n1 / n2;
    double ratio_mag = |ratio|;
    double sin_thetat = ratio_mag * sin_thetai;
    
    if (sin_thetat > 1.0) {
        // 全内反射，无透射
        return false;
    }
    thetat = asin(sin_thetat);
    return true;
}
```

### 13.4 多线程并行

```cpp
void RtSbr3DForRay3DFindPathMultiThread_SIMO(
    const MultithreadParameterConfig& multithreadParameterConfig) {
    
    int indexGap = multithreadParameterConfig.multithreadConfigThreadOneCpuCalNum;
    std::vector<int> starts;
    
    // 分片
    for (int i = 0; i < globalRayVecs_size; i += indexGap) {
        starts.emplace_back(i);
    }
    
    ThreadPoolStd::ThreadPool threadPool;
    
    for (int i = 0; i < starts.size(); ++i) {
        MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO task;
        task.start = starts[i];
        task.end = starts[i] + indexGap;
        
        threadPool.submit(RtSbr3DForRay3DFindPathMultiThread_SIMO_Core, task);
    }
    
    threadPool.join();
}
```

---

## 十四、算法复杂度分析

### 14.1 空间复杂度

| 数据结构 | 复杂度 |
|---------|--------|
| 场景点 | O(N) |
| 三角形 | O(M) |
| AABB-BVH树 | O(M) |
| 墙角 | O(K) |

### 14.2 时间复杂度

| 操作 | 平均复杂度 | 最坏复杂度 |
|-----|----------|----------|
| 射线-场景相交(O(N)个三角形) | O(log N) | O(N) |
| 路径搜索(弹跳次数B) | O(B log N) | O(B×N) |

其中N为三角形数量，B为射线弹跳次数。

### 14.3 优化策略

1. **BVH加速**：从O(N)降至O(log N)
2. **预计算加速结构**：减少实时计算量
3. **多线程并行**：线性加速比
4. **Early-out**：检测到交点立即返回
5. **内存池**：减少内存分配开销

---

## 附录：关键常量

```cpp
// 物理常量
const double PI = 3.14159265358979323846;
const double C = 299792458.0;                    // 光速(m/s)
const double EPSILON_0 = 8.8541878128e-12;       // 真空介电常数(F/m)
const double MU_0 = 1.2566370614e-6;          // 真空磁导率(H/m)

// 工程常量
const double Eps = 1e-6;
const double RayMaxMovingDistance = 1e15;
const double BoundingBoxLength = 1e10;
```

---

## 参考文献

1. [1] SBR (Shooting and Bouncing Ray) - 射线发射法基础理论
2. [2] Möller–Trumbore Algorithm - 射线三角形相交检测
3. [3] AABB-BVH - 空间加速结构
4. [4] SAH (Surface Area Heuristic) - BVH分割代价函数
5. [5] UTD (Uniform Theory of Diffraction) - 一致性绕射理论
6. [6] Fresnel Equations - 电磁波反射透射系数

---

*文档版本: v1.0*
*生成日期: 2024*
*���析源码版本: RT.XD.SBR.CGAL.25.05*