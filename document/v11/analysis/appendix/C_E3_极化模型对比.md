# 附录 C：E3 极化模型对比

> **历史数据，已废止。** 本文的“极化无差异”来自 6 列 Jones CSV 加载失败，问题已经修复。当前结果见 [V11.5 仿真实验分析报告](../v11.5_仿真实验分析报告.md)。

> 状态：⚠️ 极化效果未显现——需进一步调查  
> 实验变量：Tx/Rx 极化模型（Fixed-Fixed / Jones-Fixed / Fixed-Jones / Jones-Jones）  
> 控制变量：500k 射线，meeting 场景，Tx=Rx=patch_gain

---

## C.1 结果

| 极化组合 | 总功率 (W) | RMS-DS (ns) | K-factor (dB) | 有效路径数 |
|---|---|---|---|---|
| Fixed-Fixed | 5.69E-09 | 8.8 | −58.7 | 697 |
| Jones-Fixed | 5.69E-09 | 8.8 | −58.7 | 697 |
| Fixed-Jones | 5.69E-09 | 8.8 | −58.7 | 697 |
| Jones-Jones | 5.69E-09 | 8.8 | −58.7 | 697 |

**四种极化组合的所有聚合指标完全一致。**

## C.2 根因分析

可能原因（按可能性排序）：

### 1. Jones 极化文件未被正确加载

`patch_jones.csv` 可能加载失败。虽然 E1 验证了 `patch_gain.csv` 加载正确，但极化 CSV 格式不同（7 列：theta, phi, gain, PolTheta_re, PolTheta_im, PolPhi_re, PolPhi_im），可能存在格式解析问题。

### 2. Jones 极化效果在聚合层面被平均化

每条路径的 Jones 极化响应不同，但在数万条路径的非相干求和中，极化差异被统计平均抵消。路径级的 XPR 可能有差异，但聚合到 `xpr_stats.json` 中后，mean/median 可能相近。

### 3. patch_jones.csv 中极化纯度较高

如果 patch 天线的 Jones 极化在大多数角度都是接近纯垂直极化（PolTheta≈1, PolPhi≈0），则与 Fixed 垂直极化几乎相同，差异不可察觉。

## C.3 验证建议

1. **路径级对比**：选取 LOS 路径（path_id 相同或 LOS），比较 Fixed-Fixed vs Jones-Jones 的 `phase_rad`、`co_pol_power_linear`、`cross_pol_power_linear`
2. **APS 对比**：比较 2D APS 网格中 `power_grid_linear` 的差异
3. **CIR 相位对比**：比较 `cir.json` 中复振幅的相位分布
4. **CSV 格式验证**：检查 `patch_jones.csv` 是否能被 `LoadPolarizationCsv()` 正确解析
5. **XPR CDF 对比**：比较 `xpr_stats.json` 中 CDF 数组的差异

## C.4 当前结论

在聚合信道统计层面，Jones 极化模型与 Fixed 极化模型在当前实验设置下**未表现出可检测的差异**。这不意味着 Jones 极化没有物理效果——它可能体现在路径级的相位和单路径 XPR 上——但聚合指标无法捕获。

**这本身是一个有意义的发现**：对于 patch 天线（极化纯度较高），在统计意义上 Fixed 极化是 Jones 极化的良好近似。但对于极化纯度低的天线（如某些宽带天线），差异可能显著。

---

*（待路径级分析后更新）*
