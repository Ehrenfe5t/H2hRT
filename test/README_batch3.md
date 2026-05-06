# 批次3验证说明

## 文件用途

本说明文档用于解释模块2批次3新增的“拓扑、诊断与加速结构”验证关注点。

当前批次3主链验证依赖程序运行日志，不额外要求新的 Python 主验证脚本即可完成闭环。

---

## 当前批次3重点检查项

运行程序后，需重点检查日志中是否出现以下摘要：

1. `Batch3Topology`
   - 面元数
   - 棱边数
   - 楔边数
2. `SceneDiagnostics`
   - 退化面数量
   - 非流形边数量
   - 重复面数量
   - 缺少双侧材质面数量
   - `transmission_faces_missing_semantics`
3. `SceneAcceleration`
   - BVH 节点数
   - 叶节点数
   - 最大深度
   - 平均叶面元数
   - 楔边查询记录数
   - brute-force 对照是否通过

---

## 当前批次限制

1. brute-force vs BVH 当前为轻量抽样一致性检查，主要确保加速结构字段与有效面集合一致；
2. 当前尚未进入 `SceneQuery` 正式查询门面，因此 BVH 查询接口仍将在批次4继续闭环；
3. 当前未新增 GUI 工具，批次2 的可视化脚本仍主要服务对象语义和材质人工核查。

---

## 推荐人工验证方式

1. 用 VS2022 运行默认配置 `configs/app/minimal.json`；
2. 检查日志是否出现：
   - `Batch2 scene import and semantic recovery closed loop completed.`
   - `Batch3 topology, diagnostics and acceleration closed loop completed.`
3. 检查 `SceneDiagnostics` 中关键异常数量是否合理；
4. 检查 `SceneAcceleration` 中 brute-force 对照结果是否为 `true`。

---

## A3阶段补充关注点

当前进入 A3 后，批次3的 diagnostics 不再只服务几何基础检查，还开始承担 transmission 语义源检查职责。运行后应额外关注：

1. transmission 面是否出现 `transmission_faces_missing_semantics > 0`；
2. 若出现 transmission 相关 diagnostics 异常，应优先理解为：scene 侧 transmission 语义源不可信，而不是 search / em 先出问题；
3. 对 A3 主 transmission 样例，应优先看到 transmission 相关 diagnostics 为 0，才说明 scene 语义源具备进入后续 search / em 的资格。
4. 建议在 A3 transmission 主样例阶段优先使用 `configs/app/a3_transmission_minimal.json`，而不是继续沿用复杂主场景配置，以避免把 transmission 语义问题与复杂场景干扰混在一起。

---

## A6阶段补充关注点

进入 A6 后，批次3的 diagnostics 已不再只是基础几何检查，还开始承担“场景质量前置层”职责。运行后应额外关注：

1. `SceneDiagnostics` 中是否出现：
   - `pattern_matched_objects`
   - `default_filled_objects`
   - `partial_semantic_objects`
   - `unresolved_binding_objects`
2. 若 `partial_semantic_objects > 0` 或 `unresolved_binding_objects > 0`，应优先理解为：
   - scene 语义恢复质量不足；
   - 当前场景不宜直接被视为高质量正式实验底座；
3. 若日志中出现：
   - `PartialSemanticRecoveryObject`
   - `UnresolvedBindingObject`
   - `DiagnosticsWarning`
   则说明模块2已经开始把“弱恢复”和“弱质量场景”显式暴露出来，而不是继续静默流入后续模块；
4. 对 A6 主场景与补充场景的检查重点，不再只是“能不能导进来”，而是“导进来后的语义恢复质量是否足以长期支撑后续 search / em / export / regression”。
