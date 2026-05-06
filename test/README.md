# test 目录说明

## 脚本结构（v3 重构后）

```
test/
├── rt_utils.py                  ← 共享工具库（OBJ解析/配置加载/路径读取/坐标变换/RT运行器）
├── visualize_scene_tx_rx.py     ← 统一可视化工具（场景+Tx/Rx+路径+材质检查+覆盖预留）
├── rt_grid_search.py            ← 统一参数化网格搜索（mixed/transmission/reflection/diffraction/any）
├── check_batch2_binding.py      ← 兼容入口 → 重定向到 visualize_scene_tx_rx.py
├── search_a2_mixedpath_txrx_legacy.py   ← 已归档
├── search_a3_transmission_txrx_legacy.py ← 已归档
├── README.md
├── README_batch3.md ~ README_batch9.md
└── output/
```

## 快速使用

### 可视化场景+路径+材质

```bash
# 默认场景
python test/visualize_scene_tx_rx.py

# B1 混合路径测试场景
python test/visualize_scene_tx_rx.py --obj demo/b1_mixed_path_test.obj --config configs/app/b1_mixed_path_test.json --material-map configs/scenes/scene_material_map_b1_test.json
```

### 网格搜索最优 Tx/Rx

```bash
# 搜索混合路径
python test/rt_grid_search.py --mode mixed

# 搜索透射路径
python test/rt_grid_search.py --mode transmission --config configs/app/a3_transmission_minimal.json

# 自定义网格范围
python test/rt_grid_search.py --mode mixed --tx-range-x 1,5,1 --rx-range-x 1,5,1
```

### 编程方式使用共享库

```python
from rt_utils import *
v, tri, names = parse_obj_geometry(Path("demo/b1_mixed_path_test.obj"))
tx, rx = load_tx_rx_from_config(Path("configs/app/b1_mixed_path_test.json"))
paths = load_exported_paths(DEFAULT_PATH_JSON)
stats = classify_paths(paths)
result = run_rt_pipeline("configs/app/b1_mixed_path_test.json")
```

## 预留扩展接口

- `visualize_scene_tx_rx.py` — 预留覆盖结果可视化（`_render_coverage` 方法占位）
- `rt_utils.py` — 预留 SBR 结果加载（`load_coverage_result` 函数占位）
- `rt_grid_search.py` — 预留 SBR coverage search 模式
- B5+ 材质数据库查询：`load_material_database()` / `query_material_at_freq()` 已就绪

## 依赖

```
numpy  pyvista  pyvistaqt  pyqt5
```
