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
