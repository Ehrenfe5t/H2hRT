# configs/app 说明文档

## 目录目标

本目录存放 `AppConfig` 对应的应用级 JSON 配置文件。

由于 JSON 不适合逐行加入实现级注释，因此按新增开发要求，使用本说明文档描述：

1. 配置文件用途；
2. 顶层字段含义；
3. 当前最小示例 `minimal.json` 的使用方式；
4. 当前批次的限制和后续扩展方向；
5. 模块1自检样例的作用。

---

## `minimal.json` 的目标

`minimal.json` 用于支撑批次0 / 批次1 的最小闭环验证，目标是：

1. 程序可在 VS2022 下成功启动；
2. 模块1可以完成配置加载；
3. 模块1可以完成基础配置校验；
4. Logger 可以输出控制台日志与文件日志；
5. VersionInfo 可以输出版本摘要；
6. 后续模块2可继续复用该配置结构扩展字段。

> 注意：`minimal.json` 不是“最终正式仿真配置终版”，而是**正式 JSON 主线下的最小可运行样例**。
> 后续正式仿真仍然采用 JSON / `AppConfig` 主线，但字段会继续补全，不会停留在这个最小样例规模。

当前模块1已切换到**正式结构化 JSON 解析路径**，不再依赖此前的字符串扫描式最小加载方式。

---

## 顶层配置块说明

### `app_runtime`
- 控制运行模式、日志级别、日志输出路径、线程数等。

### `scene_import`
- 指定场景输入文件及其格式。
- 当前最小闭环使用 `demo/meeting.txt`，按 OBJ 文本处理。

### `scene_preprocess`
- 预留给模块2的法向重建、wedge 构建、BVH 参数等。

### `material`
- 指定材料数据库路径、材料映射文件路径与材料查询模式。
- 当前 transmission 默认关闭，因此 `material_mapping_file` 可以为空。

### `antenna`
- 预留给模块3的天线输入与外部模式文件配置。

### `path_search`
- 控制 LOS / Reflection / Transmission / Diffraction 开关与深度预算。
- 正式仿真方向下，除了总深度，还应包含各机制独立预算，例如：
  - `max_reflection_count`
  - `max_transmission_count`
  - `max_diffraction_count`
  - `max_scattering_count`
  - 候选数量、剪枝策略、去重策略等

### `em_solver`
- 控制工作频率与求解模式。

### `output`
- 控制输出目录、是否导出路径/统计结果/调试文件/配置快照等。

### `validation`
- 控制是否启用基础验证与参考对照。
- 当前还承担模块1自检入口：
  - `run_module1_self_check`
  - `module1_invalid_transmission_case_file`
  - `module1_invalid_diffraction_case_file`

### `experiment`
- 记录实验标签与数据集标签，便于回归追踪。

### `numeric_tolerance`
- 统一维护几何与电磁相关数值容差。

---

## 当前样例的约束

1. 当前 JSON 加载器是批次0/1的最小实现，不是完整 JSON 引擎；
2. 建议当前样例保持简单平坦的对象字段形式；
3. 若后续增加复杂数组、深层嵌套对象或转义场景，应同步升级正式 JSON 解析方案。

> 更新：模块1当前已经升级到结构化 JSON 解析路径；上述限制主要转为“当前 schema 仍是第一版，不代表所有后续模块字段都已最终补齐”。

---

## 当前验证方式

在仓库根目录运行：

```powershell
.\x64\Debug\RT.exe
```

默认读取：

```text
configs/app/minimal.json
```

成功时应看到：

1. 控制台输出启动日志；
2. `output/logs/rt.log` 被创建；
3. 日志中出现批次0/1闭环成功信息。

---

## 当前目录中的验证样例

### 正常样例
- `minimal.json`
  - 除正常启动外，还会触发模块1自检，验证关键负样例规则未回退。

### 错误样例
- `invalid_transmission_missing_mapping.json`
  - 用于验证 transmission 开启但未提供材料映射文件时，模块1能否前置拦截。
- `invalid_diffraction_without_wedge.json`
  - 用于验证 diffraction 开启但 preprocess 未启用 wedge 构建时，模块1能否前置拦截。
