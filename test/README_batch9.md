# 批次9验证说明

## 文件用途

本说明文档用于解释模块6批次9“结果表达、验证与回归闭环”的当前验证方式。

---

## 当前批次9重点检查项

运行程序后，应在日志中检查：

1. `Batch9Export`
   - `succeeded`
   - `exported_files`
   - `validation_passed`
   - `regression_blocking`
2. 结束标记
    - `Batch9 module6 export, validation and regression closed loop completed.`

---

## A8收口状态

1. `Batch8AggregateReporter`：保留为 aggregate 趋势样例层
2. `Batch9ExportReporter`：保留为主场景结果表达与交接入口
3. `A8_handoff_view.md`：作为后续接手的快速入口说明
4. `BatchX`：只保留接管关系说明，不再承载主实现

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查批次8仍正常闭环；
3. 检查 `output/batch9_exports/` 是否生成路径、信道、覆盖、通感、可视化和报告文件；
4. 检查日志中 `Batch9Export: succeeded=true`。

---

## 当前批次限制

1. 当前导出文件为第一版基础 schema；
2. 当前 Validation / Regression 为第一版摘要报告；
3. 更完整的可视化格式与基线比较仍可后续增强。

---

## A4阶段补充关注点

进入 A4 后，批次9主要用于确认“结果表达是否开始反映 path-level 求值增强”，应额外关注：

1. 导出的路径级结果是否包含 A4 新增解释字段：
   - `fs_loss_db`
   - `pol_mag`
   - `tx_semantic`
   - `medium_in / medium_out`
2. 输出的 aggregate 摘要是否反映：
   - `mean_abs_phase`
   - `avg_fs_loss_db`
   - `tx_paths`
3. validation / regression 仍应保持“摘要级闸门”定位，不在 A4 中升级为完整质量体系。

---

## A5阶段补充关注点

进入 A5 后，批次9主要用于确认“结果层是否已开始体现正式天线输入语义”：

1. 结果链至少应能说明当前主链使用的是正式 `AntennaModel` 输入，而不是仅靠默认硬编码发射源；
2. `Batch7` 中出现的：
   - `tx_antenna`
   - `tx_source`
   - `rx_antenna`
   - `rx_source`
   应被理解为 A5 已完成最小正式追溯链的重要证据；
3. 批次9当前不要求做完整天线结果导出 schema 扩张；
4. `pattern / polarization / orientation` 在 A5 阶段仍只属于正式扩展骨架，不要求批次9对其做完整结果表达。

---

## A7阶段补充关注点

进入 A7 后，批次9的定位应从“批次闭环验证器”进一步转向：

1. **主场景 validation / regression 过渡入口**；
2. 当前主场景建议优先仍采用：
   - `configs/app/a3_transmission_minimal.json`
   作为第一阶段 baseline 入口；
3. 批次9当前应重点观察：
   - `validation_failed_items`
   - `validation_failed_module`
   - `regression_diff_items`
   - `regression_warnings`
   - `regression_blockings`
   - `regression_blocking_module`
4. 在 A7 阶段，应明确区分三类资产角色：
   - 主场景 baseline：当前优先由 batch9 入口承接；
   - 机制样例：如 batch7；
   - aggregate 趋势样例：如 batch8；
5. A7 当前仍属于“最小正式守门能力”阶段，因此 batch9 虽已承接主 baseline 入口角色，但仍属于**过渡守门入口**，不等于最终完整 ValidationEngine / RegressionGate。

---

## A7资产分层定位

在 A7 的验证资产体系中，批次9的正式角色为：

1. **主场景 validation / regression 过渡入口**；
2. 负责承接主场景 baseline、validation summary、regression summary 的最小正式入口；
3. 负责把主链结果表达为后续完整守门体系可继续接管的格式；
4. 不等于最终完整 `ValidationEngine / RegressionGate`，但已经是其最重要的过渡入口之一。
