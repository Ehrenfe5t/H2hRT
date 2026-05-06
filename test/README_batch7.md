# 批次7验证说明

## 文件用途

本说明文档用于解释模块5批次7“基础物理求值主链”的当前验证方式。

---

## 当前批次7重点检查项

运行程序后，应在日志中检查：

1. `EMLosCheck`
   - `valid`
   - `delay`
   - `phase`
   - `power`
   - `fs_loss_db`
   - `pol_mag`
2. `EMReflectionCheck`
3. `EMTransmissionCheck`
   - `tx_semantic=true`
   - `medium_in / medium_out`
4. `EMDiffractionCheck`
5. 结束标记
   - `Batch7 module5 EM main-chain closed loop completed.`

---

## 推荐验证顺序

1. 运行默认配置 `configs/app/minimal.json`；
2. 检查批次6仍正常闭环；
3. 检查 LOS / Reflection / Transmission / Diffraction 四类路径级 EM 结果是否都 `valid=true`；
4. 检查 delay / phase / power 均非空且无明显异常值。

---

## A3阶段补充关注点

进入 A3 后，批次7中 transmission 的验证意义已经变化，应额外关注：

1. `EMTransmissionCheck` 不再只是“有 transmission interaction 就能算”，而是应建立在 transmission 语义完整、`medium_in / medium_out` 合法且已被模块5正式消费的基础上；
2. 对 A3 最小 transmission 主样例，若 `EMTransmissionCheck: valid=true`，应理解为模块5已经在正式消费可信 transmission 输入，而不是仅消费 interaction 标签；
3. 在当前配置禁用的机制（如 reflection / diffraction）被日志标记为 skipped，是当前样例口径的一部分，不应误判为主链失败；
4. 后续 A4 才负责 transmission 算得准不准，而 A3 当前只要求 transmission 输入语义链可信、可消费、无明显默认回退。
5. 当前 A3 最小 transmission 主样例建议优先使用 `configs/app/a3_transmission_minimal.json`，并优先检查 transmission 路径是否带出可追溯的 medium in/out 语义链。

---

## A4阶段补充关注点

进入 A4 后，批次7的验证意义进一步升级，应额外关注：

1. free-space 是否已经从占位衰减转为可解释传播基底；
2. reflection 是否对几何与法向更敏感；
3. transmission 是否显式消费介质语义并更新 medium 上下文；
4. diffraction 是否不再是统一固定缩放；
5. `EMPathResult` 中是否出现可辅助解释路径的字段：
   - `fs_loss_db`
   - `pol_mag`
   - `tx_semantic`
   - `medium_in / medium_out`

---

## A5阶段补充关注点

进入 A5 后，批次7应额外确认“模块3是否已正式进入模块5主链”：

1. `EMLosCheck` / `EMTransmissionCheck` 中应出现：
   - `tx_antenna`
   - `tx_source`
   - `rx_antenna`
   - `rx_source`
2. 当前最小闭环应至少体现：
   - Tx 已从正式 `AntennaModel` 初始化进入模块5
   - Rx 已具备最小正式语义入口
3. 当前正式支持的是 `Ideal` 天线最小闭环；
4. `pattern / polarization / orientation` 当前只要求：
   - 对象字段位置存在
   - 接口骨架位置存在
   - 不要求在 A5 内完成完整行为实现
5. 若路径级日志里仍完全看不出 `tx_source / rx_source`，则应认定 A5 尚未真正把模块3正式接进模块5。

---

## 当前批次限制

1. 当前模块5采用第一版简化自由空间段传播与交互系数；
2. Fresnel / UTD 严格模型将在后续继续增强；
3. 当前以路径级 `EMPathResult` 闭环为目标，CIR / PDP / APS 等汇总将在后续批次继续开发。

---

## A7阶段补充关注点

进入 A7 后，批次7在验证资产体系中的角色应明确为：

1. **机制级样例资产**，而不是长期主 baseline；
2. 它主要用于检查：
   - 单路径 EM 行为是否还成立；
   - transmission 语义消费是否还成立；
   - A5 的正式天线输入是否还可见；
3. 若 A7 后某次迭代出现：
   - `tx_source / rx_source` 消失；
   - `tx_semantic` 异常；
   - `medium_in / medium_out` 异常；
   则应优先视为“机制级退化样例告警”；
4. 批次7不应被当作完整 regression baseline 场景，而应作为 A7 体系中的**机制样例层**保留。

---

## A7资产分层定位

在 A7 的验证资产体系中，批次7的正式角色为：

1. **机制样例层**；
2. 负责说明单路径 / 单机制 EM 主链是否仍成立；
3. 负责对 transmission 语义消费、正式天线输入可见性做机制级核查；
4. 不承担主场景 regression baseline 的最终职责。

---

## A8收口补充

当前批次7只保留机制核查作用，不再作为正式主链交付入口；若需确认最终交付，请以批次9和 A8 快速接手视图为准。
