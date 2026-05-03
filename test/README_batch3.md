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
