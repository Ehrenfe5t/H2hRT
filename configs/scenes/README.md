# configs/scenes 说明文档

## 目录目标

本目录用于存放模块2场景导入与语义恢复阶段使用的场景级规则文件。

当前批次2中，最关键文件是：

- `scene_material_map.json`

其作用不是直接保存每个面元的双侧材质结果，而是提供：

1. 对象名到对象语义类型的映射；
2. 对象主材质定义；
3. front/back 材质规则；
4. 法向解释规则；
5. 是否允许 reflection / transmission / diffraction 候选的对象级策略。

---

## 当前样例与 demo/meeting.txt 的关系

由于 `demo/meeting.txt` 当前对象名是数字：

- `14`
- `7`
- `9`
- `1`
- `11`
- `4`
- `10`
- `13`

它本身不足以直接表达 `wall / window / floor` 等语义类别。

因此当前批次2采用：

> OBJ 原始对象名 + 外部 `scene_material_map.json` 规则恢复对象语义与双侧材质

这是符合开发草案第一版策略的：

- 建模端不强求逐面手工双侧材质；
- 模块2通过命名、法向、规则文件恢复可供模块4/5消费的语义层。

---

## `scene_material_map.json` 当前字段说明

### 顶层字段

#### `default_medium`
- 缺省外侧介质。
- 当前设置为 `Air`。

#### `objects`
- 对象级规则数组。

### 每条对象规则字段

#### `rule_name`
- 规则名称，仅用于调试和日志输出。

#### `object_name`
- 需要匹配的 OBJ 对象名。

#### `object_type`
- 模块2恢复出的对象语义类型，如：
  - `floor`
  - `ceiling`
  - `wall`
  - `window`
  - `door`
  - `table`

#### `surface_material_name`
- 对象主材质名。

#### `front_material_name` / `back_material_name`
- 当前第一版直接给出面正侧与反侧材质。

#### `normal_rule`
- 法向解释标签。
- 当前样例统一使用 `front_is_air`。

#### `reflection_enabled`
- 是否允许后续作为反射主交互对象。

#### `transmission_enabled`
- 是否允许后续作为透射边界对象。

#### `diffraction_candidate_enabled`
- 是否允许后续参与绕射候选边提取。

---

## 当前批次限制

1. 当前规则匹配采用 `object_name` 精确匹配，不支持复杂通配符；
2. 当前样例先服务于 `demo/meeting.txt`；
3. 更复杂的名称模式匹配、对象类别推断和法向修正规则可在后续批次继续增强。
