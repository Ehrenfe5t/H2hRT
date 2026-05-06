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
4. 检查批次6汇总结果是否全部为 `true`；
5. A2 阶段额外观察：Reflection 是否明显多于 1 个候选状态；
6. A2 阶段额外观察：Diffraction 候选数是否被控制规则压缩，而不是一味膨胀。
7. 若 Reflection/Transmission/Diffraction 的 `next_states` 数量出现结构性差异，应结合 A2 文档理解为“机制级质量筛选开始生效”，而不是简单比较谁越多越好。

在 A3 阶段，还应额外关注：

8. `TransmissionSelfCheck` 是否在 transmission 主样例上保持 `next_states > 0` 且 `failure_reasons = 0`；
9. `TransmissionSelfCheckDetail` 是否仍能说明 transmission 候选已正式进入质量筛选与语义闭合链；
10. 对 transmission 主样例而言，`TransmissionSelfCheck` 的意义已从“有 transmission 候选”升级为“transmission 介质语义链在 search 层已可信”。
11. 当前 A3 transmission 主样例建议优先使用 `configs/app/a3_transmission_minimal.json`；若 reflection / diffraction 被禁用，旧自检链的对应机制被受控跳过是预期行为。

---

## 当前批次限制

1. 当前批次6优先保证单次 Reflection / Transmission / Diffraction 可稳定扩展；
2. A2 阶段中，扩展器自检已开始兼具“候选质量与控制效果”观察意义，不再只是单步可运行证明；
3. mixed path 第一阶段只允许 Reflection + Transmission 代表性组合，不代表更复杂 mixed path 已完全放开；
4. 更细的剪枝、介质链与绕射几何精修仍可在后续继续增强。
5. 当前若出现 Diffraction 候选被压缩、Reflection 候选更丰富，应视为 A2 机制深化的正常现象：更强调候选质量和可控性，而不是候选总数最大化。
6. 在 A3 transmission 主样例配置中，若 reflection / diffraction 被当前配置禁用，则对应旧自检链条受控跳过是预期行为，不应误判为 transmission 语义链失败。
7. 当前 A3 最小 transmission 主样例为了避免 scene cache 丢失新增 transmission 语义字段，建议使用 `enable_scene_cache=false`；若后续恢复 cache，则应先确认 transmission 语义字段已进入 cache 序列化链。
