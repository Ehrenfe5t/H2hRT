# RT 仿真配置参数完全参考手册

> 适用于 v3 版本。配置格式：JSON。所有路径相对于项目根目录 `F:\RT\RT\`。

---

## 1. `app_runtime` — 运行时环境

控制程序的基础运行行为、日志和输出路径。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `mode` | string | `"debug"` | 运行模式。`"debug"` 输出详细日志和诊断信息；`"production"` 精简输出。影响日志级别过滤和诊断报告生成。 |
| `log_level` | string | `"INFO"` | 日志最低输出级别。可选：`"TRACE"`(所有)、`"DEBUG"`(调试)、`"INFO"`(常规)、`"WARN"`(警告)、`"ERROR"`(错误)、`"FATAL"`(致命)。低于此级别的日志不输出。 |
| `enable_console_logging` | bool | `true` | 是否在控制台打印日志。调试时建议开启，批量跑实验时可关闭以减少屏幕输出。 |
| `enable_file_logging` | bool | `true` | 是否写入日志文件。日志文件路径由 `log_file_path` 指定。 |
| `log_file_path` | string | `"output/logs/rt.log"` | 日志文件的输出路径。如果 `enable_file_logging` 为 false，此参数无效。 |
| `config_snapshot_directory` | string | `"output/config_snapshots"` | 配置快照的输出目录。每次运行自动将当前配置 JSON 完整复制一份到此目录，方便复现实验结果。 |
| `cache_directory` | string | `"output/cache"` | 场景缓存（SceneCache）的存储目录。缓存可跳过重复的 OBJ 解析和 BVH 构建，加速相同场景的多次运行。 |
| `run_id` | string | `"local-run"` | 本次运行的唯一标识。会出现在所有日志前缀中，也用于命名输出子目录（如 `output/<run_id>/`）。建议每次实验使用有意义的标识。 |
| `worker_threads` | int | `1` | 工作线程数。当前版本仅在部分模块（如暴力验证）中生效。设为 1 即可。 |

---

## 2. `scene_import` — 场景导入

控制 OBJ 场景文件的读取方式和材质映射规则。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `source_file` | string | — | **OBJ 场景文件路径。** 支持 Blender 导出的三角化 Wavefront OBJ 格式。文件编码应为 UTF-8。 |
| `source_format` | string | `"obj"` | 场景文件格式。当前仅支持 `"obj"`。STL/PLY 等格式预留未来支持。 |
| `scene_material_map_file` | string | — | **场景材质映射 JSON 文件路径。** 此文件定义 OBJ 中每个 Object 的材质属性（反射/透射/绕射标志、介质参数）。格式详见 `document/README.md` 1.3 节。 |
| `normalize_object_names` | bool | `true` | 是否规范化 OBJ 中的 Object 名称。开启后自动去除名称首尾空格和特殊字符，提高与 `scene_material_map.json` 的匹配成功率。 |
| `allow_name_auto_cleanup` | bool | `true` | 是否允许自动清理无法匹配的 Object 名称。开启后对未匹配的 Object 使用默认材质（Air→Concrete，仅反射）。 |

---

## 3. `scene_preprocess` — 场景预处理

控制从原始三角面构建拓扑边、绕射楔边、BVH 加速结构的行为。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `rebuild_normals` | bool | `false` | 是否重新计算面法向。OBJ 文件自带法向（`vn`行），通常不需要重建。仅在导入异常时开启。 |
| `enable_wedge_build` | bool | `true` | **是否构建绕射楔边。** 关闭后所有绕射相关的搜索和EM计算都不会执行。如果场景中无楔边或不需要绕射仿真，关闭可加速预处理。 |
| `enable_scene_cache` | bool | `false` | 是否启用场景缓存。开启后首次运行将场景预处理结果序列化到磁盘，后续运行直接从缓存加载（跳过 OBJ 解析、BVH 构建等），大幅加速相同场景的重复仿真。**注意：修改 OBJ 或 material_map 后需手动删除旧缓存。** |
| `enable_bvh_bruteforce_validation` | bool | `true` | **是否启用 BVH 的暴力验证。** 开启后会对随机采样射线同时跑 BVH 遍历和全量遍历，验证两者结果一致。**大场景（如 meeting.obj）建议关闭**——此选项会引发大量内存分配导致 `bad allocation`。仅在调试 BVH 实现时开启。 |
| `filter_non_manifold_wedge_sources` | bool | `true` | 是否过滤非流形边产生的楔边。非流形边（3 个以上面共享的边）在物理上不产生清晰的绕射。建议保持开启。 |
| `skip_coplanar_edges_for_wedge` | bool | `true` | 是否跳过共面边的楔边构建。两个面完全共面时，楔角为 180°，不产生绕射。开启可减少无效楔边数量。 |
| `preprocess_mode` | string | `"debug"` | 预处理模式。`"debug"` 输出详细诊断信息（退化面、非流形边、材质缺失等）。`"production"` 跳过诊断以加速。 |
| `scene_cache_format_version` | string | `"1.0.0"` | 场景缓存的格式版本号。用于识别缓存兼容性。 |
| `scene_preprocess_algorithm_version` | string | `"batch4-v1"` | 预处理算法的版本标识。记录构建此场景时使用的算法版本。 |
| `wedge_min_angle_deg` | double | `1.0` | **楔边最小角度（度）。** 楔角小于此值的边不产生绕射。角度太小（接近 0° 的刀刃）在 UTD 中会产生奇异。 |
| `wedge_max_angle_deg` | double | `179.0` | **楔边最大角度（度）。** 楔角大于此值的边（接近 180° 的平面）不产生绕射。接近平角的绕射系数趋于零。 |
| `bvh_leaf_size` | int | `8` | **BVH 叶节点最多包含的三角面数量。** 小值增加 BVH 深度和内存占用，但减少叶节点内的遍历开销。8~16 是经验值。大场景可适当增大以控制内存。 |
| `bvh_bruteforce_sample_count` | int | `16` | BVH 暴力验证时的随机采样射线数。仅在 `enable_bvh_bruteforce_validation=true` 时生效。 |

---

## 4. `material` — 材质数据库

控制材质电参数的数据来源和查询方式。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `material_database_file` | string | `"demo/ItuMaterial.csv"` | **ITU 材质电参数 CSV 文件路径。** 包含各材料在不同频率下的相对介电常数 ε_r、电导率 σ（S/m）、相对磁导率 μ_r。算法用此数据查询 Fresnel 反射/透射系数所需的复数介电常数。 |
| `material_mapping_file` | string | — | 场景材质映射 JSON 文件路径。与 `scene_import.scene_material_map_file` 功能相同，此处为冗余指定。 |
| `frequency_query_mode` | string | `"exact"` | 频率查询模式。`"exact"` 在 CSV 频点间做线性插值得到当前仿真频率的电参数。 |
| `allow_material_fallback` | bool | `false` | 是否允许材质查询失败时回退到默认值（ε_r=1, σ=0, μ_r=1）。`false` 时查询失败会导致 EM 计算跳过该交互。 |
| `default_background_medium` | string | `"air"` | 默认背景介质。未匹配规则的 Object 和场景空间默认使用此介质。 |

---

## 5. `antenna` — 天线配置

控制收发天线的类型和方向图数据来源。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `source_type` | string | `"Ideal"` | 天线类型。`"Ideal"` = 理想全向天线（增益 0dBi，X 方向线极化）。未来支持从文件加载真实方向图。 |
| `pattern_file` | string | `""` | **天线方向图 CSV 文件路径。** 为空时使用理想全向天线。指定路径时加载 (theta, phi, gain_dBi) 格式的方向图文件（B9 实现）。 |
| `polarization_file` | string | `""` | 天线极化文件路径（预留字段）。当前版本仅支持单个极化向量（X 方向）。 |

---

## 6. `path_search` — 几何寻径（模块 4）

**最重要的配置段。** 控制射线路径搜索的深度、机制类型、候选数量等。

### 6.1 路径深度与预算

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `max_path_depth` | int | `2` | **最大总交互次数。** 包含反射、透射、绕射的总和。一条 Tx→反射→反射→Rx 的路径 depth=2。增大此值会显著增加搜索空间和计算时间。推荐：简单场景 4~6，复杂场景 2~4。 |
| `max_reflection_count` | int | `2` | **最大反射次数。** 单条路径反射交互的上限。这个值与 `max_path_depth` 共同约束——实际反射次数 ≤ min(max_reflection_count, max_path_depth)。 |
| `max_transmission_count` | int | `0` | **最大透射次数。** 设为 0 时完全不搜索透射路径。需要透射时至少设为 1。注意：透射需要面元同时开启 `transmission_enabled=true`（在 scene_material_map.json 中配置）。 |
| `max_diffraction_count` | int | `0` | **最大绕射次数。** 设为 0 时完全不搜索绕射路径。需要绕射时至少设为 1。注意：绕射需要面元的 `diffraction_candidate_enabled=true`。 |
| `max_scattering_count` | int | `0` | 最大散射次数（当前未实现散射搜索和 EM，预留字段）。 |
| `max_consecutive_same_interaction` | int | `5` | **最大连续同类型交互次数。** 例如连续 3 次反射。此参数是性能保护阀而非物理限制——防止射线在两平行面间无限振荡。设为 0 则不限制。 |

### 6.2 机制开关

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `enable_los` | bool | `true` | 是否搜索直射（LOS）路径。几乎总是应该开启。 |
| `enable_reflection` | bool | `true` | **是否搜索反射路径。** 关闭后所有反射相关搜索和 EM 计算都会被跳过。 |
| `enable_transmission` | bool | `false` | **是否搜索透射路径。** 即使开启，单条路径的实际透射次数仍受 `max_transmission_count` 限制。 |
| `enable_diffraction` | bool | `false` | **是否搜索绕射路径。** 开启后楔边候选会被 DiffractionExpander 处理（B3 费马原理精确绕射点）。 |
| `enable_scattering` | bool | `false` | 是否搜索散射路径（当前未实现）。 |
| `enable_mixed_path` | bool | `true` | 是否允许混合路径（同一路径含多种交互类型）。v3 中此参数仅用于统计，不阻断路径生成。v2 中曾用于阻断混合路径。 |

### 6.3 候选控制

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `max_candidate_face_hits` | int | `64` | **面元候选上限。** BVH 查询返回的面元命中数上限。增大可减少漏检但增加计算量。对复杂场景建议 64~128。 |
| `max_candidate_wedges` | int | `64` | **楔边候选上限。** 楔边查询返回的候选楔边数上限。增大可减少漏检绕射路径。 |
| `keep_angle_metadata` | bool | `true` | 是否在路径节点中保留角度元数据（入射角、绕射角等）。用于调试和分析。 |
| `pruning_strategy` | string | `"basic"` | 路径剪枝策略。`"basic"` 使用预算计数器 + 连续同类型限制。未来可扩展更精细的剪枝。 |
| `dedup_strategy` | string | `"signature"` | 路径去重策略。`"signature"` 使用 64 位哈希签名（B2 实现）进行状态级和路径级去重。 |

### 6.4 Tx/Rx 坐标

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `debug_tx_x/y/z` | double | 1.0/1.0/1.0 | **发射天线坐标 (x, y, z)，单位米。** 这些坐标应与 OBJ 场景的坐标系一致（Blender 单位 = 米）。 |
| `debug_rx_x/y/z` | double | 3.0/1.0/1.0 | **接收天线坐标 (x, y, z)，单位米。** 同上。 |

### 6.5 参数之间的约束关系

```
实际反射次数 ≤ min(max_reflection_count, max_path_depth)
实际透射次数 ≤ min(max_transmission_count, max_path_depth)
实际绕射次数 ≤ min(max_diffraction_count, max_path_depth)
总交互次数 ≤ max_path_depth
连续同类型 ≤ max_consecutive_same_interaction
```

推荐配置示例：

| 场景 | max_path_depth | max_R | max_T | max_D |
|------|:---:|:---:|:---:|:---:|
| 仅 LOS | 0 | 0 | 0 | 0 |
| LOS + 反射 | 3 | 3 | 0 | 0 |
| LOS + 反射 + 绕射 | 4 | 3 | 0 | 2 |
| 全机制（含透射） | 5 | 4 | 2 | 2 |
| 深度搜索（论文实验） | 6 | 4 | 2 | 2 |

---

## 7. `sbr` — SBR 覆盖仿真（模块 4 B4）

Shooting and Bouncing Rays 正向射线追踪模式。与 `path_search` 的精确反向搜索互补。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `enabled` | bool | `false` | **是否启用 SBR 模式。** 开启后在精确搜索完成后额外执行 SBR 覆盖仿真。注意：SBR 使用独立的射线追踪引擎（SbrEngine），不依赖精确搜索的路径结果。 |
| `ray_count` | int | `10000` | **发射射线总数。** 射线越多覆盖面越密，漏检越少。实际可用射线数 = min(ray_count, 实际发射)。Fibonacci 球面分布保证均匀覆盖。2000 用于快速测试，10000~50000 用于正式实验。 |
| `max_ray_depth` | int | `6` | **SBR 射线最大反射深度。** 每条射线追踪到反射次数达到此值或功率低于阈值为止。当前 SBR 仅支持反射（透射和绕射待 v4）。 |
| `ray_power_threshold_linear` | double | `1e-6` | **射线功率阈值。** 当某条射线经多次反射后功率低于此值（线性），停止追踪该射线。`1e-6` = -60dB。增大可加速仿真但可能漏掉弱径。 |
| `rx_sphere_radius_factor` | double | `1.0` | **接收球半径调节因子。** 接收球半径 = `α × d / √3 × factor`（Seidel-Rappaport 公式），其中 α = √(4π/N) 为射线间平均角距，d 为射线总传播距离。增大 factor 命中更多 Rx 但可能重复计数，减小 factor 可能漏检。典型范围 0.5~3.0。 |
| `rx_grid_min_x/max_x` | double | `±5.0` | Rx 网格在 X 方向的范围（米）。 |
| `rx_grid_min_y/max_y` | double | `±5.0` | Rx 网格在 Y 方向的范围（米）。 |
| `rx_grid_z` | double | `1.5` | Rx 网格的统一 Z 坐标（米）。当前 Rx 网格为水平面（固定 Z）。 |
| `rx_grid_step_x/step_y` | double | `1.0` | Rx 网格在 X/Y 方向的间距（米）。间距越小，网格点越多，计算时间线性增长。 |

---

## 8. `em_solver` — 电磁求解（模块 5）

控制电磁计算的频率和模式。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `frequency_hz` | double | `2.4e9` | **仿真频率，单位 Hz。** 决定波长 λ=c₀/f，影响 Fresnel 系数（ε_c=ε_r-jσ/(ωε₀)）、UTD 绕射系数（k=2π/λ）、FSPL（λ/(4πd)²）。常用：1e9(1GHz)、2.4e9(WiFi)、3.5e9(5G)、6e9(WiGig)。 |
| `solver_mode` | string | `"Precise"` | 求解模式。`"Precise"` = 精确模式（保留全复振幅+相位+极化，64路径/Rx，相干叠加）。`"Coverage"` = 覆盖模式（仅功率，非相干叠加，16路径/Rx）。SBR 模式自动使用 Coverage profile。 |
| `enable_polarization` | bool | `true` | **是否计算极化。** 开启后反射用 TE/TM 分解（Fresnel），绕射用 soft/hard 分解（UTD），透射保留极化方向。关闭后仅计算标量功率，极化信息丢弃。 |

---

## 9. `output` — 结果输出（模块 6）

控制导出哪些结果文件和格式。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `output_directory` | string | `"output"` | 输出根目录。所有输出子目录（路径结果、信道、覆盖等）都在此目录下。 |
| `export_paths` | bool | `true` | **是否导出路径结果。** 生成 `precise_paths.json`（含每条路径的时延、相位、功率、极化、交互节点坐标）和 `precise_paths.csv`。这是最重要的输出文件。 |
| `export_cir` | bool | `false` | 是否导出信道冲激响应（CIR）。CIR 是路径时延和功率的离散序列。 |
| `export_pdp` | bool | `false` | 是否导出功率延迟谱（PDP）。PDP 是 CIR 的功率版本（无相位）。 |
| `export_aps` | bool | `false` | 是否导出角度功率谱（APS）。APS 需要每个路径的角度信息。 |
| `export_debug_files` | bool | `false` | 是否导出调试文件。开启后生成额外的诊断信息，供开发调试用。 |
| `export_config_snapshot` | bool | `true` | 是否导出当前配置快照。每次运行将完整的配置 JSON 复制到 `config_snapshots/` 目录。强烈建议保持开启以便复现实验结果。 |

---

## 10. `validation` — 验证与自检

控制自动化验证和回归检查。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `enable_basic_validation` | bool | `true` | 是否启用基础验证。开启后在导出阶段自动检查：至少有一条有效路径、输出文件数量合理、无致命错误。 |
| `enable_reference_compare` | bool | `false` | 是否与参考结果对比（预留，当前未实现）。 |
| `run_module1_self_check` | bool | `true` | **是否运行模块 1 自检。** 开启后用预设的无效配置（如缺少透射映射、无楔边绕射配置）验证模块 1 的错误检测能力。正常仿真建议关闭。 |
| `module1_invalid_transmission_case_file` | string | — | 模块 1 自检用的无效透射配置路径。 |
| `module1_invalid_diffraction_case_file` | string | — | 模块 1 自检用的无效绕射配置路径。 |
| `power_tolerance_db` | double | `0.1` | 功率验证容差（dB）。在对比验证中，两条路径功率差小于此值视为一致。 |

---

## 11. `experiment` — 实验标记

用于标记和组织批量实验。

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `experiment_tag` | string | — | 实验标签。用于在输出中标记本次运行属于哪个实验组。建议使用有意义的标识（如 `"meeting-LOS-baseline"`）。 |
| `dataset_tag` | string | — | 数据集标签。标记使用的场景/数据集。 |

---

## 12. `numeric_tolerance` — 数值容差

控制几何计算和去重的容差阈值。**一般不需要修改。**

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `eps_length` | double | `1e-6` | 长度比较容差（米）。小于此值的距离差视为相等。增大可减少数值噪声导致的误判，但可能合并实际不同的点。 |
| `eps_angle` | double | `1e-6` | 角度比较容差（弧度）。用于判断方向向量是否平行或正交。 |
| `eps_intersection` | double | `1e-7` | **射线-三角面求交容差。** 小于此值的 `t` 参数视为无效交点（防止自交）。此参数影响求交精度，不宜随意修改。 |
| `eps_normal` | double | `1e-6` | 法向归一化容差。法向分量小于此值时视为退化法向。 |
| `eps_deduplicate` | double | `1e-5` | **去重容差（米）。** 两条路径的交互点在此距离内视为同一点，触发去重。增大可合并更多近似重复路径，减小可保留更细微的路径差异。 |
| `eps_power` | double | `1e-9` | 功率比较容差（线性）。小于此值的功率差异视为相等。 |
| `self_hit_ignore_distance` | double | `1e-5` | **自交忽略距离（米）。** 射线从面元出发时，忽略此距离内的命中（防止射线击中自己出发的面）。 |
| `visibility_origin_offset` | double | `1e-5` | 可见性判断的起点偏移（米）。从交互点向 Rx 做可见性测试时，将起点沿射线方向偏移此距离，防止自遮挡误判。 |
| `visibility_target_shrink` | double | `1e-5` | 可见性判断的终点收缩（米）。同上，将终点反向偏移此距离。 |

---

## 附录 A：meeting.obj 可用配置（已验证）

```json
// meeting_v3.json — meeting.obj 全流程闭环配置
// 关键修改 vs minimal.json:
// 1. enable_bvh_bruteforce_validation: false  ← 修复 bad allocation
// 2. run_module1_self_check: false            ← 跳过自检
// 3. max_path_depth: 4, enable_transmission: true, enable_diffraction: true
// 4. Tx=(16,1.5,-12), Rx=(10,1.5,-10)
// 结果: 184 paths, 14 种混合路径类型, validation_passed=true
```

## 附录 B：快速配置清单（从零开始）

1. 复制 `configs/app/b1_mixed_path_test.json` 作为模板
2. 修改 `scene_import.source_file` → 你的 OBJ 文件路径
3. 修改 `scene_import.scene_material_map_file` → 你的材质映射 JSON
4. 修改 `path_search.debug_tx_*` / `debug_rx_*` → 你的收发坐标
5. 修改 `em_solver.frequency_hz` → 你的仿真频率
6. 根据场景复杂度调整 `max_path_depth` 和逐机制预算
7. 大场景关闭 `enable_bvh_bruteforce_validation`
8. 运行 `x64/Debug/RT.exe configs/app/your_config.json`
