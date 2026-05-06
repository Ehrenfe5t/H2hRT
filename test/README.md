# test 目录说明

## 目录目标

本目录用于存放当前项目的人工核查脚本、快速验证脚本与后续批次的辅助检测脚本。

当前已加入：

- `check_batch2_binding.py`
- `visualize_scene_tx_rx.py`
- `README_batch3.md`
- `README_batch4.md`
- `README_batch5.md`
- `README_batch6.md`
- `README_batch7.md`
- `README_batch8.md`
- `README_batch9.md`

用于检查批次2中：

1. OBJ 对象是否被正确读取；
2. `scene_material_map.json` 是否被正确解析；
3. 对象级语义绑定是否正确；
4. 面元双侧材质恢复结果是否合理；
5. 是否需要进一步人工核查某些对象或面元。

另外：

- `README_batch3.md` 用于说明批次3当前主链日志验证项与人工核查重点。
- `README_batch4.md` 用于说明批次4查询门面与缓存闭环的日志验证项与 cache 命中检查方式。
- `README_batch5.md` 用于说明批次5 SearchEngine 骨架与 LOS 闭环的日志验证项。
- `README_batch6.md` 用于说明批次6三类扩展器自检与 mixed path 基础串联日志验证项。
- `README_batch7.md` 用于说明批次7路径级电磁主链与 EMPathResult 验证项。
- `README_batch8.md` 用于说明批次8 CIR/PDP/APS/统计/覆盖/通感摘要与双模式 profile 验证项。
- `README_batch9.md` 用于说明批次9结构化导出、验证报告与回归报告闭环验证项。

---

## 当前脚本设计原则

为适配你当前的 **Anaconda + PyCharm** 使用方式，当前脚本已调整为：

1. **不依赖命令行参数**；
2. 所有可修改项统一放在脚本顶部“可修改配置区”；
3. 你只需要在 PyCharm 中打开脚本，改配置，然后直接运行；
4. 当前脚本已重构为 **PyQt + PyVista 交互界面**；
5. 支持导出 JSON 检查结果，便于后续回归。

---

## 当前界面能力

当前 GUI 已支持：

1. 左侧控制面板；
2. 右侧 3D 交互场景窗口；
3. 按：
   - 背侧材质
   - 对象语义
   - 对象名称
   三种模式着色；
4. 按对象、对象语义、背侧材质筛选；
5. 仅查看未解析面元；
6. 显示/隐藏线框；
7. 显示/隐藏图例；
8. 显示/隐藏对象标签；
9. **按 X / Y / Z 任一坐标轴进行“上半部分去顶裁剪”**；
10. 导出当前检查结果到 JSON。

另外，新增：

11. `visualize_scene_tx_rx.py`：
   - 可直接可视化 `demo/meeting.obj`；
   - 可在 UI 中输入算法使用的 Tx/Rx 坐标；
   - 默认采用与当前 RT 算法 `OBJImporter` 一致的“无坐标变换”显示方式；
   - 支持实验性显示变换，用于辅助判断 Blender 坐标系差异；
   - 支持把当前 Tx/Rx 直接回写到脚本读取的配置文件；
   - 支持按 X / Y / Z 某个轴做真正的几何裁剪式去顶；
   - 无论是否切换实验显示模式，都会同步变换场景与 Tx/Rx 标记，保证视觉对应关系不乱。

---

## 推荐运行方式

### 第一步：在 PyCharm 中打开

打开：

```text
test/check_batch2_binding.py
```

或打开：

```text
test/visualize_scene_tx_rx.py
```

### 第二步：按需修改顶部配置项

例如：

- `OBJ_FILE_PATH`
- `RULE_FILE_PATH`
- `SUMMARY_ONLY`
- `SHOW_VISUALIZATION`
- `EXPORT_JSON_RESULT`
- `EXPORT_JSON_PATH`
- `DEFAULT_SUMMARY_FACE_COUNT`
- `DEFAULT_COLOR_MODE`
- `DEFAULT_ENABLE_CLIP`
- `DEFAULT_CLIP_AXIS`
- `DEFAULT_CLIP_KEEP_LOWER_RATIO`

### 第三步：直接点击运行

---

## 当前默认行为

若你不改配置，默认会：

1. 读取：
   - `demo/meeting.txt`
   - `configs/scenes/scene_material_map.json`
2. 打开 GUI 交互界面；
3. 在左侧显示对象级与面元级摘要；
4. 导出 JSON 检查结果到：

```text
test/output/batch2_check.json
```

而 `visualize_scene_tx_rx.py` 默认会：

1. 读取 `demo/meeting.obj`；
2. 读取 `configs/app/minimal.json` 中当前 `debug_tx_* / debug_rx_*` 作为默认 Tx/Rx；
3. 以与当前 RT 算法一致的坐标系显示场景；
4. 在界面中用红色/蓝色球标出 Tx / Rx 位置；
5. 允许你一边观察场景，一边修改算法将要使用的坐标值；
6. 支持把当前 Tx/Rx 直接回写到当前读取的 `configs/app/minimal.json`；
7. 支持按轴裁掉超过某阈值的上半部分，并且是几何裁剪，不是整面直接隐藏。

---

## Anaconda / PyCharm 环境说明

当前机器上检测到：

```text
C:\ProgramData\anaconda3
```

说明系统中存在 Anaconda 安装，但当前 shell 未自动把 `conda` 加入 PATH。

因此建议在 PyCharm 中：

1. 直接选择 Anaconda 解释器；
2. 或选择后续在 `test/environment.yml` 中创建的专用环境；
3. 若要正常运行当前 GUI，请确保安装以下库：

```text
numpy
matplotlib
pyqt
pyvista
pyvistaqt
```
