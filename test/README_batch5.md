# 批次5验证说明

## 文件用途

本说明文档用于解释模块4批次5“核心结构与 SearchEngine 骨架”的当前验证方式。

---

## 当前批次5重点检查项

运行程序后，应在日志中检查：

1. `Batch5Search`
    - `succeeded`
    - `states`
    - `candidate_states`
    - `accepted_states`
    - `truncated_candidates`
    - `dedup_states`
    - `dedup_paths`
    - `control_rejected`
    - `invalid_sequence_rejected`
    - `mixed_path_blocked`
    - `mixed_path_generated`
    - `paths`
2. `SearchTrace`
   - 是否出现 `Initial PathState constructed.`
   - 是否出现 `LOS path established from Tx to Rx.`
3. 结束标记
   - `Batch5 module4 SearchEngine skeleton closed loop completed.`

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查 Batch4 仍正常闭环；
3. 检查 Batch5 是否输出至少 1 条 LOS 路径；
4. 检查 `SearchTrace` 中是否出现初始状态构造和 LOS 成立日志；
5. A2 阶段额外检查：是否出现 `candidate_states / accepted_states / truncated_candidates`；
6. A2 mixed path 阶段额外检查：`mixed_path_generated` 是否开始大于 0。
7. 若 `paths` 没有显著无限膨胀，但 `candidate_states / accepted_states / truncated_candidates / invalid_sequence_rejected` 已出现稳定变化，应理解为：A2 的目标是“更正式、更可控”，而不是单纯放大结果数量。

---

## 当前批次限制

1. 当前 SearchEngine 已进入 A2 强化阶段，不再只是最小 LOS 骨架；
2. 当前日志中的 `candidate_states / accepted_states / truncated_candidates` 用于观察多候选调度是否生效；
3. 当前 `control_rejected / invalid_sequence_rejected / mixed_path_blocked / mixed_path_generated` 用于观察控制层与 mixed path 能力是否开始工作；
4. 当前 Tx/Rx 仍使用 `path_search.debug_tx_* / debug_rx_*` 作为调试入口。
5. 当前 A2 mixed path 第一阶段通过标准不是“路径数越多越好”，而是至少出现一种受控的 Reflection + Transmission 正式样例，并保持下游主链可消费。
