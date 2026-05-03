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
