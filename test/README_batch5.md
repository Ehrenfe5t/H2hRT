# 批次5验证说明

## 文件用途

本说明文档用于解释模块4批次5“核心结构与 SearchEngine 骨架”的当前验证方式。

---

## 当前批次5重点检查项

运行程序后，应在日志中检查：

1. `Batch5Search`
   - `succeeded`
   - `states`
   - `dedup_states`
   - `dedup_paths`
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
4. 检查 `SearchTrace` 中是否出现初始状态构造和 LOS 成立日志。

---

## 当前批次限制

1. 当前 SearchEngine 仅完成最小 LOS 闭环；
2. 反射 / 透射 / 绕射扩展器将在批次6继续接入；
3. 当前状态去重与路径去重为第一版签名框架，后续仍可增强量化与角度容差策略；
4. 当前 Tx/Rx 使用 `path_search.debug_tx_* / debug_rx_*` 作为批次5调试入口。
