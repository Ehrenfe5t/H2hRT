# ISAC（通感一体化）文献调研

> 调研日期：2026-06-01  
> 状态：初步完成（后续持续补充下载论文PDF）

---

## 1. ISAC 概念与 6G 动机

### 1.1 核心综述

| 文献 | 来源 | 年份 | 关键贡献 |
|------|------|------|---------|
| Liu, F. et al., "Integrated Sensing and Communications: Toward Dual-Functional Wireless Networks for 6G and Beyond", *IEEE JSAC*, vol. 40, no. 6, pp. 1728-1767, 2022. | IEEE JSAC | 2022 | **ISAC 领域最权威综述**。定义 ISAC 三大范式（雷达-通信共存、协作、联合设计），覆盖波形设计、波束赋形、信号处理。DOI: 10.1109/JSAC.2022.3156632 |
| Zhang, J.A. et al., "An Overview of Signal Processing Techniques for Joint Communication and Radar Sensing", *IEEE JSTSP*, vol. 15, no. 6, pp. 1295-1315, 2021. | IEEE JSTSP | 2021 | 通感联合信号处理综述，覆盖 OFDM、FMCW、OTFS 等波形。 |
| Wild, T. et al., "Joint Design of Communication and Sensing for Beyond 5G and 6G Systems", *IEEE Access*, vol. 9, pp. 30845-30865, 2021. | IEEE Access | 2021 | 面向 B5G/6G 的通感联合设计框架。 |

### 1.2 3GPP ISAC 标准化进展

| 文献 | 来源 | 年份 |
|------|------|------|
| 3GPP TR 22.837, "Feasibility Study on Integrated Sensing and Communication", v18.2.0, 2023. | 3GPP | 2023 |
| 3GPP TR 38.877, "Study on Channel Modelling for Integrated Sensing and Communication", v18.0.0, 2024. | 3GPP | 2024 |
| 3GPP RP-232678, "New SID on Integrated Sensing and Communication for NR", 2023. | 3GPP RAN#102 | 2023 |

> 3GPP Release 19 正式启动 ISAC Study Item，TR 38.877 专门面向 ISAC 信道建模。

---

## 2. ISAC 信道建模

### 2.1 通感共享信道

| 文献 | 关键内容 |
|------|---------|
| He, R. et al., "A Novel 3D Non-Stationary GBSM for 6G THz Ultra-Massive MIMO Wireless Systems", *IEEE TAP*, 2022. | 提出面向 6G 的 3D 非平稳 GBSM 信道模型，涵盖感知参数 |
| Huang, J. et al., "Multi-Frequency Multi-Scenario Millimeter Wave MIMO Channel Measurements and Modeling for B5G Wireless Communication Systems", *IEEE JSAC*, 2020. | 多频段多场景 mmWave 信道实测与建模 |
| Liu, L. et al., "The COST 2100 MIMO Channel Model", *IEEE Wireless Communications*, 2012. | COST 2100 经典 MIMO 信道模型（含双方向参数） |

### 2.2 RT 在 ISAC 中的应用

| 文献 | 关键内容 |
|------|---------|
| Sionna RT — Hoydis, J. et al., "Sionna RT: Differentiable Ray Tracing for Radio Propagation Modeling", *IEEE TAP*, 2023. | 开源可微 RT，支持路径、时延、角度、极化参数提取 |
| Degli-Esposti, V. et al., "Ray-Tracing-Based mm-Wave Beamforming Assessment", *IEEE Access*, 2014. | RT 用于 mmWave 波束赋形和信道参数提取 |
| Wireless InSite — Remcom Inc. 商用软件 | 工业级 RT，支持 MIMO、极化、材料库 |

---

## 3. 波形设计与信号处理

### 3.1 关键文献

| 文献 | 年份 | 关键贡献 |
|------|------|---------|
| Sturm, C. and Wiesbeck, W., "Waveform Design and Signal Processing Aspects for Fusion of Wireless Communications and Radar Sensing", *Proc. IEEE*, 2011. | OFDM 雷达+通信融合的开创性工作 |
| Braun, M. et al., "On the Performance of OFDM Radar and Communication Signals in Time-Varying Channels", *IEEE TSP*, 2018. | OFDM 时变信道下通感性能分析 |
| Yuan, W. et al., "Orthogonal Time Frequency Space Modulation: A Promising Next-Generation Waveform", *IEEE Wireless Com.*, 2021. | OTFS —— 面向高移动性 ISAC 的新波形 |

---

## 4. 性能指标与 Tradeoff

### 4.1 经典框架

| 文献 | 关键内容 |
|------|---------|
| Chiriyath, A.R. et al., "Radar-Communications Convergence: Coexistence, Cooperation, and Co-Design", *IEEE TCCN*, 2017. | 雷达-通信收敛三阶段框架 |
| Li, B. and Petropulu, A.P., "Joint Transmit Designs for Coexistence of MIMO Wireless Communications and Sparse Sensing Radars in Clutter", *IEEE TSP*, 2017. | MIMO 通信与雷达共存设计 |
| Kumari, P. et al., "IEEE 802.11ad-Based Radar: An Approach to Joint Vehicular Communication-Radar System", *IEEE TVT*, 2018. | 基于 802.11ad 的车载通感联合 |

### 4.2 CRLB 与信息论边界

| 文献 | 关键内容 |
|------|---------|
| Guerci, J.R., "Cognitive Radar: The Knowledge-Aided Fully Adaptive Approach", Artech House, 2010. | 认知雷达经典专著 |
| Sadler, B.M. and Swami, A., "Communications and Radar: Ways to Coexist and Cooperate", *IEEE ComMag*, 2021. | 通信-雷达共存合作综述 |

---

## 5. 中国研究团队

### 5.1 主要高校与团队

| 单位 | 代表人物 | 方向 |
|------|---------|------|
| 北京邮电大学 | 张平院士团队 | 6G ISAC 体系架构、信道建模 |
| 东南大学 | 崔铁军院士、金石教授 | RIS/智能超表面 ISAC |
| 电子科技大学 | 梁应敞教授 | 通感一体化信号处理、认知无线电 |
| 西安电子科技大学 | 李赞教授、刘彦明教授 | 雷达通信一体化、ISAC 波形设计 |
| 清华大学 | 戴琼海院士、王劲涛教授 | 太赫兹通感一体化 |

### 5.2 已调研的博士论文

| 论文 | 单位 | 年份 | 关键贡献 |
|------|------|------|---------|
| 待补充：知网检索 "通感一体化 信道建模 博士论文" | — | 2020-2025 | — |

---

## 6. 最新 arXiv 前沿（2024-2026，已下载）

| arXiv ID | 标题 | 年份 | 关键贡献 |
|----------|------|------|---------|
| 2507.19266 | Overview of 3GPP Release 19 Study on Channel Modeling Enhancements to TR 38.901 for 6G | 2025 | **3GPP R19 信道建模增强综述** — 必读 |
| 2602.07623 | A Tutorial on 3GPP Rel-19 Channel Modeling for 6G FR3 (7-24 GHz) | 2026 | 3GPP R19 FR3 信道建模教程 |
| 2412.07336 | New Characteristics and Modeling of 6G Channels: Toward a Unified Channel Model | 2024 | 6G 统一信道模型 |
| 2603.28736 | Deterministic Modeling of Dynamic ISAC Channels in RF Digital Twin Environments | 2026 | **ISAC RT 确定性信道建模** |
| 2505.10275 | ISAC Channel Modelling -- Perspectives from ETSI | 2025 | ETSI ISAC 信道建模视角 |
| 2509.06672 | ISAC Imaging by Channel State Information using Ray Tracing | 2025 | RT 用于 ISAC 成像 |
| 2507.06588 | Deep Learning-based Human Gesture Channel Modeling for ISAC | 2025 | ISAC 人体姿态信道建模 |
| 2604.05991 | Ray-Based Simulation of Scattering from Discretized Curved Bodies for ISAC | 2026 | ISAC 散射 RT 仿真 |
| 2411.04419 | Joint Antenna Positioning and Beamforming in Movable Antenna ISAC | 2024 | 可移动天线 ISAC |
| 2509.08652 | Rotatable Array-Aided Hybrid Beamforming for ISAC | 2025 | 旋转阵列 ISAC 波束 |
| 2507.08716 | Multimodal ISAC Data Simulation with Only One Engine (Unreal) | 2025 | ISAC 仿真数据生成 |
| 2509.21118 | Neural ISAC for the MIMO-OFDM Downlink | 2025 | 神经 ISAC MIMO-OFDM |
| 2509.06270 | UrbanMIMOMap: Ray-Traced MIMO CSI Dataset | 2025 | RT MIMO CSI 数据集 |

## 7. 对本论文的启示

1. **ISAC 信道建模的核心缺口**：现有 3GPP TR 38.901 不支持感知信号建模，TR 38.877 仍在制定中。本论文的 RT 引擎可填补"高分辨率通感共享信道建模"缺口
2. **天线联合建模的必要性**：ISAC 对天线极化、方向图、姿态的依赖远超纯通信场景——这正是本论文主线 B 的核心贡献
3. **验证方法**：ISAC 验证可参照 Sionna RT 的对照方案，同时增加感知指标（角度分辨率、距离分辨率）
4. **论文定位**：聚焦"面向 ISAC 的高分辨率室内 RT 信道建模"，以天线极化-时延-角度联合建模为核心创新点

---

> 注：本调研为初步框架，后续通过 deep-research workflow 持续补充具体 DOI 和下载 PDF。  
> 下次更新：补充 CNKI 博士论文检索结果和 3GPP 标准原文摘录。
