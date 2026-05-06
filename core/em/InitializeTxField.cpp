// 文件目标：
// - 实现模块5批次7的发射场初始化逻辑。
//
// 主要功能：
// - 设定波长、初始复振幅和默认极化占位；
// - 记录当前频率和基础介质状态；
// - 为路径级电磁主链提供统一初值。

#include "InitializeTxField.h"

#include "../antenna/AntennaFactory.h"

namespace {

rt::Point3 ResolveTxPosition(const rt::GeometricPath& path)
{
    if (!path.nodes.empty())
    {
        return path.nodes.front().point;
    }
    return rt::Point3{};
}

rt::Point3 ResolveRxPosition(const rt::GeometricPath& path)
{
    if (!path.nodes.empty())
    {
        return path.nodes.back().point;
    }
    return rt::Point3{};
}

} // namespace

namespace rt {

/// <summary>
/// 初始化路径级发射场状态。
/// </summary>
/// <param name="input">电磁求解输入。</param>
/// <param name="field">待写入的场状态累积器。</param>
/// <returns>true 表示初始化成功；false 表示失败。</returns>
bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field)
{
    if (input.config == nullptr || input.path == nullptr)
    {
        return false;
    }

    const AntennaModel fallbackTx = BuildTxAntennaModel(*input.config, ResolveTxPosition(*input.path), "tx-ideal-default");
    const AntennaModel fallbackRx = BuildRxAntennaModel(*input.config, ResolveRxPosition(*input.path), "rx-ideal-default");
    const AntennaModel& txAntenna = (input.tx_antenna != nullptr) ? *input.tx_antenna : fallbackTx;
    const AntennaModel& rxAntenna = (input.rx_antenna != nullptr) ? *input.rx_antenna : fallbackRx;

    field.frequency_hz = input.config->em_solver.frequency_hz;
    if (field.frequency_hz <= 0.0)
    {
        return false;
    }
    field.wavelength_m = 299792458.0 / field.frequency_hz;
    field.total_length_m = 0.0;
    field.delay_s = 0.0;
    field.phase_rad = 0.0;
    field.amplitude_real = 1.0;
    field.amplitude_imag = 0.0;
    field.power_linear = 1.0;
    field.free_space_amplitude_scale = 1.0;
    field.free_space_power_scale = 1.0;
    field.last_segment_length_m = 0.0;
    field.tx_antenna_id = txAntenna.antenna_id;
    field.tx_antenna_source_type = txAntenna.source_type;
    field.rx_antenna_id = rxAntenna.antenna_id;
    field.rx_antenna_source_type = rxAntenna.source_type;
    field.current_medium_id = 0;
    field.last_transmission_medium_in_id = -1;
    field.last_transmission_medium_out_id = -1;
    field.transmission_semantic_consumed = false;
    field.polarization_vector = txAntenna.polarization_vector;
    field.valid = true;
    return true;
}

} // namespace rt
