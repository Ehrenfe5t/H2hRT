# 批次6验证说明

## 文件用途

本说明文档用于解释模块4批次6“Reflection / Transmission / Diffraction 扩展器”的当前验证方式。

---

## 当前批次6重点检查项

运行程序后，应在日志中检查：

1. `ReflectionSelfCheck`
   - `next_states`
   - `failure_reasons`
2. `TransmissionSelfCheck`
   - `next_states`
   - `failure_reasons`
3. `DiffractionSelfCheck`
   - `next_states`
   - `failure_reasons`
4. 汇总结果
   - `Batch6Expanders: reflection=true, transmission=true, diffraction=true`
5. 结束标记
   - `Batch6 module4 expanders closed loop completed.`

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查批次5仍正常闭环；
3. 检查三个扩展器自检是否都生成至少 1 个新状态；
4. 检查批次6汇总结果是否全部为 `true`。

---

## 当前批次限制

1. 当前批次6优先保证单次 Reflection / Transmission / Diffraction 可稳定扩展；
2. mixed path toy scene 当前主要通过 SearchEngine 主循环中多扩展器串联框架提供基础，不代表最终完整 mixed path 行为已完全收敛；
3. 更细的剪枝、介质链与绕射几何精修仍可在后续继续增强。
