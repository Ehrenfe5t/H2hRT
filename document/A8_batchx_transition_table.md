# A8 BatchX 接管关系表

## 1. 分层口径

### 正式资产

- `core/result/*`
- `document/A8_handoff_view.md`
- `document/A8_delivery_note.md`
- 模块1~6正式实现文件

### 过渡资产

- `app/Batch8AggregateReporter.cpp`
- `app/Batch9ExportReporter.cpp`
- `test/README_batch7.md`
- `test/README_batch8.md`
- `test/README_batch9.md`

### 历史 / 样例资产

- 前面批次遗留的样例脚本
- 仅用于机制演示、趋势演示、历史说明的旧 BatchX 资产

## 2. 接管关系表

| 资产 | A8后角色 | 正式接管者 | 备注 |
|---|---|---|---|
| `Batch7` | 机制样例层 | `core/result/*` + 正式主链结果 | 不再是正式交付入口 |
| `Batch8` | aggregate 趋势样例层 | `core/result/*` + `Batch9` 交付视图 | 不再承载正式交付主视图 |
| `Batch9` | 主场景结果表达与过渡交接入口 | `core/result/*` + 正式文档层 | 仍可保留，但不能反客为主 |
| `README_batch7` | 机制核查说明 | `A8_handoff_view.md` / `A8_delivery_note.md` | 保留说明，不做主依据 |
| `README_batch8` | 趋势核查说明 | `A8_handoff_view.md` / `A8_delivery_note.md` | 保留说明，不做主依据 |
| `README_batch9` | 过渡交接说明 | `A8_handoff_view.md` / `A8_delivery_note.md` | 是辅助入口，不是唯一依据 |

## 3. 保留 / 归档 / 退场口径

### 保留

以下资产建议保留：

1. 对机制核查仍有价值的 Batch7 说明
2. 对 aggregate 趋势观察仍有价值的 Batch8 说明
3. 对主场景结果导出仍有入口价值的 Batch9 说明

### 归档

以下资产可逐步转归档心智，而非立即物理删除：

1. 只承担历史解释作用的旧批次说明
2. 已被正式导出层和正式文档层替代的重复口径资产

### 退场

以下行为应视为退场目标，而不是当前动作：

1. 让 BatchX 再次成为主实现承载层
2. 让 README_batchX 再次成为唯一交付依据
3. 让样例资产继续承担正式 schema 定义职责

## 4. 使用规则

1. 看正式输出结构，优先看 `core/result/*`
2. 看当前如何接手，优先看 `A8_handoff_view.md`
3. 看最终交付口径，优先看 `A8_delivery_note.md`
4. 看 BatchX 历史角色，再回看 `README_batch7/8/9.md`
