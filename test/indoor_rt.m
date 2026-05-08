%% indoor_rt.m — 自实现RT 3D场景 + 功率覆盖切片
% 读取sbr_coverage.mat, 渲染meeting.obj 3D场景, 交互选择平面显示功率覆盖
% 用法: 先运行 python test/export_for_matlab.py 生成 .mat 文件
%       然后在MATLAB中: indoor_rt
clear; close all;

%% 加载数据
load('output/matlab/sbr_coverage.mat');
fprintf('Loaded: %d Rx (%d active), %d verts, %d faces\n', ...
    n_total, n_active, size(obj_vertices,1), size(obj_faces,1));

%% ========== 用户参数 (在此修改) ==========
slice_plane = 'Y';     % 'X', 'Y', 或 'Z'
slice_value = 1.5;     % 切片位置 (m)
show_ceiling = false;  % 是否显示天花板
opacity_walls = 0.85;  % 墙壁不透明度
opacity_cover = 0.55;  % 覆盖平面不透明度
%% =========================================

%% 1. 提取切片平面功率数据
if strcmp(slice_plane, 'Y')
    mask = abs(rx_y - slice_value) < 0.06;
    plane_label = sprintf('Y = %.2f m', slice_value);
    cx = rx_x(mask); cy = rx_z(mask);
    xl = 'X (m)'; yl = 'Z (m)';
    plane_normal = [0 1 0]; plane_pos = [0 slice_value 0];
elseif strcmp(slice_plane, 'Z')
    mask = abs(rx_z - slice_value) < 0.06;
    plane_label = sprintf('Z = %.2f m', slice_value);
    cx = rx_x(mask); cy = rx_y(mask);
    xl = 'X (m)'; yl = 'Y (m)';
    plane_normal = [0 0 1]; plane_pos = [0 0 slice_value];
else % X
    mask = abs(rx_x - slice_value) < 0.06;
    plane_label = sprintf('X = %.2f m', slice_value);
    cx = rx_y(mask); cy = rx_z(mask);
    xl = 'Y (m)'; yl = 'Z (m)';
    plane_normal = [1 0 0]; plane_pos = [slice_value 0 0];
end

pwr_slice = power_dBm(mask);
hit_slice = ray_hits(mask);
active_mask = hit_slice > 0;

fprintf('Slice %s: %d Rx, %d active\n', plane_label, sum(mask), sum(active_mask));

% 插值网格
if sum(active_mask) >= 3
    xi = linspace(min(cx), max(cx), round((max(cx)-min(cx))/0.02)*3);
    yi = linspace(min(cy), max(cy), round((max(cy)-min(cy))/0.02)*3);
    [XI, YI] = meshgrid(xi, yi);
    ZI = griddata(cx(active_mask), cy(active_mask), pwr_slice(active_mask), XI, YI, 'cubic');
    ZI_near = griddata(cx(active_mask), cy(active_mask), pwr_slice(active_mask), XI, YI, 'nearest');
    ZI(isnan(ZI)) = ZI_near(isnan(ZI));
    min_pwr = min(pwr_slice(active_mask));
    ZI(isnan(ZI)) = min_pwr - 3;
end

%% 2. 3D场景渲染
fig = figure('Position', [50 50 1600 850], 'Name', 'RT Power Coverage', 'NumberTitle', 'off');

% --- 左子图: 3D视图 ---
ax3d = subplot(1, 2, 1);
hold(ax3d, 'on');

% 去天花板
if ~show_ceiling
    tri_centers_z = zeros(size(obj_faces,1), 1);
    for i = 1:size(obj_faces,1)
        tri_centers_z(i) = mean(obj_vertices(obj_faces(i,:)+1, 3));
    end
    keep_faces = tri_centers_z < max(obj_vertices(:,3)) - 0.05;
    faces_to_draw = obj_faces(keep_faces, :) + 1;
else
    faces_to_draw = obj_faces + 1;
end

patch(ax3d, 'Vertices', obj_vertices, 'Faces', faces_to_draw, ...
    'FaceColor', [0.65 0.65 0.65], 'EdgeColor', [0.35 0.35 0.35], ...
    'FaceAlpha', 0.75, 'EdgeAlpha', 0.4, 'LineWidth', 0.3);

% 功率覆盖平面 (3D)
if exist('XI','var') && sum(active_mask) >= 3
    if strcmp(slice_plane, 'Y')
        [PX, PZ] = meshgrid(xi, yi);
        PY = ones(size(PX)) * slice_value;
        surf(ax3d, PX, PY, PZ, ZI, 'FaceAlpha', opacity_cover, ...
            'EdgeColor', 'none', 'FaceLighting', 'gouraud');
    elseif strcmp(slice_plane, 'Z')
        [PX, PY] = meshgrid(xi, yi);
        PZ = ones(size(PX)) * slice_value;
        surf(ax3d, PX, PY, PZ, ZI, 'FaceAlpha', opacity_cover, ...
            'EdgeColor', 'none', 'FaceLighting', 'gouraud');
    else
        [PY, PZ] = meshgrid(xi, yi);
        PX = ones(size(PY)) * slice_value;
        surf(ax3d, PX, PY, PZ, ZI, 'FaceAlpha', opacity_cover, ...
            'EdgeColor', 'none', 'FaceLighting', 'gouraud');
    end
end

% Tx/Rx
plot3(ax3d, tx_pos(1), tx_pos(2), tx_pos(3), 'p', ...
    'MarkerSize', 18, 'MarkerEdgeColor', 'k', 'MarkerFaceColor', 'w', 'LineWidth', 2);
plot3(ax3d, rx_pos(1), rx_pos(2), rx_pos(3), 'o', ...
    'MarkerSize', 12, 'MarkerEdgeColor', 'k', 'MarkerFaceColor', [0 1 0.5], 'LineWidth', 1.5);

% 光照
light(ax3d, 'Position', [20 5 5], 'Style', 'infinite');
light(ax3d, 'Position', [-5 10 -5], 'Style', 'infinite');
lighting(ax3d, 'gouraud');
material(ax3d, 'dull');

xlabel(ax3d, 'X (m)'); ylabel(ax3d, 'Y (m)'); zlabel(ax3d, 'Z (m)');
title(ax3d, ['3D Scene + Power Slice at ' plane_label], 'FontSize', 13, 'FontWeight', 'bold');
axis(ax3d, 'equal'); grid(ax3d, 'on'); box(ax3d, 'on');
view(ax3d, 50, 25);
colormap(ax3d, 'jet');
caxis(ax3d, [min_pwr-3 max(pwr_slice(active_mask))]);
cb3 = colorbar(ax3d); cb3.Label.String = 'Power (dBm)';

% --- 右子图: 2D切片热力 ---
ax2d = subplot(1, 2, 2);
hold(ax2d, 'on');

if exist('XI','var') && sum(active_mask) >= 3
    pcolor(ax2d, XI, YI, ZI);
    shading(ax2d, 'interp');
    colormap(ax2d, 'jet');
    caxis(ax2d, [min_pwr-3 max(pwr_slice(active_mask))]);
end

% 建筑轮廓
tol = 0.08;
ax_idx = find(strcmp(slice_plane, {'X','Y','Z'}));
oa = setdiff([1 2 3], ax_idx);
for fi = 1:size(obj_faces,1)
    f = obj_faces(fi,:) + 1;
    v = obj_vertices(f,:);
    vals = v(:, ax_idx);
    crossings = [];
    for i = 1:3
        j = mod(i,3)+1;
        if (vals(i)-slice_value)*(vals(j)-slice_value) < 0 && abs(vals(i)-vals(j))>1e-9
            t = (slice_value-vals(i))/(vals(j)-vals(i));
            if t>=0 && t<=1
                crossings = [crossings; v(i,oa(1))+t*(v(j,oa(1))-v(i,oa(1))), ...
                                        v(i,oa(2))+t*(v(j,oa(2))-v(i,oa(2)))];
            end
        elseif abs(vals(i)-slice_value) < tol
            crossings = [crossings; v(i,oa(1)) v(i,oa(2))];
        end
    end
    if size(crossings,1) >= 3
        keep = true(size(crossings,1),1);
        for ii = 1:size(crossings,1)
            if keep(ii)
                for jj = ii+1:size(crossings,1)
                    if keep(jj) && norm(crossings(ii,:)-crossings(jj,:)) < tol
                        keep(jj) = false;
                    end
                end
            end
        end
        uq = crossings(keep,:);
        if size(uq,1) >= 3
            cx_v = mean(uq(:,1)); cy_v = mean(uq(:,2));
            [~, ord] = sort(atan2(uq(:,2)-cy_v, uq(:,1)-cx_v));
            uq = uq(ord,:);
            fill(ax2d, uq(:,1), uq(:,2), [0.23 0.23 0.23], ...
                'EdgeColor', [0.1 0.1 0.1], 'LineWidth', 1.5, 'FaceAlpha', 0.85);
        end
    end
end

plot(ax2d, tx_pos(oa(1)), tx_pos(oa(2)), 'p', 'MarkerSize', 16, ...
    'MarkerEdgeColor', 'k', 'MarkerFaceColor', 'w', 'LineWidth', 2);
plot(ax2d, rx_pos(oa(1)), rx_pos(oa(2)), 'o', 'MarkerSize', 10, ...
    'MarkerEdgeColor', 'k', 'MarkerFaceColor', [0 1 0.5], 'LineWidth', 1.5);

xlabel(ax2d, xl); ylabel(ax2d, yl);
title(ax2d, ['Power Coverage — ' plane_label], 'FontSize', 13, 'FontWeight', 'bold');
axis(ax2d, 'equal tight');
cb2 = colorbar(ax2d); cb2.Label.String = 'Power (dBm)';
set(ax2d, 'FontSize', 11);

% 统计
med_pwr = median(pwr_slice(active_mask));
annotation('textbox', [0.015 0.96 0.48 0.035], 'String', ...
    sprintf('Active: %d/%d (%d%%) | Median: %.1f dBm | Range: [%.0f, %.0f] dBm | 2.4GHz Fresnel SBR', ...
    sum(active_mask), sum(mask), round(100*sum(active_mask)/max(1,sum(mask))), ...
    med_pwr, min(pwr_slice(active_mask)), max(pwr_slice(active_mask))), ...
    'FontSize', 8.5, 'BackgroundColor', 'w', 'EdgeColor', [0.5 0.5 0.5]);

%% 保存
out_file = sprintf('output/plots/indoor_rt_%s%.1f.png', slice_plane, slice_value);
saveas(gcf, out_file);
fprintf('Saved: %s\n', out_file);
