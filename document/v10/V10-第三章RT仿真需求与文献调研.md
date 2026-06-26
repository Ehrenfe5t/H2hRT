# V10 第三章 RT 仿真需求与文献调研

生成日期：2026-06-15  
适用范围：第三章“天线精细化建模与射线追踪算法优化研究”及后续 ISAC 信道仿真章节  
当前代码基线：V10，支持 `pipeline.precise_path_source = "SBR"` 的点对点路径源与 SBR 覆盖仿真

## 1. 研究定位重新收敛

根据当前论文目标和 V10 代码状态，建议将研究方向明确拆成两条主线。

第一条主线是“精细化天线建模与传统 RT 解耦再耦合”。其核心不是证明几何寻径算法能穷尽所有真实路径，而是证明传统 RT 中“标量增益 + 固定极化”的天线简化会改变可观测信道。RT 引擎只需提供几何上合理、物理交互类型明确、残差可审查的多径集合；创新重点放在每条路径到达 Tx/Rx 后，如何叠加方向图、Jones 极化、Ludwig-3 基矢、姿态旋转和接收端共轭匹配。

第二条主线是“面向 ISAC 的 RT 多径结果到指标映射”。其核心是把 RT 输出的 delay、AoD、AoA、complex amplitude、polarization、co/cross-pol、path geometry 等路径级信息，进一步构造成 PDP、CIR、APS、XPR/XPD、RMS delay spread、K 因子、覆盖功率、距离/角度分辨相关特征等 ISAC 可用指标。这里的重点是电磁链路和指标定义一致，而不是把所有可能几何路径全部找全。

因此，第三章的 RT 算法需求应表述为：

- 几何寻径需要“足够复杂、多样、可复核、物理一致”，不宜宣称“完备穷尽所有路径”。
- 电磁计算需要“相干、复矢量、极化、材质、天线姿态和方向图一致”，这是论文可信度的主支点。
- 输出需要“路径级全量数据 + 信道级统计数据 + 可视化审查数据”，支撑论文图表和误差分析。

## 2. 已整理文献的参考价值

### 2.1 双向信道与天线-信道解耦

本地文献：`paper/1-theory/3-antenna-coupling/The Double-Directional Radio Channel.pdf`

Steinbauer、Molisch、Bonek 提出的 double-directional radio channel 强调同时刻画发射端 DOD/AoD 和接收端 DOA/AoA，并指出可提取的多径参数应尽量独立于具体天线配置。这对本论文非常关键：第三章可以把“传播信道”和“天线可观测信道”分开叙述。RT 先给出天线无关的几何多径，再由 Tx/Rx 天线模型对每条路径进行方向图和极化加权。

对 V10 的启示：

- `GeometricPath` 应尽量保存天线无关的路径几何、交互节点、交互类型、残差和长度。
- `EMPathResult` 再保存天线作用后的复振幅、相位、功率、AoD/AoA、co/cross、XPR。
- 第三章应强调“天线模型作为路径后处理/电磁计算层的可替换模块”，而不是与几何搜索强耦合。

### 2.2 MEG、APS 与天线方向图耦合

本地文献：

- `Analysis_for_mean_effective_gain_of_mobile_antennas_in_land_mobile_radio_environments.pdf`
- `Angular_power_distribution_and_mean_effective_gain_of_mobile_antenna_in_different_propagation_environments.pdf`

MEG 文献的核心结论是：移动天线实际有效增益由天线方向图与环境入射角功率分布共同决定，不能只看天线标称峰值增益。角功率分布和 XPR/XPD 的测量文献也说明，不同传播环境下 elevation power distribution 和 cross-polarization power ratio 会显著变化。

对本论文的价值：

- 支撑 3.1 中“方向图、极化、姿态不是孤立属性，而是与 APS 共同决定可观测信道”的论述。
- 支撑 3.4 的实验设计：同一几何场景下切换 ideal/dipole/patch/horn 或改变姿态，比较 PDP、APS、XPR、总接收功率和 MEG-like 指标。
- 说明第三章需要真实 APS 输出，而不是单角度列表。

对 V10 的直接需求：

- 当前 `BuildAPS` 只使用 `aoa_theta_deg`，还不足以支撑二维 APS 和 APS 相似性分析；应扩展为 AoA/AoD theta-phi 网格。
- 需要导出每个角度 bin 的功率和归一化功率，便于画角谱热图。

### 2.3 Jones、Ludwig-3 与极化 RT-to-CIR

本地文献：`From_Ray_Tracing_to_Channel_Impulse_Responses_A_Review_on_the_Description_of_Polarimetric_Time-Invariant_SISO_Channels.pdf`

该综述明确把 Jones calculus、任意天线、RT 路径和 CIR 描述连接起来，重点讨论极化属性如何影响传统 CIR。在线元数据也说明，该工作聚焦 time-invariant SISO radio channel、arbitrary antennas、LoS 和多次反射，并指出 Tx/Rx 天线与环境极化共同影响 CIR 和定位性能。来源：Graz University of Technology 页面，https://tugraz.elsevierpure.com/en/publications/from-ray-tracing-to-channel-impulse-responses-a-review-on-the-des/

对本论文的价值：

- 这是第三章“Tx Jones 注入 -> 路径极化演化 -> Rx 共轭匹配 -> CIR”的直接理论支撑。
- 该文排除了绕射、散射和带宽限制，因此本论文可以自然定位为在其基础上面向工程 RT 引擎加入材质、透射、绕射和输出指标。
- 可作为“极化不是附加标量损耗，而是路径级复矢量传播”的关键依据。

对 V10 的直接需求：

- 每条路径必须保留复振幅和复极化状态，不能只导出功率。
- 反射、透射、绕射后必须保留 TE/TM 或等效极化基变换导致的相位差。
- Rx 端应使用复 Jones 响应计算接收电压，同时 co/cross 分解要与 Ludwig-3 或 Rx Jones 共/交定义一致。

### 2.4 极化 MIMO 与 XPR/XPD

本地文献：`Polarized_MIMO_channels_in_3-D_models_measurements_and_mutual_information.pdf`

该类文献说明交叉极化天线和 3D 极化信道会影响 MIMO mutual information、容量和信道相关性。虽然第三章可以不做完整 MIMO 容量分析，但极化路径权重、XPR/XPD 分布和极化相关损耗是有明确通信意义的。

对本论文的价值：

- 支撑 XPR/XPD 不只是天线指标，而是传播环境、路径交互和 Rx 投影共同作用的结果。
- 支撑“传统固定极化模型会低估/高估极化分集和交叉极化干扰”的论述。

对 V10 的直接需求：

- 路径级输出应包含 `co_pol_power_linear`、`cross_pol_power_linear`、`xpr_dB`。
- 聚合层应增加 XPR/XPD 分布统计，例如均值、中位数、分位数、功率加权均值。

### 2.5 RT 建模综述与小区/室内应用

本地文献：`Radio Science - 2015 - Fuschini - Ray tracing propagation modeling for future small-cell and indoor applications  A review.pdf`

Fuschini 等综述指出，RT 在小区和室内毫米波等短距离场景中重新变得重要，原因是高频下射线近似更合理、室内场景边界更可控，并可用于多维信道表征、MIMO 容量、实时 RT 等应用。

对本论文的价值：

- 支撑自研 RT 引擎用于室内通感一体场景的合理性。
- 支撑几何光学、UTD、材质反射/透射、路径级多维参数输出的工程价值。

对 V10 的直接需求：

- 必须提供可视化和残差审查，证明路径几何不是“黑箱采样结果”。
- 必须对 SBR 的采样收敛性进行实验，而不是声称一次仿真结果就是完备真实路径集。

### 2.6 高分辨参数估计与天线校准

本地文献：`Impact_of_Incomplete_and_Inaccurate_Data_Models_on_High_Resolution_Parameter_Estimation_in_Multidimensional_Channel_Sounding.pdf`

该文献强调高分辨多维信道估计依赖准确的数据模型；天线模型、测量设置和非分辨随机成分都会限制参数估计精度。

对本论文的价值：

- 支撑“天线模型不精细会导致 AoA/AoD、delay、complex polarimetric path weights 等参数偏差”的论点。
- 可用于解释为什么第三章要把天线方向图和极化响应接入 RT，而不是只做路径损耗。

对 V10 的直接需求：

- 路径级结果必须导出 AoA/AoD、delay、complex amplitude、polarization 和 residual，便于与测量或高分辨估计结果对比。
- 论文应承认未建模的 diffuse scattering、近场、阵列互耦等会成为残余误差来源。

### 2.7 XL-MIMO 与近场趋势

本地文献：`XL-MIMO channel measurement, characterization, and modeling for 6G a survey.pdf`

XL-MIMO 综述强调 6G 中近场效应和空间非平稳性是重要新问题。对本论文而言，它不一定是第三章必须实现的内容，但可以作为未来工作和 ISAC 扩展背景。

对 V10 的边界约束：

- 当前 V10 更适合远场或局部平面波路径近似。
- 若论文第三章讨论单天线或少量 Rx 点对点仿真，可以不实现 XL-MIMO 近场球面波和阵列互耦。
- 若后续章节扩展阵列角估计，需补阵列流形、元素方向图、相位中心和近场模型。

## 3. 最新研究现状

### 3.1 标准化趋势：3GPP Rel-19 已引入 ISAC 信道建模

3GPP TR 38.901 是 0.5-100 GHz 信道模型技术报告，3GPP 官网显示 Release 19 版本持续更新，19.0.0 于 2025-07-04 上传，19.2.0 于 2026-01-15 上传。来源：https://www.3gpp.org/dynareport/38901.htm

3GPP 变更请求页面显示，TR 38.901 存在 “Rel-19 CR to introduce channel model for ISAC”，状态为 agreed/approved，对应 FS_Sensing_NR。来源：https://portal.3gpp.org/ChangeRequests.aspx?q=1&specnumber=38.901

这说明 ISAC channel model 已经从学术概念进入标准化流程。对本论文的意义是：第三章和后续章节应把 RT 输出的多径参数组织成可被通信与感知共同使用的 channel representation，而不是只输出覆盖功率。

### 3.2 产业/联盟趋势：7-24 GHz 与 JCAS/ISAC 测量建模正在补充 3GPP

Next G Alliance 2024 报告页面说明，其报告研究 communication channels 和 JCAS/ISAC channel models，覆盖 midbands、mmWave、sub-THz，并考虑基于新测量对 3GPP TR 38.901 的 7-24 GHz 模型进行修正。来源：https://nextgalliance.org/white_papers/channel-measurements-and-modeling-for-joint-integrated-communication-and-sensing-as-well-as-7-24-ghz-communication/

Next G Alliance Phase II 页面进一步指出，TR 38.901 对 7-24 GHz 总体适用，但观测到一些偏差，并涉及 InF、InH、penetration loss 等测量。来源：https://nextgalliance.org/white_papers/channel-measurements-and-modeling-for-joint-integrated-communication-and-sensing-as-well-as-7-24-ghz-communication-channels-phase-ii/

这对论文的启示：

- 室内场景、穿透损耗、材质参数、角度/时延/极化输出是当前研究热点。
- 第三章不必追求大规模统计模型完整替代 3GPP，而应强调 deterministic RT 为 ISAC 生成路径级“可解释样本”。

### 3.3 商用 RT 工具趋势：完整 3D 天线方向图、相位和极化是成熟功能

Remcom Wireless InSite 的天线建模页面说明，其可导入来自测量、全波仿真等来源的天线方向图，支持完整三维 magnitude、phase、polarization，并可包含多频点、MIMO 天线位置和互耦 S 参数。来源：https://www.remcom.com/wireless-insite-em-propagation-software/antenna-modeling

MathWorks ray tracing 文档说明，反射和绕射损耗计算会跟踪水平/垂直极化，环境可改变信号极化并造成损耗。来源：https://www.mathworks.com/help/comm/ug/ray-tracing-for-wireless-communications.html

这说明“RT 中追踪极化和天线方向图”不是过度设计，而是成熟工具的重要能力。论文创新点不应写成“首次考虑天线方向图”，而应写成：

- 在自研零依赖 RT 引擎中，实现可审查的 Ludwig-3 Jones 天线方向图、姿态和路径级复矢量电磁链路。
- 针对论文场景，比较简化天线模型与精细化天线模型对 PDP、APS、XPR、覆盖和 ISAC 特征的影响。

### 3.4 学术热点：RT-to-CIR、极化、数字孪生和可微/神经 RT

近年文献热点包括：

- RT 到 CIR 的极化链路建模：强调路径级极化和接收端 feed-point 电压。
- 面向数字孪生的 RT 加速：关注场景预处理、几何简化、路径重用和高效生成。
- ISAC imaging / CSI-based sensing：利用 RT 或校准 RT 的路径分量生成环境感知特征。
- 神经 RT / surrogate channel model：用学习方法加速 RT 或拟合时空角信道。

对本论文最有用的切入点是前两项：V10 已经具备 BVH、SBR、路径导出、电磁链路和天线方向图接口，适合围绕“高可信路径级信道样本生成”展开。

## 4. 对第三章的 RT 仿真需求定义

### 4.1 几何寻径需求

第三章不需要证明 RT 能找全所有路径，但必须满足以下最低要求：

1. 路径物理交互类型明确：LoS、reflection、transmission、diffraction 至少能区分并导出。
2. 路径几何可审查：每条路径需要输出 Tx、交互节点、Rx 坐标、segment length、总长度、AoD/AoA。
3. 路径残差可审查：反射残差、Snell 残差、Keller 残差、总 geometry residual 应导出。
4. 路径复杂度可配置：最大深度、反射/透射/绕射次数、ray count、tube radius、Rx 捕获半径应可配置。
5. 路径多样性可验证：需要至少包含 LoS、单反射、多反射、透射、绕射或绕射+反射混合路径中的若干类。
6. SBR 采样需做收敛性实验：至少比较 3 组 ray count / ray tube 参数下路径数、总功率、PDP 主峰、XPR 均值是否趋于稳定。

建议论文表述：

> 本文不以穷尽所有几何可行路径为目标，而以生成经物理交互约束和残差筛选的高可信路径集合为基础，重点研究精细化天线模型对可观测信道参数的影响。

### 4.2 电磁计算需求

这是第三章最关键的仿真需求。

1. Tx 注入必须支持：
   - 发射功率和初始相位；
   - Tx 方向图 `G_tx(theta, phi)`；
   - Tx Ludwig-3/Jones 极化 `E_theta, E_phi`；
   - Tx 姿态 `forward/up/right` 或等效欧拉角；
   - 世界坐标与天线局部坐标转换。

2. 路径传播必须支持：
   - 自由空间相位 `exp(-j k R)`；
   - Friis/球面扩散幅度衰减；
   - 材质介质损耗；
   - Fresnel 反射 TE/TM 复系数；
   - Fresnel 透射 TE/TM 复系数与 TIR 处理；
   - UTD 绕射系数和极化基变换；
   - 相干叠加所需的复振幅保留。

3. Rx 接收必须支持：
   - Rx 方向图 `G_rx(theta, phi)`；
   - Rx Jones 极化响应；
   - Rx 姿态；
   - 复共轭匹配得到接收电压/路径复幅度；
   - co/cross-pol 功率和 XPR。

4. 数值稳定性必须支持：
   - 小距离、小角度、掠入射保护；
   - NaN/Inf 过滤；
   - 极低功率路径裁剪；
   - dB/linear 转换一致；
   - 相干求和避免重复相位叠加。

### 4.3 天线模型需求

第三章建议采用“单频、远场、路径级 Jones 天线模型”为主，不强制实现宽带天线时延色散。

必须支持：

- gain CSV 加载和插值；
- Jones polarization CSV 加载和插值；
- ideal/dipole/patch/horn 至少四类模型或 CSV；
- Tx/Rx 独立天线配置；
- 多 Rx 每个 Rx 的位置、姿态和天线模型覆盖；
- 姿态奇异性校验，例如 forward 与 up 平行时拒绝配置。

建议暂不作为第三章 P0：

- 阵列互耦 S 参数；
- 近场球面波；
- HFSS 多频点群时延建模；
- 完整 PSM 矩阵级 Tx/Rx 多端口模型。

这些可以作为未来工作或第五章扩展，除非论文后续必须强调“宽带时延-极化-旋转”三属性。

### 4.4 信道指标输出需求

第三章至少需要输出：

- 路径表：delay、length、phase、complex amplitude、power、AoD/AoA、interaction sequence、co/cross、XPR、antenna id/source、geometry residual。
- CIR：复 tap，支持相干叠加。
- PDP：功率 tap，支持 delay bin。
- APS：二维 AoA/AoD theta-phi 功率谱。
- XPR/XPD：路径级和统计级。
- 大尺度统计：总接收功率、路径损耗、K 因子、RMS delay spread、有效路径数。
- 覆盖模式：虚拟 Rx 接收功率，允许非相干叠加。

ISAC 相关建议输出：

- sensing delay/range profile：由 CIR/PDP 延迟轴映射到距离分辨；
- angle profile：由 APS 映射到角度分布；
- dominant scatterer/path list：强路径的反射/绕射/透射节点；
- polarization feature：XPR/XPD 均值、分位数、极化保持/去极化程度；
- clutter-like feature：非 LoS 多径总功率、强散射路径数、时延扩展；
- 可选通信指标：基于 SNR 的 Shannon capacity proxy，不建议在第三章深入 BER。

## 5. V10 当前支撑度与缺口

### 5.1 已满足

- V10 已支持点对点 precise 使用 SBR 作为主寻径源：`configs/app/v10_sbr_ptp_template.json` 中 `pipeline.precise_path_source = "SBR"`。
- Tx 端已有方向图、Jones 极化、姿态变换、局部角查询和增益加权链路。
- Rx 端已有方向图、Jones 极化、姿态变换、接收投影和 XPR 输出。
- 路径导出已经包含 AoD/AoA、co/cross、XPR、geometry residual 等关键字段。
- `configs/antennas` 中已有 dipole/patch/horn 的 gain/Jones CSV，可作为第三章模型库雏形。
- SBR coverage 和 point-to-point 模式已可分离配置。

### 5.2 必须补齐的 P0

1. Tx/Rx 独立天线配置：当前全局单 `antenna` 块会导致 Tx 和 Rx 无法分别设置模型和姿态。
2. 多 Rx 独立天线配置：`rx_list` 当前只有 id 和坐标，不支持每个 Rx 的天线文件和姿态。
3. APS 二维化：当前 APS 只有单角度 metric，无法支撑二维角谱图和 APS 相似性。
4. co/cross-pol 数学口径修正：需要明确使用 Ludwig-3 theta/phi 分解，或使用 Rx Jones 共极化方向及其复正交方向分解。
5. SBR 收敛性和路径审查实验：需要证明路径集合随 ray count / tube radius 不出现不可控漂移。

### 5.3 建议补齐的 P1

1. XPR/XPD 统计导出：均值、中位数、分位数、功率加权均值。
2. MEG-like 指标：用 APS 与天线方向图积分或离散求和，展示姿态影响。
3. ISAC feature set 扩展：从基础 path_count/earliest_delay 扩展到 range profile、angle profile、dominant paths、polarization features。
4. 宽带 CFR 口径统一：若继续使用 fixed_gain approximation，论文必须明确它不是逐频点 EM 重评估。
5. 配置模板整理：形成 `ch3_ptp_antenna_compare.json`、`ch3_pose_sweep.json`、`ch3_xpr_compare.json`、`ch3_coverage_power.json`。

### 5.4 可暂缓的 P2

- 近场 XL-MIMO；
- 阵列互耦；
- 完整 PSM 多端口；
- diffuse scattering；
- 实测 VNA 自动校准闭环；
- BER/完整链路级通信仿真。

## 6. 第三章建议实验矩阵

### 实验 E1：天线模型对路径可观测功率的影响

目的：验证 ideal、dipole、patch、horn 不同方向图导致相同几何路径的接收功率、PDP、APS、XPR 改变。

输入：

- 固定场景；
- 固定 Tx/Rx；
- 固定 SBR 路径参数；
- 切换天线 gain/Jones CSV。

输出：

- `precise_paths.json/csv`
- CIR/PDP
- APS theta-phi heatmap
- XPR distribution
- total received power

### 实验 E2：Rx 姿态敏感性

目的：验证 Rx forward/up 变化对可观测信道的影响。

输入：

- Tx 天线固定；
- Rx 位置固定；
- Rx yaw/pitch/roll 或 forward/up sweep。

输出：

- received power vs pose；
- XPR vs pose；
- dominant path power vs pose；
- APS center shift。

### 实验 E3：极化模型对 XPR/PDP/CIR 的影响

目的：比较固定极化与逐角度 Jones 极化的差异。

输入：

- 同一 gain pattern；
- 固定 polarization vs Jones polarization CSV。

输出：

- co/cross power；
- XPR CDF；
- CIR 幅相差异；
- PDP 主峰/旁峰变化。

### 实验 E4：V10 SBR 几何路径收敛性

目的：证明 V10 SBR 路径集合足以支撑第三章电磁分析。

输入：

- ray_count = 2k / 10k / 50k 或 Debug/Release 对应可承受规模；
- ray_tube_radius_scale sweep；
- diffraction_rays_per_event sweep。

输出：

- paths_after_postprocess；
- EM valid path count；
- total received power；
- RMS delay spread；
- XPR mean；
- max/mean geometry residual。

论文结论应写为“在选定参数后，主要信道指标进入稳定区间”，而不是“路径已穷尽”。

### 实验 E5：ISAC 路径特征输出

目的：把 RT 结果映射到感知可用特征。

输入：

- 包含强反射体/拐角/透射墙体的室内场景；
- 精细天线与 ideal 天线对比。

输出：

- range profile；
- angle profile；
- dominant reflector path list；
- clutter/multipath power ratio；
- XPR/XPD feature；
- coverage received power map。

## 7. 论文写作口径建议

建议第三章的创新表述：

1. 提出一种与几何 RT 解耦的天线精细化响应模型，在 Ludwig-3 框架下统一描述方向图、Jones 极化和三维姿态。
2. 将该模型接入自研 RT 的路径级复矢量电磁计算链路，实现 Tx 角度相关注入、传播交互极化演化和 Rx 共轭匹配。
3. 通过 V10 SBR 生成具备多样交互机制的可信几何路径集合，并以残差、可视化和收敛性实验保证路径可用性。
4. 从路径级复电场构建 CIR、PDP、APS、XPR 等可观测信道指标，分析精细天线模型相对简化天线模型的偏差。
5. 面向 ISAC，进一步将时延、角度、极化和强路径结构映射到 range/angle/polarization/clutter-like 特征。

建议避免的强口径：

- “精确穷尽所有物理路径”。
- “完整宽带天线时延色散建模”，除非实现 `frequency_sweep_em`。
- “完整 PSM 全极化散射矩阵”，除非补多端口矩阵模型。
- “完整通信容量/BER/感知精度闭环验证”，除非补系统级模块和实测/标定数据。

## 8. 下一步开发优先级

1. 拆分 `tx_antenna`、`rx_antenna` 和 per-Rx antenna override。
2. 重构 APS 为 AoA/AoD 二维功率谱并导出 heatmap 数据。
3. 修正或明确 co/cross-pol 分解口径。
4. 增加 XPR/XPD 和 MEG-like 统计导出。
5. 增加 SBR 收敛性实验脚本和输出模板。
6. 扩展 ISAC feature set：range profile、angle profile、dominant paths、polarization features。

完成以上 1-4 后，第三章可进入较稳的“论文可写状态”。完成 5-6 后，第四章/ISAC 分析会更有支撑。

## 9. 参考来源

本地文献：

- `paper/1-theory/3-antenna-coupling/The Double-Directional Radio Channel.pdf`
- `paper/1-theory/3-antenna-coupling/Analysis_for_mean_effective_gain_of_mobile_antennas_in_land_mobile_radio_environments.pdf`
- `paper/1-theory/3-antenna-coupling/Angular_power_distribution_and_mean_effective_gain_of_mobile_antenna_in_different_propagation_environments.pdf`
- `paper/1-theory/3-antenna-coupling/From_Ray_Tracing_to_Channel_Impulse_Responses_A_Review_on_the_Description_of_Polarimetric_Time-Invariant_SISO_Channels.pdf`
- `paper/1-theory/3-antenna-coupling/Polarized_MIMO_channels_in_3-D_models_measurements_and_mutual_information.pdf`
- `paper/1-theory/3-antenna-coupling/Radio Science - 2015 - Fuschini - Ray tracing propagation modeling for future small‐cell and indoor applications  A review.pdf`
- `paper/1-theory/3-antenna-coupling/Impact_of_Incomplete_and_Inaccurate_Data_Models_on_High_Resolution_Parameter_Estimation_in_Multidimensional_Channel_Sounding.pdf`
- `paper/1-theory/3-antenna-coupling/XL-MIMO channel measurement, characterization, and modeling for 6G a survey.pdf`

在线来源：

- Graz University of Technology, “From Ray Tracing to Channel Impulse Responses: A Review on the Description of Polarimetric Time-Invariant SISO Channels”, https://tugraz.elsevierpure.com/en/publications/from-ray-tracing-to-channel-impulse-responses-a-review-on-the-des/
- 3GPP TR 38.901 specification page, https://www.3gpp.org/dynareport/38901.htm
- 3GPP TR 38.901 change requests, https://portal.3gpp.org/ChangeRequests.aspx?q=1&specnumber=38.901
- Next G Alliance, “Channel Measurements and Modeling for Joint/Integrated Communication and Sensing, as well as 7-24 GHz Communication”, https://nextgalliance.org/white_papers/channel-measurements-and-modeling-for-joint-integrated-communication-and-sensing-as-well-as-7-24-ghz-communication/
- Next G Alliance Phase II, https://nextgalliance.org/white_papers/channel-measurements-and-modeling-for-joint-integrated-communication-and-sensing-as-well-as-7-24-ghz-communication-channels-phase-ii/
- MathWorks, “Ray Tracing for Wireless Communications”, https://www.mathworks.com/help/comm/ug/ray-tracing-for-wireless-communications.html
- Remcom Wireless InSite Antenna Modeling, https://www.remcom.com/wireless-insite-em-propagation-software/antenna-modeling
