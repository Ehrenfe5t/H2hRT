# 批次8验证说明

## 文件用途

本说明文档用于解释模块5批次8“多路径汇总与双模式求值”的当前验证方式。

---

## 当前批次8重点检查项

运行程序后，应在日志中检查：

1. `PreciseEMAggregate`
   - `paths`
   - `cir_taps`
   - `pdp_taps`
   - `aps_entries`
   - `total_power`
   - `mean_abs_phase`
   - `tx_paths`
2. `CoverageEMAggregate`
   - `avg_fs_loss_db`
3. `EMProfileCompare`
   - precise 与 coverage 摘要差异
4. 结束标记
   - `Batch8 module5 aggregate and dual-profile closed loop completed.`

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查批次7仍正常闭环；
3. 检查 PreciseEM / CoverageEM 两种模式是否都能生成 CIR / PDP / APS / Statistics / Coverage / ISAC 摘要；
4. 检查 profile 对比日志是否输出。

---

## 当前批次限制

1. 当前 CIR / PDP / APS / Coverage / ISAC 为第一版基础结果；
2. 当前双模式差异主要体现在 profile 结构与汇总策略摘要，不代表最终完整工程级差异实现已经全部收敛；
3. 结果导出与可视化仍属于后续批次范围。

---

## A4阶段补充关注点

进入 A4 后，批次8应额外核查以下 path-level 增强是否传导到 aggregate：

1. `CIR` 是否仍可稳定输出，并保留路径级复振幅真源；
2. `PDP` 是否仍可稳定输出，并反映更合理的 delay/power 分布；
3. `APS` 是否不再依赖明显失真的占位角指标；
4. `ChannelStatistics` 是否能体现：
   - `mean_abs_phase`
   - `transmission_path_count`
5. `CoverageResult` 是否能体现：
   - `average_free_space_loss_db`
6. `ISACFeatureSet` 是否能体现：
   - `average_polarization_magnitude`
   - `transmission_path_count`

---

## A7阶段补充关注点

进入 A7 后，批次8在验证资产体系中的角色应明确为：

1. **aggregate 趋势样例资产**；
2. 它主要用于检查：
   - path-level 增强是否仍稳定传导到 aggregate；
   - `mean_abs_phase / avg_fs_loss_db / tx_paths` 等趋势量是否还可追踪；
3. 批次8应更多承担“趋势稳定性”和“对比观察”角色，而不是最终 blocking 基线本体；
4. 若 A7 后某次迭代导致 aggregate 趋势明显异常，批次8应优先作为**aggregate 退化样例层**给出前置信号。

---

## A7资产分层定位

在 A7 的验证资产体系中，批次8的正式角色为：

1. **aggregate 趋势样例层**；
2. 负责说明 path-level 增强是否仍稳定传导到 aggregate；
3. 负责给主场景 regression 提供趋势对照线索；
4. 不作为 A7 最终 blocking baseline 本体，但可作为重要的趋势观察样例层。

---

## A8收口补充

当前批次8仅保留趋势样例职责，不再承担正式交付主视图；正式结果表达与交接请转向批次9及 A8 快速接手视图。
