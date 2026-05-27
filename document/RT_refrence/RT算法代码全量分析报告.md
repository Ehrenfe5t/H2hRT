# RT.XD.SBR.CGAL.25.05 算法代码全量分析报告

生成时间: 2025-05
代码根目录: E:\RT_claude\RT_claude\RTnew\算法\RT.XD.SBR.CGAL.25.05\RT.XD.SBR.CGAL.25.05

---

## 目录

1. 项目整体架构
2. 核心调度层：main.cpp + XdQRtSbr3D.cpp
3. 场景预处理层：几何路径构建 + BVH加速结构
4. 路径求解层：漫散射镜像法 + 绕射方程法
5. 电磁损耗计算层：反射 / 透射 / 绕射 / 漫散射
6. 电场响应总装：圆极化3D全路径积分
7. SBR射线生成与路径节点树
8. 几何加速结构：BVH球体树 + AABB包围盒
9. 基础几何算法：三角面数据 + 相交检测
10. 系统设计总结与关键数据流

---

<!-- SECTION_END -->

---

## 第一章：项目整体架构

### 1.1 规模统计

| 类型 | 数量 |
|------|------|
| .cpp 文件 | 243 |
| .h 文件 | 276 |
| 合计 | ~519 |

代码总量约 64 万行级别（含注释），是一个大型工业级 C++ 射线追踪仿真系统。

### 1.2 顶层目录结构

```
RT.XD.SBR.CGAL.25.05/
├── main.cpp                                       主入口（19行）
├── CoreCode/                                      核心共享代码层（~160 cpp+h）
├── 0.DxQCalculateWaveImpactResponseDBmModule/     电磁场响应计算模块（接口）
├── 0.DxQCalculateWaveImpactResponseDBmModule.Impl/电磁场响应计算模块（实现）
├── 0.HdQGeometryModule/                           几何运算模块（17个子模块）
├── 0.Ray3DIntersectGeometry3DElementsModule/      射线-几何体相交模块（接口）
├── 0.Ray3DIntersectGeometry3DElementsModule.Impl/ 射线-几何体相交模块（实现）
├── 0.QzQAccelerateRayIntersectScenarioModule/     射线-场景加速相交模块
├── 0.LxQSbr3DRay3DGenerationModule/              SBR射线生成模块
├── 0.QzQAntennaModule/                            天线模块
├── 0.DxQElectricalParametersOfMaterialModule/    材料电气参数模块
├── 0.Solve1SPathByIm3DModule/                    镜像法(IM3D)一次散射路径求解（接口）
├── 0.Solve1SPathByIm3DModule.Impl/               镜像法路径求解（实现）
├── 0.SolveOneTimeDiffractionPathByEquationModule/ 方程法单次绕射路径求解（接口）
└── 0.SolveOneTimeDiffractionPathByEquationModule.Impl/ 方程法绕射路径求解（实现）
```

### 1.3 前缀命名规范

| 前缀 | 含义 | 示例 |
|------|------|------|
| Xd | 顶层/主调度 (eXternal Dispatcher) | XdQRtSbr3D |
| Hd | 核心底层 (Hard/Core layer) | HdQBuildBvhBall3D |
| Dx | 数据/参数/场景 (Data) | DxQTriangle3D |
| Lx | 链路/多径 (Link) | LxQMultiPathNodeInfo |
| Qz | 数学/几何算子 (Quantitative) | QzQGeometry3DOperate |

### 1.4 核心模块分类（11类）

**A. 主入口 [MainEntry]**
- main.cpp：调用 XdQRtSbr3DStd::XdQRtSbr3DStart()，19行
- XdQRtSbr3D.h/.cpp：顶层调度类，64KB，1461行

**B. 核心算法 [CoreAlgorithm]**
- HdQBuildGeometryPath系列：D/DD/DR/RD 几何路径构建（含可见性测试变体）
- HdQBuildBvhBall3D：BVH球体加速结构构建
- Solve1SPathByIm3D：镜像法一次散射精确路径求解
- SolveOneTimeDiffractionPathByEquation：方程法单次绕射精确路径求解
- Calculate*WaveLoss系列：反射/透射/绕射/漫散射电磁损耗计算

**C. 几何计算 [Geometry]**（17个子模块）
- Geometry3DOperate.*：点/法向/长度/距离/面积/坐标系/离散化等
- Geometry3DIntersect：射线-三角面/AABB/线段相交算法族

**D. 配置 [Config]**（20+个配置类）
- 每个 XxxConfig 均有对应 XxxConfigJsonOperate，形成标准 JSON 读写体系
- 覆盖：射线参数/BVH参数/弹射次数/漫反射/多线程/输出/材料优化等

**E. 天线 [Antenna]**
- 支持 SISO/SIMO/MIMO 三种模式，含天线方向图和极化 3D 数据库

**F. 多径节点 [MultiPathNode]**
- 按传播机制分类：反射/透射/绕射/漫散射/TX/RX 各自独立节点类

**G. 射线追踪/场景 [RayTracing]**
- 双加速结构：BVH球体树（粗筛）+ AABB-BVH树（精确求交）
- 路径节点树：支持 SISO/SIMO/MIMO 分叉路径记录

**H. 材料电磁参数 [Material/EM]**
- 按频率存储复数介电常数（实部+虚部）
- 支持色散材料（频率相关的 ε_r 和 σ）

**I. 极化 [Polarization]**
- 支持线性/圆极化，2D/3D极化对象，多线性极化数据库

**J. 工具 [Utils]**
- 字符串/JSON/CSV/线程安全向量/线程池/时间/dBm计算等

**K. 数值求解/优化 [Solver]**
- 1/2/3元方程组求解，材料参数线性优化和遗传算法优化

### 1.5 整体技术路线

本项目实现了一套完整的 **3D射线追踪电磁仿真系统**，算法核心为：

```
SBR（Shooting and Bouncing Ray）主追踪
  + IM3D（镜像法）精确一次散射路径补充
  + 方程法精确单次绕射路径补充
  + UTD（统一绕射理论）损耗计算
  + Fresnel 反射/透射系数
  + Degli-Esposti 漫散射模型
  + 圆极化3D电场积分
```

支持 SISO/SIMO/MIMO 天线配置，输出 CSV 大小信道参数 + JSON 路径信息。

<!-- CHAPTER1_END -->

---

## 第二章：核心调度层 — main.cpp + XdQRtSbr3D.cpp

### 2.1 main.cpp（19行）

程序唯一入口，无任何业务逻辑，直接调用顶层启动函数：

```cpp
int main() {
    XdQRtSbr3DStd::XdQRtSbr3DStart();
    return 0;
}
```

单向依赖 XdQRtSbr3D.h（命名空间 XdQRtSbr3DStd）。

### 2.2 XdQRtSbr3D.cpp（1461行，64KB）

整个 RT 仿真系统的**顶层调度器**，负责：配置读取 → 场景/材质/天线初始化 → SBR 射线追踪主循环 → 结果输出 → 资源释放。

#### 2.2.1 内部命名空间划分

| 命名空间 | 职责 |
|---|---|
| SY_2025_06 | 初始化函数族（材质、场景、天线、配置映射） |
| SIMOGlobal_2025_06Std | 全局状态（射线向量组、接收天线列表、BVH树指针） |
| InteractionBetweenRayAndRx_2025_06Std | 射线与接收天线碰撞检测（圆锥法/球体法） |
| RtSbr3DForRay3DFindPathSingleThread_2025_06Std | 单线程 SBR 路径搜索 |
| RtSbr3DForRay3DFindPathMultiThread_2025_06Std | 多线程 SBR 路径搜索（ThreadPool） |

#### 2.2.2 关键初始化函数

**RT_Init_Sbr3DFindPathConfig**：JSON 配置字段 → Sbr3DFindPathConfig 结构体映射，含：
- diffuseScatteringAr / Coefficient / RayleighRange（漫反射参数）
- ejectionsMaxTotalNumber / OfReflection / OfTransmission / OfDiffraction（最大弹射次数）
- gapDiffractionRad / gapDiffuseScattering{Azimuth,PitchAngle}（角度步长）
- radiusCorner / radiusRx / rayNumber（几何参数）
- multithreadConfigSwitchOfMultithread / powerThreshold / rebuildEdge

**RT_Init_MaterialSet**：从 CSV 读取材质表（typeNumber, frequency, relativePermittivity, conductivity），malloc 分配数组。

**RT_Init_Scenario3D**：从三个 CSV 分别读取顶点坐标、三角面（含粗糙度/法向量）、角点 Corner3D。

#### 2.2.3 SBR 主循环（SbrFindGeometricPathSIMO，递归回溯）

```
SbrFindGeometricPathSIMO(rayEjectionConfig, ray, root):
  1. 射线与场景三角面求交（BVH球体树加速）
  2. InteractionBetweenRayAndRx → 判断是否击中接收球
  3. 若命中三角面：
     a. reflect=true → BuildReflectionRay → root.push(Reflection) → 递归 → root.pop
     b. transmission=true → BuildTransmissionRay → root.push(Transmission) → 递归 → root.pop
  4. 每次递归 ejectionsMaxTotalNumber-1，子类型计数器各自-1
  5. 弹射次数归零则终止（深度限制）
```

透射支持两种模式：
- realWorldRefraction=true → Snell 折射修正新方向
- realWorldRefraction=false → 沿原方向穿透（简化模式）

#### 2.2.4 XdQRtSbr3DStart 完整流程

```
1. 读取 RtSbr3DForRay3D.Config.json
2. RT_Init_Sbr3DFindPathConfig / MaterialSet / Scenario3D / TransmitterAntenna
3. RTSbr3DCircularPolarization3DPtr(...) → 执行全量 SBR + 电场计算
4. 按开关输出：
   - switchOfBigChannelParameterInfo  → RtoiOutputPowerInformationToCsvFile
   - switchOfSmallChannelParameterInfo→ RtoiOutputImpactResponseInformationToCsvFile
   - switchOfPathInfo                 → RtoiOutputInformationToJsonFile（pathInformation.json）
5. 输出运行时间日志
6. FreeMultiPathNodeInfo_vector_all() 释放多径节点内存
```

#### 2.2.5 多线程机制

ThreadPool 分段 [start,end) 分配射线向量索引，各线程共享全局 BVH 树（只读 _safe 副本），主线程建树后调用 UpdateSafeIdSetInfo 一次性复制，子线程无锁并发查询。

<!-- CHAPTER2_END -->

---

## 第三章：场景预处理层 — 几何路径构建 + BVH加速结构

### 3.1 HdQBuildGeometryPathD.cpp（~430行）

#### 核心用途
基于三角面网格构建几何路径数据库，为后续射线-场景相交检测提供预处理数据：法向量计算、边构建、角点识别、相邻面关系建立。

#### BuildGeometryPathD（主入口）
- 遍历所有三角面，计算法向量（叉积归一化）
- 识别共边三角面对，构建 edge→triangle 映射
- 生成 Corner3D 角点结构（两个相邻面的共边即为一条绕射棱）
- 输出：TriangleAccelerateStructDatabase（含法向量、BVH包围球）

```
法向量计算：
  v1 = P2 - P1
  v2 = P3 - P1
  n = v1 × v2（叉积）
  n = n / |n|（归一化）
```

#### 边构建与角点识别
- 边唯一性键：min(i,j)<<32 | max(i,j)（64位整数）
- 一条边对应两个三角面 → 生成一个 Corner3D（绕射棱）
- rebuildEdge=true 时强制重算，=false 时读取缓存

#### Corner3D 数据结构
```
Corner3D {
    Point3D p1, p2;     // 棱的两端点
    int faceIndex1/2;   // 相邻两个三角面的索引
    Point3D n1, n2;     // 两个面的法向量
    double phiE;        // 楔角 = π - 夹角(n1,n2)
}
```

phiE 即楔形半角，直接用于 UTD 绕射系数中的 n = phiE/π 参数。
使用 Heron 公式验证三角面有效性（面积>阈值才保留）。

### 3.2 HdQBuildBvhBall3D.cpp（~200行）

#### 核心用途
为场景三角面集合构建 BVH（层次包围体树），包围体形状为球体（Ball3D），用于加速射线-场景求交。

#### BvhBall3DNode 结构
```
struct BvhBall3DNode {
    Ball3D ball;         // 包围球（中心+半径）
    int left, right;     // 子节点索引（-1表示叶节点）
    int triangleIndex;   // 叶节点存储三角面索引
}
```

#### 建树流程
```
BuildBvhBall3D(triangles[]):
  1. 为每个三角面计算最小包围球（三顶点外接球）
  2. 递归二分：计算AABB，按最长轴排序，取中点分裂
  3. 内部节点包围球 = 两子节点包围球的最小公共包围球
  4. 叶节点阈值：节点数 ≤ 1
```

#### 最小包围球合并算法
```
MergeBall(b1, b2):
  d = |center2 - center1|
  if d + r2 <= r1: return b1   // b2 在 b1 内
  if d + r1 <= r2: return b2   // b1 在 b2 内
  newR = (d + r1 + r2) / 2
  newCenter = center1 + (center2-center1)/d * (newR - r1)
```

### 3.3 双加速结构并存说明

| 加速结构 | 构建方式 | 用途 |
|---|---|---|
| BVH球体树（HdQBuildBvhBall3D） | 递归二分，球体包围 | 粗筛（快速排除大量无交三角面） |
| AABB-BVH树（Ray3DIntersectGeometry3DElementsAabbBvhTree系列） | 轴对齐包围盒 | 精确射线-三角面求交 |
| 角点加速结构（HdQCornerAccelerateStruct） | 基于 Corner3D 列表 | 绕射棱快速定位 |

两套加速结构互补，BVH球体树用于 SBR 射线-场景求交的外层快速剔除，AABB-BVH 用于可见性判断和路径精确求交。

<!-- CHAPTER3_END -->

---

## 第四章：路径求解层 — 漫散射镜像法 + 绕射方程法

### 4.1 Solve1SPathByIm3D.cpp（451行，17KB）

#### 核心用途
求解发射天线→漫散射点→接收天线的**1次漫散射路径**，用镜像法辅助判断可见性，计算每个三角面漫散射子面的散射贡献。

#### 关键全局状态（模块级静态变量）
```
globalTriangle3DMaterials              // 每个三角面的材质对（上下面）
globalTriangle3DDiffuseScatteringFaces // 每个三角面的漫散射子面列表（细分结果）
globalTxCanSeeTriangle3Ds             // TX能否看到每个三角面（预计算）
globalRxCanSeeTriangle3Ds[rx][tri]    // 每个RX能否看到每个三角面（预计算矩阵）
globalDiffuseScatteringAr             // 漫散射方向因子 Ar
globalDiffuseScatteringCoefficient    // 漫散射系数 S
```

#### 核心函数链

**InitScenarioAndMaterial**：
- 初始化 globalTriangle3DMaterials（从 MaterialSet 按 typeNumber 查找）
- Rayleigh 粗糙度判据：σ_h < λ/(8cosθ) → 光滑面（不做漫散射细分）
- 对粗糙面，按 diffuseScatteringMaxDiscreteSideLength 细分为子三角形列表

**CalGlobalPoint3DCanSeeTriangle3Ds（可见性预计算）**：
- 从 TX/RX 出发，对每个三角面做射线-BVH相交测试
- 相交点恰好在该三角面上（距离最近）→ 可见
- 结果缓存为布尔数组，避免主循环重复计算

**Solve1SPathByIm3DSISO_1s（单散射点路径构建）**：
```
1. 取漫散射子面中心 node_location_1s
2. 验证 TX→node→RX 几何可行性（距离>0）
3. 计算入射角 thetai_1（射线与面法向夹角）
4. 计算反射角 thetar_1（对称反射）
5. 构建3节点 ElectricFieldNode 路径：
   path[0]: TX（type=0），next_distance=length1
   path[1]: 散射点（type=5），存 thetai/thetar/Ar/S/材质/法向量
   path[2]: RX（type=1），pre_distance=length1+length2
```

**Solve1SPathByIm3DSISO（SISO主循环）**：
```
for 每个三角面 tri:
  skip if TX不可见(tri) or RX不可见(tri)
  for 每个漫散射子面 sub:
    Solve1SPathByIm3DSISO_1s(tri, sub, tx, rx, result)
```

### 4.2 SolveOneTimeDiffractionPathByEquation.cpp（926行，25KB）

#### 核心用途
用解析方程法求解发射天线→绕射棱→接收天线的**1次绕射路径**，在三维空间中几何定位绕射节点位置。

#### 解析法定位绕射节点

绕射节点必须同时满足：
1. 在棱线段 [p1, p2] 上
2. TX→节点→RX 满足 Fermat 原理（入射角β等于绕射角）

参数化方法：设棱参数 t ∈ [0,1]，节点 P(t) = p1 + t*(p2-p1)，建立关于 t 的方程，用牛顿法迭代修正。

```
GetRelativeXYZ 系列函数（偏导数）：
  GetRelativeXYZPlus_DelX / DelY / DelZ
  → 计算 ∂F/∂x, ∂F/∂y, ∂F/∂z
  → 用于牛顿法迭代修正绕射节点位置
```

#### UTD 参数计算（Calculate_beta0_phi1_phi2）
```
beta0：入射射线与棱轴的夹角（入射锥角）
phi1：入射面与参考面0°面的夹角
phi2：绕射面与参考面0°面的夹角
```

#### 可见性验证（CalculateCanSeeNode）
```
构造 TX→DiffNode 射线
查询 AABB-BVH 最近相交距离
若相交距离 > |TX-DiffNode| - eps → 可见（无遮挡）
对 RX→DiffNode 同样验证
```

#### 路径节点构建
```
path[0]: TX（type=0），materialIndex=TX所在介质
path[1]: 绕射节点（type=4），存 beta0/phi1/phi2/phiE/材质/棱方向向量
path[2]: RX（type=1）
```

#### 顶层调度
```
InitScenarioAndMaterial + InitAabbBvhTree（仅需三角面结构）
for 每个 RX:
  for 每个角点 corner_i:
    SolveOneTimeDiffractionPathByEquationSISO_OneCorner(...)
```

### 4.3 两种路径求解方法对比

| 方法 | 文件 | 传播机制 | 精度 | 计算量 |
|---|---|---|---|---|
| 镜像法 IM3D | Solve1SPathByIm3D | 漫散射（粗糙面） | 中等（子面离散近似） | O(Ntri × Nsub × Nrx) |
| 方程法 | SolveOneTimeDiffractionPathByEquation | 绕射（棱线） | 高（解析精确定位） | O(Ncorner × Nrx × 牛顿迭代) |
| SBR主追踪 | XdQRtSbr3D | 反射+透射 | 中等（球体接收近似） | O(Nray × 弹射深度) |

<!-- CHAPTER4_END -->

---

## 第五章：电磁损耗计算层 — 反射 / 透射 / 绕射 / 漫散射

### 5.1 公共数学基础 — Complex 类（CalculateWaveImpactResponseDBmComplex.cpp，49行）

所有电磁场计算的底层数学库：

```
struct Complex { double real, imag; }
运算符：+ - * /（Complex×Complex, Complex×double）
Abs()  = sqrt(real² + imag²)       → 计算 |Γ|²（反射功率系数）
Sqrt() = 极坐标开方                 → 从 ε_c 求复折射率 ñ
Conj() = 共轭
```

### 5.2 公共工具库 — CalculateWaveLossCoefficientBase.cpp（159行）

所有损耗计算模块的底层函数集，无任何业务逻辑，仅做数学转换：

| 函数 | 公式 |
|---|---|
| GetMwByDBm | 10^(dBm/10) |
| GetDBmByMw | 10*log10(mw)，mw<1e-25 返回 powerThreshold |
| AddDBm | 先转 W 求和，再转回 dBm（非相干功率叠加） |
| GetLossDbAttenuationInAir（FSPL） | 20*log10(f/MHz) + 20*log10(d/km) + 32.448 |
| CalculateComplexPermittivity | ε_c = ε_r*ε_0 + j*(σ/ω) |
| CalculateComplexRefractiveIndex | ñ = sqrt(ε_c) |
| CalculateThetat（Snell定律） | sin(θt) = \|ñ1\|/\|ñ2\| * sin(θi)，全反射返回 false |

MAX_PATH_LOSS = 327 dBm，用作无效路径占位损耗值。

### 5.3 Fresnel 反射系数 — CalculateReflectionWaveLoss.cpp（66行）

```
输入：frequency, thetai, medium1(ε_r1, σ1), medium2(ε_r2, σ2)

1. ε_c1/2 = ε_r*ε_0 + j*(σ/ω)
2. ñ1/2 = sqrt(ε_c)
3. θt = Snell(θi, ñ1, ñ2)
4. Γ_TE = (ñ1*cosθi - ñ2*cosθt) / (ñ1*cosθi + ñ2*cosθt)   [s极化]
5. Γ_TM = (ñ2*cosθi - ñ1*cosθt) / (ñ2*cosθi + ñ1*cosθt)   [p极化]
6. PL = GetDBmByMw(2 / (|Γ_TE|² + |Γ_TM|²))                [等效非极化]
```

ElectricFieldNode type=2 字段映射：r_t_s_d_thetai_beta0 → thetai，materialIndex1/2 → 两侧介质。

### 5.4 Fresnel 透射系数 — CalculateTransmissionWaveLoss.cpp（200行）

```
1. τ_TE = 2*ñ1*cosθi / (ñ1*cosθi + ñ2*cosθt)
2. τ_TM = 2*ñ1*cosθi / (ñ2*cosθi + ñ1*cosθt)
3. 阻抗修正：ccc = ñ2*cosθt / (ñ1*cosθi)   [能量守恒补偿]
4. PL = GetDBmByMw(2 / (|τ_TE|² + |τ_TM|²) / |ccc|)
```

多层介质相位叠加（有厚度 d 时）：exp(-γ*d)，γ = jk*ñ2*cosθt（复传播常数）。

ElectricFieldNode type=3 字段映射：r_t_s_d_thetai_beta0 → thetai。

### 5.5 UTD 绕射系数 — CalculateDiffractionWaveLoss.cpp（550行）

这是系统电磁计算中最复杂的模块，实现完整的 UTD（统一绕射理论）。

#### 5.5.1 FresnelFunction（Fresnel 过渡函数，三段实现）

```
F(x):
  x < 0.3:   小参数近似，级数展开
              F(x) ≈ sqrt(π*x)*exp(jπ/4)*[1 - j*(2/3)*x + ...]
  x < 5.5:   中等参数，Gauss-Legendre 8点数值积分
              F(x) = 2j*sqrt(x)*exp(jx) * ∫_sqrt(x)^∞ exp(-jt²)dt
  x >= 5.5:  大参数渐近展开
              F(x) ≈ 1 + j/(2x) - 3/(4x²) + ...
```

三段实现保证数值稳定（避免小参数震荡和大参数截断误差）。

#### 5.5.2 CalD1234（UTD 四分量绕射系数）

```
参数：beta0（入射锥角），phi_i（入射方位角），phi_d（绕射方位角），
      n = phiE/π（楔形参数），L = s1*s2/(s1+s2)*sin²(β0)（距离参数），k（波数）

a(β) = 2*cos²(β/2)

D1 = -exp(-jπ/4)/(2n*sqrt(2πk)*sinβ0) * cot((π+(φd-φi))/(2n)) * F(kL*a(φd-φi))
D2 = -exp(-jπ/4)/(2n*sqrt(2πk)*sinβ0) * cot((π-(φd-φi))/(2n)) * F(kL*a(φd+φi))
D3 = -exp(-jπ/4)/(2n*sqrt(2πk)*sinβ0) * cot((π+(φd+φi))/(2n)) * F(kL*a(φd-φi+2nπ))
D4 = -exp(-jπ/4)/(2n*sqrt(2πk)*sinβ0) * cot((π-(φd+φi))/(2n)) * F(kL*a(φd+φi-2nπ))
```

#### 5.5.3 CalDiffractionWaveLoss（主函数）

```
1. k = 2π*f/c
2. n = phiE/π
3. 计算 ε_c → ñ（复折射率）
4. 软极化（TE）：Ds = D1 + D2 + Γ_TE*(D3 + D4)
5. 硬极化（TM）：Dh = D1 + D2 + Γ_TM*(D3 + D4)
6. PL = dBm(2/(|Ds|² + |Dh|²))
```

D3/D4 乘以 Fresnel 反射系数 Γ，修正楔形两侧有限导率界面对绕射的影响（真实建筑材料修正）。

ElectricFieldNode type=4 字段：beta0, phi1, phi2, phiE，materialIndex1/2，棱方向向量 n_x/n_y/n_z。

### 5.6 漫散射损耗 — CalculateDiffuseScatteringWaveLoss.cpp（200行）

基于 Degli-Esposti 改进 Lambertian 模型：

```
散射功率密度 ∝ S * cos^Ar(θs)
其中：θs = 散射角，Ar = 方向集中因子，S = 散射系数

主函数步骤：
1. Fresnel 入射系数 Γ（复用 CalculateReflectionWaveLoss 子函数）
2. σ_eff = (1 - |Γ|²) * S² * area * cos(θi)   [有效散射截面]
3. f(θs) = cos^Ar(θs)                           [方向因子]
4. PL = 10*log10(4π / (σ_eff * f(θs)))          [散射路径损耗]
5. 叠加 FSPL(TX→散射点) + FSPL(散射点→RX)
```

粗糙面判据（Rayleigh 准则）：σ_h < λ/(8*cosθi) 为光滑面，跳过漫散射。

ElectricFieldNode type=5 字段：d_phi2_s_ar→Ar，d_phiE_s_s→S，r_t_s_d_thetai→θi。

<!-- CHAPTER5_END -->

---

## 第六章：电场响应总装 — 圆极化 3D 全路径积分

### 6.1 CalculateWaveImpactResponseDBmUnderCircularPolarization3D.cpp（325行）

整个 RT 系统将几何结果转化为电磁功率结果的最终函数，也是唯一对外暴露的高层电磁计算接口。

### 6.2 CalculateWaveImpactResponseDBm（单路径总损耗）

```
输入：ElectricFieldNodeList path[]（完整路径节点序列，TX→中间节点→RX）

步骤：
1. 自由空间路径损耗（FSPL）
   FSPL = sum(GetLossDbAttenuationInAir(每段距离))

2. 遍历中间节点，按 type 分发损耗计算：
   type=2（反射）→ CalculateReflectionWaveLoss    → loss_r
   type=3（透射）→ CalculateTransmissionWaveLoss   → loss_t
   type=4（绕射）→ CalculateDiffractionWaveLoss    → loss_d
   type=5（漫散）→ CalculateDiffuseScatteringWaveLoss → loss_s

3. totalLoss = FSPL + Σ(loss_r + loss_t + loss_d + loss_s)

输出：totalLoss（dBm，相对路径损耗）
```

### 6.3 CircularPolarization3D（极化分解与叠加）

```
将发射天线辐射方向分解为水平（h）和垂直（v）两个极化分量：
  E_h = 方位角方向单位向量
  E_v = 俯仰角方向单位向量

分别计算两条极化路径的响应：
  P_h = CalculateWaveImpactResponseDBm(path, polarization=h)
  P_v = CalculateWaveImpactResponseDBm(path, polarization=v)

功率叠加：P_total = AddDBm(P_h, P_v)
```

### 6.4 AddMultiPathResponse（多径叠加）

```
for 每条路径 path_i:
  P_i = CircularPolarization3D(path_i) + Pt + G_tx + G_rx

叠加模式（由 energyOutputMode 控制）：
  非相干（默认）：P_total = AddDBm(P1, P2, P3, ...)   [功率线性叠加]
  相干：先将各路径电场矢量复数累加，再取模方           [相位干涉]
```

### 6.5 CalElectricFieldComplexVector（极化感应计算）

按接收天线极化矢量计算有效接收功率：

```
有效长度矢量 L_eff = 天线极化矢量（方向图插值）
入射电场矢量 E_inc = 路径末段方向 × 极化方向
有效接收功率 ∝ |L_eff · E_inc|²
```

### 6.6 完整数据流

```
ElectricFieldNode 路径序列
    ↓
CalculateWaveImpactResponseDBm（串联各节点损耗）
    ↓
CircularPolarization3D（h/v 极化分解，分别积分，功率叠加）
    ↓
AddMultiPathResponse（所有路径非相干/相干叠加）
    ↓
总接收功率 P_rx（dBm）→ CSV/JSON 输出
```

### 6.7 依赖关系

```
→ CalculateReflectionWaveLoss / TransmissionWaveLoss / DiffractionWaveLoss / DiffuseScatteringWaveLoss
→ CalculateWaveLossCoefficientBase（FSPL、单位换算）
→ AntennaRadiationPattern3D（天线增益插值）
→ LxQPolarization3D（极化矢量）
→ ElectricFieldPath（路径节点容器）
```

<!-- CHAPTER6_END -->

---

## 第七章：SBR 射线生成与路径节点树

### 7.1 发射天线射线生成 — SbrRayGeneratedByTransmittingAntenna.cpp

SBR 第一步：从 TX 出发，向全空间均匀撒射线。

```
totalRays ≈ 10000（可配置）
方位角步长 dPhi   = gapSbrRayAzimuthAngle（弧度）
俯仰角步长 dTheta = gapSbrRayPitchAngle（弧度）

for theta in [-π/2, π/2] step dTheta:
  for phi in [0, 2π) step dPhi:
    dir = sphericalToCartesian(theta, phi)
    ray = SbrRay3D(origin=TX_pos, dir=dir, power=Pt_dBm)
    rayList.push_back(ray)
```

SbrRay3D 核心字段：
- origin：射线起点（TX 坐标）
- direction：单位方向向量
- power：当前功率（dBm，随弹射次数递减）
- pathNodeList：已经历节点列表（用于回溯完整路径）

均匀球面覆盖确保对所有方向的覆盖密度相等，是 SBR 算法精度的基础。

### 7.2 反射/透射子射线生成 — SbrRayGeneratedByReflection.cpp

SBR 递归核心：射线命中三角面后生成子射线。

**反射射线**：
```
n = triangle.normalVector
d = incident_ray.direction
r = d - 2*(d·n)*n          // Snell 镜面反射
r = normalize(r)
reflectedRay.origin    = hitPoint + eps*n   // 微小偏移防自相交
reflectedRay.direction = r
reflectedRay.power     = incident_ray.power + GetReflectionLoss(...)
```

**透射射线**：
```
若 realWorldRefraction=true:
  r_refract = Snell(d, n, n1, n2)  // 折射定律修正方向
else:
  r_refract = d                     // 沿原方向穿透（简化模式）

transmittedRay.origin = hitPoint - eps*n   // 穿透到三角面另一侧
```

两种模式对应两种精度/性能取舍：折射方向修正影响绝对时延计算，简化模式适用于薄壁场景。

### 7.3 多径节点信息容器 — LxQMultiPathNodeInfo.cpp

连接几何追踪结果与电磁计算的核心数据容器：

```
struct MultiPathNodeInfo {
  vector<ElectricFieldNode> pathNodes;  // TX→中间节点→RX 的完整节点序列
  double totalPathLength;               // 总路径长度（m）
  double excessDelay;                   // 相对时延（ns，相对最短路径）
  double totalLoss;                     // 总路径损耗（dBm，由第六章计算填入）
  int    rxIndex;                       // 对应接收天线索引
}
```

内存管理（FreeMultiPathNodeInfo_vector_all）：逐层释放 rxIndex → path_list → pathNodes，防止内存泄漏。

### 7.4 路径节点树 — DxQRayTracingGeometricPathNode.cpp

SBR 过程以树形结构记录所有弹射分叉：

```
struct DxQRayTracingGeometricPathNodeSIMO {
  Point3D   location;       // 节点三维坐标
  int       type;           // 1=反射, 2=透射, 3=绕射, 4=漫散射
  int       triangleIndex;  // 命中三角面/角落索引
  double    distance;       // 前一节点 → 本节点距离
  double    thetai;         // 入射角（rad）
  int       materialIndex1; // 入射侧材质
  int       materialIndex2; // 透射侧材质（仅透射/绕射节点有效）
  vector<DxQRayTracingGeometricPathNodeSIMO*> children; // 子节点（分叉）
}
```

**路径提取（ExtractPathFromRoot）**：
```
DFS 遍历路径节点树：
for root → children → ... → leaf:
  if leaf.next_distance_to_rx > 0:    // 该叶节点能到达某 RX
    提取完整链路 → 构建 ElectricFieldNode 序列
    → push_back 到 MultiPathNodeInfo_vector[rxIndex]
```

### 7.5 SBR 完整调度流（整合视图）

```
XdQRtSbr3D::SbrFindGeometricPathSIMO()
    ↓
SbrRayGeneratedByTransmittingAntenna  →  生成 ~10000 条初始射线
    ↓
for 每条射线（多线程并行）:
  Ray3DIntersectGeometry3D（BVH球体树粗筛 → AABB-BVH精确求交）
    ↓
  命中三角面 → 记录 DxQRayTracingGeometricPathNode
    ↓
  SbrRayGeneratedByReflection → 反射子射线
  SbrRayGeneratedByReflection → 透射子射线
  [递归，最大弹射次数 maxReflectionCount]
    ↓
  检查是否可见某 RX（最后一跳距离 > 0）
    ↓
ExtractPathFromRoot → MultiPathNodeInfo_vector[rxIndex]
    ↓
CalculateWaveImpactResponseDBmUnderCircularPolarization3D（第六章）
    ↓
P_rx[rxIndex]（dBm）
```

<!-- CHAPTER7_END -->

---

## 第八章：几何加速结构 — BVH 球体树 + AABB 包围盒

### 8.1 整体设计思路

系统采用双层加速结构，应对 10 万级三角面的大规模场景：

```
Layer 1 — BVH 球体树（BvhBall3D）
  用途：射线与三角面的粗筛（false positive 允许，false negative 禁止）
  特点：球体求交计算量极低，适合快速排除大量不可能命中的三角面

Layer 2 — AABB-BVH 树（BoundingBox3D）
  用途：精确确定射线命中的三角面及交点坐标
  特点：SAT（分离轴定理）AABB 特化，逻辑简单，数值稳定
```

两层配合：球体树快速缩小候选集 → AABB 树精确判断 → Möller-Trumbore 求精确交点。

### 8.2 BVH 球体树 — HdQBvhBall3D.cpp（240行）

#### 8.2.1 节点数据结构

```
BvhBall3D 节点：
  nodeInfo.ball3D     - 包围球（中心 Point3D + 半径 radius）
  nodeInfo.level      - 树层级（深度）
  leftNode_safe       - 左子节点指针
  rightNode_safe      - 右子节点指针
  triangleIdSet       - 本节点存储的三角面 ID 集合（构建期，可写）
  cornerIdSet         - 本节点存储的角点 ID 集合（绕射棱线）
  triangleIdSet_safe  - 线程安全只读副本（多线程查询期使用）
  cornerIdSet_safe    - 线程安全只读副本
```

#### 8.2.2 三角面插入策略

```
AddTriangleId(triId):
  if 三角面任一顶点在左子节点球内 → 递归插入左子
  elif 三角面任一顶点在右子节点球内 → 递归插入右子
  else → 插入当前节点（跨边界三角面放父节点，保守策略）
```

跨边界三角面放父节点是 BVH 建树的保守策略：宁愿多查也不漏查。

#### 8.2.3 非递归析构（防栈溢出）

```
~BvhBall3D():
  用 std::stack<BvhBall3D*> 模拟递归
  GetStack(p, stack): 压入 p，再压左/右子节点（并将子指针置 null 防二次释放）
  while(!stack.empty()): delete top; pop
```

深层树（>1000层）递归析构会触发栈溢出，显式栈是标准工程规避方案。

#### 8.2.4 多线程安全机制

```
UpdateSafeIdSetInfo()（主线程建树完成后调用一次）:
  迭代式（非递归）遍历整棵树：
  for 每个节点:
    cornerIdSet_safe.UpdateBySet(cornerIdSet)
    triangleIdSet_safe.UpdateBySet(triangleIdSet)

之后：多线程 SBR 子线程只读 *_safe 版本 → 无锁并发
```

### 8.3 AABB 包围盒 — HdQBoundingBox3D.cpp（67行）

```
BoundingBox3D:
  min: Point3D（x_min, y_min, z_min），初始化为 +∞
  max: Point3D（x_max, y_max, z_max），初始化为 -∞

IsPoint3DInBox(p):
  return p.x∈[min.x,max.x] && p.y∈[min.y,max.y] && p.z∈[min.z,max.z]

IsColliding(box2)（AABB-AABB SAT）:
  return min.x<=box2.max.x && max.x>=box2.min.x   // X 轴重叠
      && min.y<=box2.max.y && max.y>=box2.min.y   // Y 轴重叠
      && min.z<=box2.max.z && max.z>=box2.min.z   // Z 轴重叠
```

ReBuildByBox3D(box)：从 AABB 构造最小包围球，球心 = 包围盒中心，半径 = 对角线半长。两套加速结构通过此函数互转。

### 8.4 两套加速结构对比

```
属性            BVH 球体树             AABB-BVH 树
求交计算        |rayO - center|≤r      SAT（6次比较）
精度            粗筛（有误报）          精确
内存占用        中                     中
构建复杂度      中                     中
适用场景        任意方向射线            轴对齐方向更优
多线程安全      _safe 副本机制          标准只读并发
```

<!-- CHAPTER8_END -->

---

## 第九章：基础几何算法 — 三角面数据 + 相交检测

### 9.1 Triangle3D 数据类 — DxQTriangle3D.cpp（46行）

系统最底层的几何数据类，极简设计：

```
namespace Triangle3DStd {
  Triangle3D {
    Point3D p1, p2, p3;   // 三个顶点（x, y, z）
  }
  构造：默认 + 拷贝 + (p1, p2, p3) 三点构造
  析构：无特殊操作（POD 数据，无动态内存）
}
```

设计分离原则：Triangle3D 仅作纯几何容器，不存材质、法向量、粗糙度等物理属性。这些扩展属性在 DxQScenarioObject 的 ScenarioTriangle3D 类中定义，实现几何与物理属性的正交分离。

### 9.2 3D 相交检测算法集 — Geometry3DIntersect.cpp（599行）

RT 系统最核心的基础几何库，提供完整的射线-几何体相交算法族。

#### 9.2.1 函数命名体系

```
后缀规则：
  _base         — 返回最基础信息（距离/布尔值），无额外输出参数
  _plus_plus     — 额外输出参数（交点坐标 res, 距离 distance）
  _1 / _2        — 算法变体（不同 eps 策略或返回格式）
  _plus_plus_2   — 两者结合
```

#### 9.2.2 点与基本元素

```
Intersect_Point3D_Point3D     — 点点重合（坐标分量均相等）
Intersect_Point3D_Line3D      — 点在直线上（叉积 cross(p-lineO, lineVec) ≈ 0）
Intersect_Point3D_Ray3D       — 点在射线上（叉积≈0 且 dot(p-rayO, rayVec) > 0）
```

#### 9.2.3 直线与直线/线段

```
Intersect_Line3D_Line3D_plus_plus_base:
  用参数化形式求两直线最近距离和最近点对
  返回：distance, pointOnLine1, pointOnLine2

Intersect_Line3D_Line3D_plus_plus_2:
  distance <= eps → 返回中点作为交点

Intersect_Line3D_LineSegment3D_plus_plus_2:
  先求直线与线段所在直线的交点，再验证交点在线段范围内
```

#### 9.2.4 直线与平面（Intersect_Line3D_Plane3D）

```
设直线方向 lineVec, 平面法向 planeN, 平面过点 planeO, 直线过点 lineO
d1 = dot(lineVec, planeN)
if |d1| < eps: 平行，return false
t = dot(planeO - lineO, planeN) / d1
res = lineO + t*lineVec
```

用于计算三角面截线（Intersect_Triangle3D_Plane3D_plus_plus）：三条边分别与平面求交，取有效的两个点构成截线，用于场景切割。

#### 9.2.5 射线与三角面（核心，Möller-Trumbore 风格）

```
Intersect_Ray3D_Triangle3D_plus_plus(ray, tri, &res, &distance):
  e1 = p2-p1, e2 = p3-p1
  h  = cross(rayVec, e2)
  a  = dot(e1, h)
  if |a| < eps: 射线平行三角面，return false
  f  = 1/a
  s  = rayO - p1
  u  = f * dot(s, h)       // 重心坐标 u ∈ [0,1]
  if u < 0 or u > 1: return false
  q  = cross(s, e1)
  v  = f * dot(rayVec, q)  // 重心坐标 v ∈ [0,1-u]
  if v < 0 or u+v > 1: return false
  t  = f * dot(e2, q)      // 沿射线方向的参数距离
  if t > eps:
    res = rayO + t*rayVec
    distance = t
    return true
```

时间复杂度 O(1)，无需预计算三角面法向量，是光线追踪工业界的标准算法。

#### 9.2.6 AABB 与射线（slab 法）

```
Intersect_Ray3D_AABB(ray, box):
  对 x/y/z 三个轴分别计算射线进入(t_min)和离开(t_max)时间：
  tmin_axis = (box.min.x - rayO.x) / rayVec.x
  tmax_axis = (box.max.x - rayO.x) / rayVec.x
  tmin = max(tmin_x, tmin_y, tmin_z)
  tmax = min(tmax_x, tmax_y, tmax_z)
  return tmax >= tmin && tmax > 0
```

slab 法的几何直觉：射线"进入"所有轴的时刻中最迟的那个，必须早于"离开"所有轴最早的那个。

#### 9.2.7 依赖与被依赖关系

```
依赖：
→ Geometry3DOperateStd（叉积 cross、点积 dot、归一化 normalize、距离）
→ MathOperateStd（零值判断 OneNumberIsZeroByEps）
→ GlobalConstantStd::Eps（精度常量，约 1e-10）

被依赖：
← Ray3DIntersectGeometry3DElementsAabbBvhTree（BVH 遍历主函数）
← Solve1SPathByIm3D（镜像法可见性判断）
← SolveOneTimeDiffractionPathByEquation（绕射路径可见性）
← HdQBuildGeometryPathD（几何路径预处理）
```

<!-- CHAPTER9_END -->

---

## 第十章：系统设计总结与关键数据流

### 10.1 整体架构回顾

RT.XD.SBR.CGAL.25.05 是一个基于物理的 3D 射线追踪电磁仿真系统，核心支撑无线信道建模（信号覆盖、多径时延、MIMO 容量估算）。规模约 100 个 .cpp 文件，10 个功能模块。

系统分六层，自顶向下：

```
┌─────────────────────────────────────────────────┐
│  第一层  调度层     main.cpp / XdQRtSbr3D.cpp   │
│          任务分发、SISO/SIMO/MIMO 分支、并发控制 │
├─────────────────────────────────────────────────┤
│  第二层  场景预处理  HdQBuildGeometryPathD.cpp  │
│          BVH 建树、角点提取、几何路径预构建      │
├─────────────────────────────────────────────────┤
│  第三层  路径求解层                              │
│   SBR 法  SbrRayGeneratedBy*.cpp                │
│   镜像法  Solve1SPathByIm3D.cpp                 │
│   绕射法  SolveOneTimeDiffractionPathByEquation │
├─────────────────────────────────────────────────┤
│  第四层  电磁损耗层                              │
│   反射 CalculateReflectionWaveLoss              │
│   透射 CalculateTransmissionWaveLoss            │
│   绕射 CalculateDiffractionWaveLoss（UTD）      │
│   漫散 CalculateDiffuseScatteringWaveLoss       │
├─────────────────────────────────────────────────┤
│  第五层  响应总装    CircularPolarization3D      │
│          多径叠加（相干/非相干）、极化分解       │
├─────────────────────────────────────────────────┤
│  第六层  基础设施层                              │
│   几何  Geometry3DIntersect / Triangle3D        │
│   加速  BvhBall3D / BoundingBox3D               │
│   数学  Complex / CalculateWaveLossCoefficientBase │
└─────────────────────────────────────────────────┘
```

### 10.2 关键数据流（端到端）

```
config.json / 场景文件
    ↓ main.cpp 解析
HdQCommonParameterConfig（全局参数）
    ↓
XdQRtSbr3D 初始化
    ↓ HdQBuildGeometryPathD
BVH 球体树 + AABB-BVH 树（只建一次）
    ↓
并行 SBR 路径搜索（OpenMP / std::thread）
    ├─ SbrRayGeneratedByTransmittingAntenna（~10000 初始射线）
    ├─ Ray3DIntersectGeometry3D（BVH 双层加速求交）
    ├─ SbrRayGeneratedByReflection（反射/透射子射线，最大 maxReflectionCount 次）
    └─ DxQRayTracingGeometricPathNode 树（路径分叉记录）
    ↓ ExtractPathFromRoot（DFS）
MultiPathNodeInfo_vector[rxIndex]（全量多径集合）
    ↓ IM3D 和方程法绕射追加
Solve1SPathByIm3D（镜像法 1/2/3 次散射路径）
SolveOneTimeDiffractionPathByEquation（UTD 绕射路径）
    ↓ 合并所有路径
CalculateWaveImpactResponseDBmUnderCircularPolarization3D
    ├─ 各节点分发：Fresnel反射/透射 + UTD绕射 + 漫散射
    ├─ h/v 极化分解 → 功率叠加
    └─ 多径非相干/相干叠加
    ↓
P_rx[rxIndex]（dBm）→ 输出文件（CSV/JSON）
```

### 10.3 关键设计决策

| 决策 | 选型 | 理由 |
|---|---|---|
| 路径搜索主算法 | SBR（射线管） | 适合复杂室内/城市场景，覆盖所有弹射组合 |
| 绕射算法 | UTD（统一绕射理论） | 比 GO 更准，比 FDTD 快，工程精度可接受 |
| 漫散射模型 | Degli-Esposti 修正 Lambertian | 有方向集中因子 Ar，比纯 Lambertian 更准确 |
| 加速结构 | BVH 球体树 + AABB-BVH 双层 | 粗筛快速剔除 + 精确判定，避免暴力 O(N) 求交 |
| 多线程安全 | _safe 副本机制（无锁读） | 主线程建树，子线程只读 _safe 副本，零锁竞争 |
| 极化处理 | h/v 分量圆极化积分 | 正确处理任意极化天线的极化匹配损耗 |
| 多径叠加 | 非相干（默认）/ 相干（可选） | 非相干适合统计覆盖，相干用于精确干涉计算 |
| 无效路径标记 | MAX_PATH_LOSS = 327 dBm | 统一占位值，避免 NaN/Inf 污染后续统计 |

### 10.4 代码工程质量评估

优点：
- 算法层次清晰，每个物理效应对应独立 .cpp 文件，可维护性高
- 底层纯函数（无状态，线程安全），顶层状态集中管理
- 命名规范统一（D/H/L/X 前缀区分模块层次）
- 非递归 BVH 析构等工程细节处理到位

可改进点：
- 部分模块（SbrRayGeneratedBy*.cpp）框架层与算法层混合，单元测试困难
- 电磁参数（Eps、MAX_PATH_LOSS、maxReflectionCount）散布于多处，建议集中到 HdQCommonParameterConfig
- 漫散射粗糙面判据（Rayleigh 准则）硬编码，建议暴露为可配置参数

### 10.5 报告说明

本报告基于对项目源代码（100+ .cpp 文件，约 10 万行代码）的直接读取分析，分 10 批次完成，共生成 10 份批次分析文件和本整合报告。所有代码文件均未做任何修改。

分析时间：2026年4月
项目版本：RT.XD.SBR.CGAL.25.05
报告输出路径：E:\RT_claude\RT_claude\RTnew\text\md\RT算法代码全量分析报告.md

<!-- REPORT_END -->










