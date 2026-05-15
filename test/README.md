# RT 测试与可视化工具集 (v7)

> RT 电磁传播仿真系统的 Python 测试、验证和可视化工具集。
> 对标 Sionna RT 绘图风格。所有脚本可独立运行。

## 快速开始

```bash
# 1. 信道数据绘图 (CIR/PDP/AoD/AoA/Stats) — Sionna 风格
python test/plot_channel.py

# 2. 功率覆盖 3D 交互可视化 — 继承 scene_positions.py 渲染风格
python test/plot_coverage_3d.py

# 3. 功率覆盖静态热力图 (PNG) — Sionna RadioMap 风格
python test/plot_coverage_static.py --all-planes

# 4. 3D 场景 + 路径 + 材质交互可视化
python test/rt_visual.py

# 5. 三级验证体系运行
python test/validate/run_all.py
```

## 脚本清单

### `rt_utils.py` — 共享工具库

所有脚本的公共依赖库。提供:
- OBJ 解析 (`parse_obj`/`parse_obj_geometry`)
- 配置加载 (`load_app_config`/`load_tx_rx_from_config`)
- 路径数据读取 (`load_precise_paths`/`load_exported_paths`)
- 信道计算 (对标 Sionna): `compute_cir`, `compute_pdp`, `compute_aod_aoa`, `compute_channel_params`, `compute_cdf`
- 覆盖数据读取: `load_coverage_records`
- dB 转换: `safe_db`
- 坐标变换, 场景包围盒, PyVista PolyData 构建
- RT.exe 运行器: `run_rt_pipeline`
- 材质数据库加载: `load_material_database`/`query_material_at_freq`
- Sionna 风格 matplotlib 默认参数: `set_sionna_style()`

### `plot_channel.py` — 信道数据绘图 (Sionna风格)

从 `precise_paths.json` 生成 Sionna RT 匹配的全套信道图表:

| 图表 | 说明 |
|------|------|
| Fig1: CIR + PDP | 茎叶图: 复振幅-时延 (navy stem) + 功率-时延 (orange stem) |
| Fig2: AoD + AoA | 极坐标散点: 发射角/到达角, 功率加权尺寸, hot彩图着色 |
| Fig3: Stats | 功率-时延散点 (plasma着色) + 功率 CDF (蓝色线+填充) |

用法:
```bash
python test/plot_channel.py \
  -i output/a1_real_chain/paths/precise_paths.json \
  -o output/plots/channel \
  -t "Meeting Room"
```

### `plot_coverage_3d.py` — 功率覆盖 3D 交互相机

PyVista/PyQt5 交互窗口。继承 `E:\hzh\usrp\代码\scene_positions.py` 渲染风格:
- **场景渲染**: OBJ 建筑面元 (smooth_shading) + 特征边线 + 3光源系统
- **天花板裁剪 (去顶)**: X/Y/Z 轴切除上半部分, 可调阈值立即预览
- **功率覆盖切片**: X/Y/Z 平面选择, 位置滑块, 原始功率热力图 (jet colormap)
- **无平滑处理**: 每个 Rx 网格点直接显示原始功率值 (StructuredGrid, interpolation=none)
- **全空间覆盖**: 无功率的 Rx 位置按 colorbar 最低值填充 (不空白)
- **Tx/Rx 天线标记**: 红色 Tx 球 + 蓝色 Rx 球

用法:
```bash
# 默认: meeting.obj + a1_real_chain coverage
python test/plot_coverage_3d.py

# 自定义场景和覆盖数据
python test/plot_coverage_3d.py \
  --obj demo/meeting.obj \
  --sbr output/meeting-cov-hires/coverage/sbr_coverage.json
```

### `plot_coverage_static.py` — 功率覆盖静态热力图

matplotlib 批量输出 PNG。对标 Sionna RadioMap 2D 热力图风格:
- 支持 XY / XZ / YZ 三个方向的平面切片
- `--plane xz` 选平面, `--position 1.5` 定位置
- `--all-planes` 一次性三个平面全出
- 输出 `coverage_stats.json` 覆盖统计

用法:
```bash
# 单个平面: XZ 平面 (俯视) 在 Y=1.5m 高度切片
python test/plot_coverage_static.py --plane xz --position 1.5

# 全部三个平面 (自动选取中位位置)
python test/plot_coverage_static.py --all-planes -o output/plots/coverage

# 指定天线标记位置
python test/plot_coverage_static.py --plane xz --tx 16.0 1.5 -12.0 --rx 10.0 1.5 -10.0
```

### `rt_visual.py` — 3D 场景 + 路径 + 材质交互工具

综合交互式可视化:
- OBJ 场景渲染 (面元/线框/材质着色/裁剪)
- Tx/Rx 天线位置 (可拖拽, 可回写配置 JSON)
- 多径轨迹 (按交互类型分色显示)
- 材质规则绑定摘要 (表格视图)
- 预留: SBR 覆盖结果叠加

用法:
```bash
python test/rt_visual.py
```

### `validate/run_all.py` — 三级验证体系入口

L1/L2/L3 全量验证一键运行:
- L1: 解析解单元验证 (Fresnel TE/TM, Friis FSPL, Snell折射)
- L2: 与 RT.exe 交叉验证 (路径数/引擎运行)
- L3: 统计域验证

用法:
```bash
python test/validate/run_all.py
```

### 辅助/参考脚本

| 文件 | 说明 |
|------|------|
| `validate/cross_ref/scene_translator.py` | OBJ → 参考实现 CSV 格式转换 |
| `validate/cross_stats/macro_validate.py` | 宏观物理合理性检查 (LOS/FSPL/时延) |
| `validate/cross_stats/validate_physical.py` | 8项物理合理性验证 |
| `validate/full_validate.py` | 针对 meeting_full 场景的 7项验证 |

## 依赖

```
numpy>=1.20     pyvista>=0.40    pyvistaqt>=0.2
PyQt5>=5.15     matplotlib>=3.5   trimesh>=4.0
```

## L1 解析解验证套件 (v8 新增)

独立的6场景解析解自主验证闭环:

```
test/
├── scenes/L1_canonical/   ← 6个手写 OBJ 规范场景
├── configs/               ← 每场景独立 JSON 配置
├── materials/L1_materials.csv  ← ITU-R P.2040 材质库
├── expected/L1_expected.json   ← 理论期望值
└── validate/
    ├── run_L1_validation.py    ← 独立理论计算 (无需仿真输出)
    └── validate_L1.py          ← 仿真输出对比验证
```

**用法**:
```bash
# 1. 验证理论计算 (独立, 无需仿真)
python test/validate/run_L1_validation.py --verbose

# 2. 运行各场景仿真后验证
python test/validate/validate_L1.py test/output/
```

## 输出目录约定

```
output/
├── plots/
│   ├── channel/          ← plot_channel.py 输出
│   │   ├── *_01_cir_pdp.png
│   │   ├── *_02_angles.png
│   │   ├── *_03_stats.png
│   │   └── *_channel_params.json
│   └── coverage/         ← plot_coverage_static.py 输出
│       ├── *_coverage_xy.png
│       ├── *_coverage_xz.png
│       ├── *_coverage_yz.png
│       └── *_coverage_stats.json
├── {run_id}/
│   ├── paths/reference_manifest.json
│   ├── coverage/sbr_coverage.json
│   └── ...
└── validate_results/     ← run_all.py 输出
    └── validate_report.json
```
