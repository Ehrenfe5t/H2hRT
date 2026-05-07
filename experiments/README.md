# V4 论文实验模板

## 可用实验配置

| 配置 | 用途 | 路径搜索 | SBR |
|------|------|---------|-----|
| `../configs/app/meeting_v3.json` | meeting.obj 全机制 (depth=8, R/T/D全开) | 664 paths | 关闭 |
| `../configs/app/b1_mixed_path_test.json` | 混合路径测试盒 (depth=4, R+D) | 362 paths | 关闭 |
| `../configs/app/a3_transmission_minimal.json` | 单面透射基线 (depth=2, T only) | 2 paths | 关闭 |
| `../configs/app/meeting_coverage.json` | meeting.obj SBR 覆盖 (20000 rays, 8 Rx) | 19 paths | **开启** |
| `../configs/app/b4_sbr_test.json` | 混合路径测试盒 SBR (2000 rays, 15 Rx) | 362 paths | **开启** |

## 快速运行

```bash
# 精确路径仿真
x64/Debug/RT.exe configs/app/meeting_v3.json

# SBR 覆盖仿真
x64/Debug/RT.exe configs/app/meeting_coverage.json

# 验证体系
python test/validate/run_all.py

# 论文图表
python test/plot_pdp_aps.py
```

## 修改 Tx/Rx 坐标

编辑对应配置文件的 `path_search.debug_tx_*` / `debug_rx_*` 字段。

## 修改仿真频率

编辑 `em_solver.frequency_hz` 字段。常用值: 1e9 (1GHz), 2.4e9 (WiFi), 3.5e9 (5G), 6e9 (WiGig)。
