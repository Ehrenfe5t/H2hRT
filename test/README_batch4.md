# 批次4验证说明

## 文件用途

本说明文档用于解释模块2批次4“查询门面与缓存闭环”的当前验证方式与限制。

---

## 当前批次4重点检查项

程序运行后，应在日志中检查：

1. `SceneCache`
   - `cache_hit`
   - `cache_format_version`
   - `preprocess_mode_debug`
   - `face_count / edge_count / wedge_count`
2. `Batch4QuerySelfCheck`
   - `closest_hit`
   - `all_hits`
   - `visible`
   - `wedge_candidates`
3. 结束标记
   - `Batch4 query facade and scene cache closed loop completed.`

---

## 推荐验证顺序

1. 第一次运行：
   - 观察 `cache_hit=false`；
   - 确认 cache 文件被写出到 `output/cache/`。
2. 第二次运行：
   - 观察 `cache_hit=true`；
   - 确认仍能通过 `Batch4QuerySelfCheck`。

---

## 当前批次限制

1. `SceneCacheMeta` 当前使用 JSON；
2. `SceneCacheContent` 当前使用工程内自定义二进制序列化；
3. 当前 query self-check 仍是小场景基础自检，不等于模块4真实寻径回归；
4. 更复杂的 `PathState -> QueryContext` 派生将在模块4开发中继续接实。

---

## A3阶段补充关注点

进入 A3 后，批次4的意义不再只是“query 能不能用”，还要关注 transmission 样例下：

1. `closest_hit` 是否能稳定落到 transmission 面；
2. `all_hits` 是否呈现出可解释的 transmission 命中顺序；
3. `visible` 在 transmission 主样例中不应被误解为“必须直视可见”，因为 A3 会正式区分 LOS 被挡但 transmission path 成立的情况；
4. 若 transmission 主样例已在 scene 语义源层通过，但 query 命中顺序不稳定，应优先理解为 search 侧 transmission 成立性问题，而不是 scene 语义源问题。
5. 当前 A3 最小主样例建议优先使用 `configs/app/a3_transmission_minimal.json`；若切回复杂场景配置，再观察 transmission 命中顺序时应明确那已不再是 A3 第一主样例口径。

---

## A6阶段补充关注点

进入 A6 后，批次4的关注点应从“query/cache 能不能跑”升级为“query/cache 是否开始具备长期实验底座特征”。运行后应额外关注：

1. `SceneCache` 中是否出现：
   - `replay_ready`
   - `status_reason`
2. `status_reason` 的意义应明确理解为：
   - cache 为什么命中 / 未命中；
   - 当前 cache 是“刚从当前 scene 构建”还是“已成功回放”；
3. `Batch4QuerySelfCheck` 中除 `closest_hit / all_hits / visible / wedge_candidates` 外，还应关注：
   - `range_hits`
4. `range_hits` 的意义不是单纯多一个数字，而是帮助判断 query 结果在距离区间语义上是否稳定、可解释；
5. 对 A6 而言，批次4的真正目标不是“cache_hit=true 就算通过”，而是：
   - cache 的命中/回放原因更清楚；
   - query 行为更一致；
   - 这些增强不破坏 A1~A5 已建立主链。
