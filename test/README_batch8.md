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
2. `CoverageEMAggregate`
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
