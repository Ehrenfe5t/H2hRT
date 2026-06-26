% ============================================================
% TDMS CIR 多径提取、可视化（含底噪与阈值线）及单帧总功率、小尺度参数计算
% 说明：
% 1) 读取 TDMS 中的 I/Q 数据，形成复数 CIR
% 2) 仅处理第一帧，计算 PDP(dBm)
% 3) 基于底噪与动态范围计算阈值：
%       Nth = max(Pmax - THRESH_DROP_DB, Pnoise + DR/3)
% 4) 检测超过阈值的局部峰值作为有效多径
% 5) 用户可通过 selected_path_indices 矩阵选择保留哪些多径 (0-based 索引)。
%    **注意**: 这些索引是**实际检测到的多径的索引**，而非简单的采样点索引。
% 6) 在第一帧图中加入：
%       - 黑色底噪线
%       - 红色阈值线
%       - **红色散点仅标出用户选定的多径 (并应用 X 轴平移)**
% 7) 累加用户选定的有效多径分量的功率，并在控制台输出该帧的总接收功率。
% 8) 计算并输出用户选定的多径的平均附加时延、均方根时延扩展、最大附加时延
% ============================================================

clear; clc; close all;
set(0, 'DefaultAxesFontName', 'Times New Roman');      % 设置坐标轴默认字体为宋体
set(0, 'DefaultTextFontName', 'Times New Roman');      % 设置文本默认字体为宋体
%% 1. 参数配置
filename      = "E:\hzh\usrp\代码\Tx1\2026_04_29_16_52_38_CIR.tdms";  % TDMS 文件名
output_csv    = 'Multipath_Report_Frame1.csv'; % 输出 CSV 文件名
FRAME_TO_SHOW = 1;                           % 指定用于处理和可视化的帧序号 (固定为 1)
ZC_Len        = 2048;                        % 每帧长度 (采样点数)
Fs            = 200e6;                       % 采样率 200 MHz
dt            = 1 / Fs;                      % 采样间隔 (秒)
Power_Comp      = 65;                          % 功率补偿值（dB），用于将幅度转换为dBm。

% 噪声估计参数
NOISE_TAIL_RATIO = 0.40;   % 用后 40% 时延区间估计底噪
THRESH_DROP_DB   = 35;     % Nth = max(Pmax - THRESH_DROP_DB, Pnoise + DR/3) 的固定衰减值

%% --- 多径选择参数 ---
% 您可以在这里设置您想要保留的多径的 0-based 索引。
% 这些索引对应于 **从数据中检测到的多径** 的索引。
% 例如，如果控制台输出“所有检测到的多径的 0-based 索引: 217 222 225”，
% 那么您可以选择 `selected_path_indices = [0, 1, 2];` 来分别绘制这三条（对应实际索引 217, 222, 225）。
%
% 设置策略：
% 1. 运行一次，查看控制台输出的 `all_detected_indices_0based`。
% 2. 根据这个列表，确定您感兴趣的 **检测到的多径的顺序索引** (0, 1, 2, ...)。
% 3. 设置 `selected_path_indices`。
%
% 示例：
% selected_path_indices = [0, 1, 2]; % 选择检测到的前三条多径 (对应实际索引 217, 222, 225)
% selected_path_indices = [0];       % 选择检测到的第一条多径 (对应实际索引 217)
% selected_path_indices = [];        % 保留所有检测到的多径 (默认行为)

selected_path_indices = []; % *** 默认: 选择所有检测到的多径 ***
% selected_path_indices = [0]; % *** 示例: 选择检测到的第一条多径 ***
% selected_path_indices = [0, 1, 2]; % *** 示例: 选择检测到的前三条多径 ***


%% --- 可视化控制参数 ---
PLOT_PATHS_SCATTER = true; % 是否绘制检测到的路径的散点图 (true/false)
DELAY_THRESHOLD_NS_FOR_PLOT = 0; % 绘制散点的延迟阈值 (ns)。设为 0 表示绘制所有选定的路径。

%% 依赖检查
fprintf('正在进行依赖检查...\n');
if ~exist('tdmsread', 'file')
    error('错误: 当前 MATLAB 未找到 tdmsread 函数。\n请确认已安装 NI TDMS Import Support Package 或相关工具箱。');
end
if ~exist(filename, 'file')
    error('错误: 未找到指定的 TDMS 文件: %s', filename);
end
fprintf('依赖检查通过。\n\n');

%% 2. 读取 TDMS 数据
fprintf('正在读取 TDMS 文件: %s ...\n', filename);

try
    rawData = tdmsread(filename);
catch ME
    error('tdmsread 调用失败: %s\n请检查文件格式或 tdmsread 函数的可用性。', ME.message);
end

raw_I = [];
raw_Q = [];

% --- 兼容 tdmsread 可能返回的不同数据格式 ---
if isnumeric(rawData)
    fprintf('识别到数据为数值矩阵格式。\n');
    if size(rawData, 2) == 1
        raw_I = double(rawData(:, 1));
        raw_Q = zeros(size(raw_I));
    else
        raw_I = double(rawData(:, 1));
        raw_Q = double(rawData(:, 2));
    end
elseif iscell(rawData)
    fprintf('识别到数据为 Cell 数组格式。\n');
    numeric_cells = {};
    tt_cells      = {};
    for k = 1:numel(rawData)
        v = rawData{k};
        if isnumeric(v) && isvector(v)
            numeric_cells{end+1} = v(:);
        elseif istimetable(v) || istable(v)
            vnames = v.Properties.VariableNames;
            if ~isempty(vnames)
                tt_cells{end+1} = v.(vnames{1})(:);
            end
        end
    end
    if ~isempty(numeric_cells)
        raw_I = double(numeric_cells{1});
        if numel(numeric_cells) >= 2
            raw_Q = double(numeric_cells{2});
        else
            raw_Q = zeros(size(raw_I));
        end
    elseif ~isempty(tt_cells)
        raw_I = double(tt_cells{1});
        if numel(tt_cells) >= 2
            raw_Q = double(tt_cells{2});
        else
            raw_Q = zeros(size(raw_I));
        end
    else
        error('tdmsread 返回的 cell 中未找到可用的数值通道数据 (I/Q)。');
    end
elseif istimetable(rawData) || istable(rawData)
    fprintf('识别到数据为 Timetable/Table 格式。\n');
    vnames = rawData.Properties.VariableNames;
    if isempty(vnames)
        error('Timetable/Table 中没有变量列，无法解析通道数据 (I/Q)。');
    end
    raw_I = double(rawData.(vnames{1})(:));
    if numel(vnames) >= 2
        raw_Q = double(rawData.(vnames{2})(:));
    else
        raw_Q = zeros(size(raw_I));
    end
else
    error('未知的 tdmsread 输出数据类型：%s', class(rawData));
end

% --- 保证 I/Q 长度一致 ---
len_data = min(length(raw_I), length(raw_Q));
if len_data == 0
    error('未成功解析到 I/Q 数据，请检查 TDMS 文件内容和读取逻辑。');
end
raw_I    = raw_I(1:len_data);
raw_Q    = raw_Q(1:len_data);

% --- 形成复数 CIR ---
CIR_Full = raw_I + 1j * raw_Q;
CIR_Full = CIR_Full(:); % 确保为列向量

Total_Len  = length(CIR_Full);
Num_Frames = floor(Total_Len / ZC_Len);
fprintf('数据加载完成。\n');
fprintf('  总采样点数: %d\n', Total_Len);
fprintf('  每帧采样点数 (ZC_Len): %d\n', ZC_Len);
fprintf('  计算得到的总帧数: %d\n', Num_Frames);

if Num_Frames < 1
    error('有效帧数为 0，请检查 ZC_Len 或 TDMS 数据长度是否匹配。');
end

% --- 检查 FRAME_TO_SHOW 合法性 ---
if FRAME_TO_SHOW < 1 || FRAME_TO_SHOW > Num_Frames
    warning('指定的 FRAME_TO_SHOW (%d) 超出有效帧范围 [1, %d]。将自动设置为 1。', FRAME_TO_SHOW, Num_Frames);
    FRAME_TO_SHOW = 1;
end
fprintf('将用于处理和可视化的帧序号设为: %d\n\n', FRAME_TO_SHOW);


%% --- 只处理指定帧 (FRAME_TO_SHOW) 的数据 ---
fprintf('--- 开始处理帧 %d ---\n', FRAME_TO_SHOW);

% --- 提取指定帧数据 ---
f = FRAME_TO_SHOW;
idx_start  = (f - 1) * ZC_Len + 1;
idx_end    = idx_start + ZC_Len - 1;
frame_data = CIR_Full(idx_start : idx_end); % 当前帧的复数 CIR

% --- 计算 CIR 幅度并转换为 dBm ---
% ** 警告：请务必确认 Power_Comp 的具体含义和使用方式。**
pdp_data_dB = 20 * log10(abs(frame_data) + eps) - Power_Comp;

% --- 底噪与阈值计算 ---
tail_start_idx = max(1, floor((1 - NOISE_TAIL_RATIO) * ZC_Len) + 1);
noise_region_dB = pdp_data_dB(tail_start_idx:end);
Pnoise_dB = mean(noise_region_dB, 'omitnan');
Pmax_dB   = max(pdp_data_dB);
DR_dB     = Pmax_dB - Pnoise_dB;
Nth_dB    = max(Pmax_dB - THRESH_DROP_DB, Pnoise_dB + DR_dB / 3);

% --- 多径检测 ---
% detect_paths_by_threshold 返回的是 0-based 索引
[all_detected_indices_0based, Pnoise_dB, Nth_dB, DR_dB] = detect_paths_by_threshold(pdp_data_dB, [], [], [], THRESH_DROP_DB, NOISE_TAIL_RATIO);

% --- 打印所有检测到的多径索引，方便用户选择 ---
fprintf('--- 多径检测结果 ---\n');
fprintf('共检测到 %d 条有效多径分量。\n', length(all_detected_indices_0based));
if ~isempty(all_detected_indices_0based)
    fprintf('  所有检测到的多径的 0-based 索引: ');
    fprintf('%d ', all_detected_indices_0based);
    fprintf('\n');
end
fprintf('--------------------\n\n');

% --- 确定最终要使用的多径索引 ---
% 我们需要一个列表，表示用户想要选择的 **多径的顺序索引** (0, 1, 2, ...)。
% 并且这个列表要对应于 `all_detected_indices_0based`。

final_indices_to_use = []; % 最终用于计算和绘制的多径索引 (基于用户选择的顺序索引)

if isempty(selected_path_indices)
    % 用户未指定，则使用所有检测到的多径
    final_indices_to_use = all_detected_indices_0based;
    fprintf('用户未指定保留哪些多径，将使用所有检测到的 %d 条多径。\n', length(all_detected_indices_0based));
else
    % 用户指定了，检查用户指定的 **顺序索引** 是否有效
    num_detected_paths = length(all_detected_indices_0based);
    
    % 筛选出用户指定且在有效范围内 (0 到 num_detected_paths - 1) 的索引
    valid_order_indices_mask = selected_path_indices >= 0 & selected_path_indices < num_detected_paths;
    valid_order_indices = selected_path_indices(valid_order_indices_mask);

    if length(valid_order_indices) < length(selected_path_indices)
        num_invalid = length(selected_path_indices) - length(valid_order_indices);
        fprintf('警告: 用户指定的 %d 个多径顺序索引中有 %d 个超出了检测到的多径范围 [0, %d]，将被忽略。\n', ...
                length(selected_path_indices), num_invalid, num_detected_paths - 1);
    end
    
    % 根据有效的顺序索引，获取实际的多径采样点索引
    final_indices_to_use = all_detected_indices_0based(valid_order_indices + 1); % +1 是因为 MATLAB 索引是 1-based

    fprintf('用户指定保留 %d 条多径 (基于检测到的顺序索引)。\n', length(final_indices_to_use));
end


% --- 存储提取的有效路径信息 ---
valid_paths = []; % 存储结构体，包含 'delay_s', 'power_mW', 'power_dBm', 'index_0based' (实际采样点索引)
Total_Linear_Power_mW_Frame = 0; % 累加当前帧的所有有效路径的线性功率 (mW)
Total_Path_Count_Frame = 0;      % 仅计算当前帧的路径数

if ~isempty(final_indices_to_use)
    Total_Path_Count_Frame = length(final_indices_to_use);
    fprintf('该帧 (%d) 最终选定的有效多径分量数量: %d\n', f, Total_Path_Count_Frame);

    % --- 提取时延和功率 ---
    for k = 1 : Total_Path_Count_Frame
        % idx_0based 是实际的采样点索引 (e.g., 217, 222, ...)
        idx_0based = final_indices_to_use(k); 
        
        % idx_1based 用于 MATLAB 数组访问
        idx_1based = idx_0based + 1;          

        % --- 计算时延 (秒) ---
        current_delay_s = idx_0based * dt;

        % --- 获取当前路径的功率 (dBm) ---
        current_power_dBm = pdp_data_dB(idx_1based);

        % --- 将 dBm 转换为线性功率 (mW) ---
        current_power_mW = 10^(current_power_dBm / 10) / 1000;

        % --- 累加总功率 (mW) ---
        Total_Linear_Power_mW_Frame = Total_Linear_Power_mW_Frame + current_power_mW;

        % --- 存储当前路径信息 ---
        valid_paths(k).index_0based = idx_0based; % 存储实际采样点索引
        valid_paths(k).delay_s      = current_delay_s;
        valid_paths(k).power_dBm    = current_power_dBm;
        valid_paths(k).power_mW     = current_power_mW;
    end

    % --- 计算并输出该帧的总功率 (dBm) ---
    fprintf('--- 单帧总功率计算 ---\n');
    if Total_Path_Count_Frame > 0
        Total_Received_Power_dBm_Frame = 10 * log10(Total_Linear_Power_mW_Frame * 1000); % 转换为 dBm
        fprintf('该帧内所有选定多径分量的总接收功率 (dBm): %.4f\n', Total_Received_Power_dBm_Frame);
    else
        Total_Received_Power_dBm_Frame = -inf; % 没有路径，功率为负无穷
        fprintf('该帧 (%d) 未检测到任何有效多径分量（或未选中任何多径）。\n', f);
    end
else
    fprintf('该帧 (%d) 未检测到任何有效多径分量（或未选中任何多径）。\n', f);
end
fprintf('----------------------\n\n');


%% --- 小尺度参数计算 ---
fprintf('--- 小尺度参数计算 ---\n');

tau_bar_s = 0;       % 平均附加时延 (秒) - 基于新时延轴
tau_rms_s = 0;       % 均方根时延扩展 (秒) - 基于新时延轴
tau_max_s = 0;       % 最大附加时延 (秒) - 基于新时延轴

if Total_Path_Count_Frame > 0
    % --- 准备计算参数所需的数据 ---
    
    % --- 获取选定的多径的实际采样点索引 ---
    % final_indices_to_use 包含的是实际的采样点索引 (e.g., 217, 222, ...)
    delay_ns_axis = (0:ZC_Len-1) * dt * 1e9;
    % --- 1. 获取这些多径在原始时延轴 (ns) 上的值 ---
    original_delays_ns = delay_ns_axis(final_indices_to_use + 1);
    
    % --- 2. 计算这些多径在“平移+缩放后”的时延轴上的值 ---
    % 这是您想要作为“真正时延”的值
    transformed_delays_ns = (original_delays_ns - 1110-225) / 1; 
    
    % --- 3. 准备计算功率 ---
    powers_mW = [valid_paths.power_mW];    % 所有有效路径的线性功率 (mW)

    % --- 显式进行归一化 (可选，但使逻辑更清晰) ---
    total_power_sum_mW = sum(powers_mW);
    
    if total_power_sum_mW <= 0
        % 处理极端情况，避免除以零
        tau_bar_s = 0;
        tau_rms_s = 0;
        tau_max_s = 0;
    else
        normalized_powers = powers_mW / total_power_sum_mW; % 概率 p(τᵢ)

        % --- 1. 计算平均附加时延 (tau_bar) ---
        % 使用“平移+缩放后”的时延值进行计算
        tau_bar_s_ns = sum(normalized_powers .* transformed_delays_ns); 

        % --- 2. 计算均方根时延扩展 (tau_rms) ---
        % 使用“平移+缩放后”的时延值进行计算
        squared_delay_diff = (transformed_delays_ns - tau_bar_s_ns).^2;
        tau_rms_s_ns = sqrt(sum(normalized_powers .* squared_delay_diff));

        % --- 3. 计算最大附加时延 (tau_max) ---
        % 注意: tau_max 是最大时延与最小（最早）时延的差值。
        % 如果我们用“平移+缩放后”的时延值来计算，结果会受到平移的影响。
        % 如果您希望 tau_max 反映的是 **原始时延轴上** 的最大扩散，
        % 则应该使用 original_delays_ns 来计算。
        % 如果您希望 tau_max 反映的是 **新时延轴上** 的扩散范围，则使用 transformed_delays_ns。
        %
        % **这里我假设您希望 tau_max 也基于新的时延轴**
        [sorted_transformed_delays_ns, sort_idx] = sort(transformed_delays_ns);
        tau_first_transformed_ns = sorted_transformed_delays_ns(1);
        tau_last_transformed_ns  = sorted_transformed_delays_ns(end);
        tau_max_s_ns = tau_last_transformed_ns - tau_first_transformed_ns;
        
        % --- 将计算结果（ns）转换回秒（s）以统一单位 ---
        tau_bar_s = tau_bar_s_ns / 1e9;
        tau_rms_s = tau_rms_s_ns / 1e9;
        tau_max_s = tau_max_s_ns / 1e9;

        % --- 如果您希望 tau_max 反映的是 **原始时延轴上** 的最大扩散 ---
        % (即，不考虑平移，只看相对时间差)
        % 则计算方式应为:
        % tau_max_s_orig_axis = max(original_delays_ns) - min(original_delays_ns);
        % tau_max_s_ns_orig_axis = tau_max_s_orig_axis;
        % tau_max_s = tau_max_s_ns_orig_axis / 1e9; % 转换为秒
        % --- 请根据您的实际需求选择上述两种 tau_max 计算方式 ---

    end

    % --- 输出小尺度参数 ---
    fprintf('平均附加时延 (tau_bar, 基于新时延轴): %.4f ns\n', tau_bar_s * 1e9);
    fprintf('均方根时延扩展 (tau_rms, 基于新时延轴): %.4f ns\n', tau_rms_s * 1e9);
    fprintf('最大附加时延 (tau_max, 基于新时延轴): %.4f ns\n', tau_max_s * 1e9);

else
    fprintf('未检测到有效多径分量（或未选中任何多径），无法计算小尺度参数。\n');
end


% --- 数据收集 (用于 CSV 导出) ---
All_Metrics_Data_Frame1 = [];
if ~isempty(valid_paths)
    VarNames = {'Frame', 'Index_0Based', 'Delay_ns', 'Power_dBm', 'FadingDiff_dB_from_FrameMax', ...
                'Phase_rad', 'PhaseDiff_rad_from_FrameMain', 'NoiseFloor_dBm_thisFrame', 'Threshold_dBm_thisFrame', ...
                'TauBar_ns', 'TauRms_ns', 'TauMax_ns'};
    num_paths = length(valid_paths);
    row_template = [ ...
        f, ...
        0, ...
        0, ...
        0, ...
        0, ...
        0, ...
        0, ...
        Pnoise_dB, ...
        Nth_dB, ...
        tau_bar_s * 1e9, ... % 平均附加时延 (ns)
        tau_rms_s * 1e9, ... % 均方根时延扩展 (ns)
        tau_max_s * 1e9];    % 最大附加时延 (ns)

    All_Metrics_Data_Frame1 = repmat(row_template, num_paths, 1);

    for k = 1 : num_paths
        All_Metrics_Data_Frame1(k, 2) = valid_paths(k).index_0based; % Index_0Based (实际采样点索引)
        All_Metrics_Data_Frame1(k, 3) = valid_paths(k).delay_s * 1e9; % Delay_ns
        All_Metrics_Data_Frame1(k, 4) = valid_paths(k).power_dBm;    % Power_dBm
        max_pwr_dBm_in_frame = max([valid_paths.power_dBm]);
        All_Metrics_Data_Frame1(k, 5) = max_pwr_dBm_in_frame - valid_paths(k).power_dBm;

        All_Metrics_Data_Frame1(k, 6) = 0; % Placeholder for Phase_rad
        All_Metrics_Data_Frame1(k, 7) = 0; % Placeholder for PhaseDiff_rad_from_FrameMain
    end

    fprintf('第一帧分析完成。\n\n');

    fprintf('正在导出 CSV 报告: %s ...\n', output_csv);
    if exist(output_csv, 'file')
        try
            delete(output_csv);
            fprintf('  检测到旧文件，已删除。\n');
        catch ME
            error('无法删除旧 CSV 文件 "%s"。\n请检查该文件是否被其他程序（如 Excel）占用，关闭后重试。\n详细错误: %s', ...
                output_csv, ME.message);
        end
    end
    T_Export = array2table(All_Metrics_Data_Frame1, 'VariableNames', VarNames);
    try
        writetable(T_Export, output_csv);
        fprintf('SUCCESS: CSV 报告导出成功。\n\n');
    catch ME
        error('写入 CSV 文件 "%s" 失败。\n详细错误: %s', output_csv, ME.message);
    end
else
    fprintf('警告: 未检测到任何有效多径分量（或未选中任何多径），跳过 CSV 导出。\n\n');
end


%% 5. 可视化 (仅针对 FRAME_TO_SHOW)
fprintf('正在生成单帧 PDP 可视化图 (Frame %d)...\n', FRAME_TO_SHOW);
figure('Name', sprintf('Frame %d PDP & Multipath Analysis', FRAME_TO_SHOW), 'Color', 'w');

pdp_sel       = pdp_data_dB;
noise_sel_dB  = Pnoise_dB;
thresh_sel_dB = Nth_dB;

% X 轴的原始时延 (ns)
delay_ns_axis = (0:ZC_Len-1) * dt * 1e9;

% --- 绘制 PDP 曲线 ---
% 应用 X 轴的平移和缩放
x_axis_transformed = (delay_ns_axis - 1110-225) / 1;
plot(x_axis_transformed, pdp_sel, 'b', 'LineWidth', 0.8, 'DisplayName', 'PDP Curve');
hold on; grid on;

title(sprintf('Frame %d PDP, Noise Floor, Threshold & Selected Paths', FRAME_TO_SHOW));
xlabel('Delay (ns)', 'FontSize', 12, 'FontName', 'Times New Roman');
ylabel('Power (dBm)', 'FontSize', 12, 'FontName', 'Times New Roman');
xlim([0, 3000]);
ylim([-180, -60]);

% --- 修改图例名称 ---
h_noise = yline(noise_sel_dB, 'k-', 'LineWidth', 1.5, ...
    'Label', sprintf(' '), ...
    'LabelHorizontalAlignment', 'left', ...
    'LabelVerticalAlignment', 'bottom', ...
    'DisplayName', 'Noise Floor = -128.79dBm'); % *** 添加 DisplayName ***

h_thresh = yline(thresh_sel_dB, 'r-', 'LineWidth', 1.5, ...
    'Label', sprintf(' '), ...
    'LabelHorizontalAlignment', 'left', ...
    'LabelVerticalAlignment', 'bottom', ...
    'DisplayName', 'Threshold = -109.06dBm'); % *** 添加 DisplayName ***

% --- 绘制用户选定的多径散点图 ---
if PLOT_PATHS_SCATTER && ~isempty(final_indices_to_use) % 检查是否需要绘制且有选定的多径
    % --- 获取选定路径的实际时延 (ns) ---
    % final_indices_to_use 包含的是实际的采样点索引 (e.g., 217, 222, ...)
    selected_delays_ns = delay_ns_axis(final_indices_to_use + 1);

    % --- 应用与 PDP 曲线相同的 X 轴变换 ---
    plot_x_transformed_ns = (selected_delays_ns - 1110-225) / 1;

    % --- 筛选出实际延迟大于等于阈值的路径 ---
    valid_selection_mask_for_plotting = selected_delays_ns >= DELAY_THRESHOLD_NS_FOR_PLOT;

    % --- 筛选出需要绘制的 X 坐标和 Y 坐标 ---
    indices_to_plot_transformed_ns = plot_x_transformed_ns(valid_selection_mask_for_plotting);
    plot_y_dBm = pdp_sel(final_indices_to_use(valid_selection_mask_for_plotting) + 1); % Y 值 (dBm)

    % --- 绘制散点图 ---
    if ~isempty(indices_to_plot_transformed_ns)
        if length(indices_to_plot_transformed_ns) == length(plot_y_dBm)
            scatter(indices_to_plot_transformed_ns, plot_y_dBm, ...
                60, 'r', 'filled', 'DisplayName', 'Detected Paths');
            legend('Location', 'best');
        else
            warning('scatter plot X and Y coordinates have mismatched lengths.');
            legend('Location', 'best');
        end
    else
        % 如果没有点满足延迟阈值
        legend('Location', 'best');
        fprintf('没有选定路径的实际延迟值 (selected_delays_ns) 大于或等于 %.2f ns，因此未绘制散点。\n', DELAY_THRESHOLD_NS_FOR_PLOT);
    end
else
    % 如果不绘制散点，或者没有选定/检测到的多径
    if ~isempty(final_indices_to_use)
        fprintf('PLOT_PATHS_SCATTER 设置为 false，不绘制散点图。\n');
    else
        fprintf('没有检测到有效多径分量，或用户未选择任何多径，因此不绘制散点。\n');
    end
    legend('Location', 'best');
end

hold off;
fprintf('单帧 PDP 图已生成。\n\n');


%% ========================================================================
%  内部函数定义 (保持不变)
% ========================================================================
function [final_indices, Pnoise_dB, Nth_dB, DR_dB] = detect_paths_by_threshold(pdp_dB, Pnoise_dB_in, Pmax_dB_in, DR_dB_in, THRESH_DROP_DB, NOISE_TAIL_RATIO)
    % ----------------------------------------------------------------------
    % 功能: 根据底噪和动态范围计算阈值，并寻找超过阈值的局部峰值作为多径。
    % 输出:
    %   final_indices: 检测到的有效多径的 0-based 索引 (这些索引是实际的采样点索引)。
    % ----------------------------------------------------------------------
    pdp_dB = pdp_dB(:); % 确保 pdp_dB 是列向量
    len = length(pdp_dB);

    if isempty(Pnoise_dB_in)
        tail_start_idx = max(1, floor((1 - NOISE_TAIL_RATIO) * len) + 1);
        noise_region_dB = pdp_dB(tail_start_idx:end); 
        Pnoise_dB = mean(noise_region_dB, 'omitnan');
    else
        Pnoise_dB = Pnoise_dB_in;
    end

    if isempty(Pmax_dB_in)
        Pmax_dB   = max(pdp_dB);
    else
        Pmax_dB = Pmax_dB_in;
    end

    if isempty(DR_dB_in)
        DR_dB     = Pmax_dB - Pnoise_dB;
    else
        DR_dB = DR_dB_in;
    end

    Nth_dB    = max(Pmax_dB - THRESH_DROP_DB, Pnoise_dB + DR_dB / 3);

    collected_indices = [];
    for k = 1:len
        val = pdp_dB(k);
        prev_val = -inf;
        if k > 1
            prev_val = pdp_dB(k-1);
        end
        next_val = -inf;
        if k < len
            next_val = pdp_dB(k+1);
        end
        is_local_peak = (val >= prev_val) && (val > next_val);
        if is_local_peak && (val >= Nth_dB)
            collected_indices(end+1) = k - 1; % 记录的是 0-based 采样点索引
        end
    end
    final_indices = unique(sort(collected_indices));
end