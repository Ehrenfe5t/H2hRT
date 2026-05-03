# 批次7验证说明

## 文件用途

本说明文档用于解释模块5批次7“基础物理求值主链”的当前验证方式。

---

## 当前批次7重点检查项

运行程序后，应在日志中检查：

1. `EMLosCheck`
   - `valid`
   - `delay`
   - `phase`
   - `power`
2. `EMReflectionCheck`
3. `EMTransmissionCheck`
4. `EMDiffractionCheck`
5. 结束标记
   - `Batch7 module5 EM main-chain closed loop completed.`

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查批次6仍正常闭环；
3. 检查 LOS / Reflection / Transmission / Diffraction 四类路径级 EM 结果是否都 `valid=true`；
4. 检查 delay / phase / power 均非空且无明显异常值。

---

## 当前批次限制

1. 当前模块5采用第一版简化自由空间段传播与交互系数；
2. Fresnel / UTD 严格模型将在后续继续增强；
3. 当前以路径级 `EMPathResult` 闭环为目标，CIR / PDP / APS 等汇总将在后续批次继续开发。
