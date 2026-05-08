clear
clc

% 读取功率数据
filename = '1.5-10-1\frequency[4000].MHz.csv';
data = readtable(filename);

% 提取接收天线位置和功率数据
rx_x = data.Var6;
rx_y = data.Var7;
power_dBm = data.Var9;

% 发射天线位置（从数据中获取）
tx_x = 6;
tx_y = 10;
tx_z = 0.4;

% 创建功率覆盖网格数据
unique_x = unique(rx_x);
unique_y = unique(rx_y);
num_x = length(unique_x);
num_y = length(unique_y);

% 重塑功率数据为网格格式
Z_power = reshape(power_dBm, num_y, num_x);

% 读取场景点数据
points_filename = 'ScenarioAcceleratePoint3D.csv'; % 您的点数据文件
points_data = readtable(points_filename);

% 提取点坐标
points = [points_data.x, points_data.y, points_data.z];

% 读取三角面元数据
triangles_filename = 'ScenarioAccelerateTriangle3D.csv'; % 您的三角面元数据文件
triangles_data = readtable(triangles_filename);

% 提取三角形索引（注意：索引从0开始，MATLAB需要从1开始）
% 根据您的数据格式，三角形索引列名可能是triangleP1Index, triangleP2Index, triangleP3Index
triangles = [triangles_data.triangleP1Index + 1, ...
             triangles_data.triangleP2Index + 1, ...
             triangles_data.triangleP3Index + 1];

% 3D可视化：场景几何 + 功率覆盖平面
figure('Position', [50, 50, 1400, 800]);
hold on;

% 1. 绘制场景几何（三角面元）
% 使用patch函数绘制三角形
% 1. 识别并删除顶部三角面元（天花板）
max_z = max(points(:, 3));  % 场景的最大高度
z_threshold = max_z - 0.05;  % 设置阈值，接近最大高度的面为顶部

% 计算每个三角面元的平均Z坐标
non_top_indices = [];  % 存储非顶部面的索引

for i = 1:size(triangles, 1)
    tri_vertices = points(triangles(i, :), :);
    avg_z = mean(tri_vertices(:, 3));
    
    % 如果平均Z坐标不接近最大高度，则保留这个面
    if avg_z <= z_threshold
        non_top_indices = [non_top_indices; i];
    end
end

% 只保留非顶部三角面元
non_top_triangles = triangles(non_top_indices, :);

% 绘制场景几何（只绘制非顶部三角面元）
patch('Vertices', points, 'Faces', non_top_triangles, ...
    'FaceColor', [0.7, 0.7, 0.7], ...  % 灰色
    'EdgeColor', [0.4, 0.4, 0.4], ...  % 深灰色边线
    'FaceAlpha', 0.8, ...  % 半透明，可以看到后面的功率图
    'EdgeAlpha', 0.6, ...
    'LineWidth', 0.5, ...
    'DisplayName', '场景');

% 2. 绘制功率覆盖平面（在z=1高度，即接收天线高度）
% 创建网格
[X_grid, Y_grid] = meshgrid(unique_x, unique_y);
Z_plane = ones(size(X_grid)) * tx_z; % 在接收天线高度

% 绘制功率覆盖曲面
surf_handle = surf(X_grid, Y_grid, Z_plane, Z_power, ...
    'FaceAlpha', 0.5, ...  % 较高的透明度，可以看到下面的场景
    'EdgeColor', 'none', ...  % 不显示边线
    'FaceLighting', 'gouraud', ...
    'DisplayName', '功率覆盖');

% 标记发射天线位置
plot3(tx_x, tx_y, tx_z, 'r^', ...
    'MarkerSize', 15, ...
    'MarkerFaceColor', 'r', ...
    'LineWidth', 2, ...
    'DisplayName', '发射机');

% 添加光照效果
light('Position', [0, 0, 10], 'Style', 'infinite');
light('Position', [10, 10, 5], 'Style', 'infinite');
lighting gouraud;

% 设置颜色映射
colormap('jet');
colorbar_handle = colorbar;
ylabel(colorbar_handle, 'Power (dBm)', 'FontSize', 11);


% 7. 设置坐标轴和标签
xlabel('X Position (m)', 'FontSize', 12, 'FontWeight', 'bold');
ylabel('Y Position (m)', 'FontSize', 12, 'FontWeight', 'bold');
zlabel('Z Position (m)', 'FontSize', 12, 'FontWeight', 'bold');


% 设置图例
legend('Location', 'best', 'FontSize', 10);

% 10. 设置图形属性
axis equal;
grid on;
box on;

% 设置坐标轴范围，确保显示所有场景和功率数据
xlim([min([points(:,1); rx_x]), max([points(:,1); rx_x])]);
ylim([min([points(:,2); rx_y]), max([points(:,2); rx_y])]);
zlim([min(points(:,3)), max([points(:,3); tx_z+1])]);

view(45, 45);  % 方位角45度，仰角30度

% 保存3D视图
print(gcf, 'power_coverage_3d_view[0].png', '-dpng', '-r600');
