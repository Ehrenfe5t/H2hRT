clc;
close all;
clear all;
tic;



figure("Position",[10,10,968,850])
hold on;

filenameScenarioPoint3D='ModelSci4\\ScenarioAcceleratePoint3D.T.csv';
filenameScenarioTriangle='ModelSci4\\ScenarioAccelerateTriangle3D.T.csv';
DrawTriangles(filenameScenarioPoint3D,filenameScenarioTriangle);

% 读取 JSON 文件内容
jsonStr = fileread('Out.room.10GHz.05.m.0.e15.d0.s0.r0.t10.1\\[0].json');
% 将 JSON 字符串解码为 MATLAB 结构体
jsonData = jsondecode(jsonStr);
simo_paths = jsonData.propagationFrequencyInformations.propagationReceiverInformations;

for i=1:length(simo_paths)
    siso_paths = simo_paths(i).propagationPathInformations;
    for j=1:length(siso_paths)
        path = siso_paths(j).propagationNodeInformations;
        for k=2:1:length(path)
            location_1 = path(k-1).location;
            location_2 = path(k).location;
            DrawOneSegment(location_1,location_2);
        end

    end

end


clim([-120,0]);
xlabel('x(m)');
ylabel('y(m)');
set(gca,'FontSize',21,'fontname','Times New Roman');
axis equal
view(2);
xlim([-0.02,20.02]);
ylim([-0.02,20.02]);

% saveas(gcf, 'pic.p0.r5.RT-MR.png', 'png'); % 默认分辨率为96 DPI
set(gcf, 'PaperPositionMode', 'auto'); % 确保图形大小正确
print(gcf, '-dpng', '-r600', 'pic.r600.L1.5GHz.png');  % 设置 1000 DPI


toc;


function DrawOneLineSegment(lineSegment,color,LineWidth)
x=[lineSegment(1),lineSegment(4)];
y=[lineSegment(2),lineSegment(5)];
z=[lineSegment(3),lineSegment(6)];
hold on
plot3(x,y,z,'color',color,'LineWidth',LineWidth);
end

function DrawOneSegment(location_1,location_2)

lineSegment = [location_1.x,location_1.y,location_1.z,location_2.x,location_2.y,location_2.z];
color= [0,1,0];

DrawOneLineSegment(lineSegment,color,0.6);
end

function DrawTriangles(filenameScenarioPoint3D,filenameScenarioTriangle)


dataScenarioPoint3D = readmatrix(filenameScenarioPoint3D, 'NumHeaderLines', 1);
dataScenarioTriangle = readmatrix(filenameScenarioTriangle, 'NumHeaderLines', 1);

color_0 = [191,191,191]./255;%地面	0,176,76[]
color_1 = [191,191,191]./255;%天花板	1,[]
color_2 = [191,191,191]./255;%讲台	2,[]
color_3 = [191,191,191]./255;%Wall	3,[]
color_4 = [191,191,191]./255;%桌子	4,176,85,[]
color_5 = [191,191,191]./255;%椅子	5,[]
color_6 = [191,191,191]./255;%空调	6,[]
color_7 = [191,191,191]./255;%门	    7,[]


dataTriangle = GetScenarioTriangle(dataScenarioPoint3D,dataScenarioTriangle);

[r1,~] = size(dataTriangle);

transparency=0.1;
edgealpha=1;

dataTriangle_first =zeros(8,11);

[r2,~] = size(dataTriangle_first);
for i=1:r2
    dataTriangle_first(i,10) =i-1;
end


for i=1:r2

    p1=[dataTriangle_first(i,1),dataTriangle_first(i,2),dataTriangle_first(i,3)];
    p2=[dataTriangle_first(i,4),dataTriangle_first(i,5),dataTriangle_first(i,6)];
    p3=[dataTriangle_first(i,7),dataTriangle_first(i,8),dataTriangle_first(i,9)];

    if dataTriangle_first(i,10) == 0
        DrawOneTriangle(p1,p2,p3,color_0,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 1
        DrawOneTriangle(p1,p2,p3,color_1,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 2
        DrawOneTriangle(p1,p2,p3,color_2,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 3
        DrawOneTriangle(p1,p2,p3,color_3,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 4
        DrawOneTriangle(p1,p2,p3,color_4,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 5
        DrawOneTriangle(p1,p2,p3,color_5,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 6
        DrawOneTriangle(p1,p2,p3,color_6,transparency,edgealpha);
    elseif dataTriangle_first(i,10) == 7
        DrawOneTriangle(p1,p2,p3,color_7,transparency,edgealpha);
    end

end

for i=1:r1




    p1=[dataTriangle(i,1),dataTriangle(i,2),dataTriangle(i,3)];
    p2=[dataTriangle(i,4),dataTriangle(i,5),dataTriangle(i,6)];
    p3=[dataTriangle(i,7),dataTriangle(i,8),dataTriangle(i,9)];

    if dataTriangle(i,10) == 0
        DrawOneTriangle(p1,p2,p3,color_0,transparency,edgealpha);
    elseif dataTriangle(i,10) == 1
        DrawOneTriangle(p1,p2,p3,color_1,transparency,edgealpha);
    elseif dataTriangle(i,10) == 2
        DrawOneTriangle(p1,p2,p3,color_2,transparency,edgealpha);
    elseif dataTriangle(i,10) == 3
        DrawOneTriangle(p1,p2,p3,color_3,transparency,edgealpha);
    elseif dataTriangle(i,10) == 4
        DrawOneTriangle(p1,p2,p3,color_4,transparency,edgealpha);
    elseif dataTriangle(i,10) == 5
        DrawOneTriangle(p1,p2,p3,color_5,transparency,edgealpha);
    elseif dataTriangle(i,10) == 6
        DrawOneTriangle(p1,p2,p3,color_6,transparency,edgealpha);
    elseif dataTriangle(i,10) == 7
        DrawOneTriangle(p1,p2,p3,color_7,transparency,edgealpha);
    end


    % DrawOneTriangle(p1,p2,p3,color,transparency,edgealpha);

end


end


function DrawCorners(filenameScenarioPoint3D,filenameScenarioCorner)

dataScenarioPoint3D = readmatrix(filenameScenarioPoint3D, 'NumHeaderLines', 1);
dataScenarioCorner = readmatrix(filenameScenarioCorner, 'NumHeaderLines', 1);


[r1,~] = size(dataScenarioCorner);

for i=1:r1
    p1_index = dataScenarioCorner(i,1);
    p2_index = dataScenarioCorner(i,2);
    p1 = dataScenarioPoint3D(p1_index+1,:);
    p2 = dataScenarioPoint3D(p2_index+1,:);

    x=[p1(1),p2(1)];
    y=[p1(2),p2(2)];
    z=[p1(3),p2(3)];
    hold on
    plot3(x,y,z,'color','black','LineWidth',2);
end

end


function dataTriangle = GetScenarioTriangle(dataScenarioPoint3D,dataScenarioTriangle)
[r,~]=size(dataScenarioTriangle);
dataTriangle=zeros(r,10);
for i=1:r
    dataTriangle(i,1) = dataScenarioPoint3D(dataScenarioTriangle(i,1)+1,1);
    dataTriangle(i,2) = dataScenarioPoint3D(dataScenarioTriangle(i,1)+1,2);
    dataTriangle(i,3) = dataScenarioPoint3D(dataScenarioTriangle(i,1)+1,3);
    dataTriangle(i,4) = dataScenarioPoint3D(dataScenarioTriangle(i,2)+1,1);
    dataTriangle(i,5) = dataScenarioPoint3D(dataScenarioTriangle(i,2)+1,2);
    dataTriangle(i,6) = dataScenarioPoint3D(dataScenarioTriangle(i,2)+1,3);
    dataTriangle(i,7) = dataScenarioPoint3D(dataScenarioTriangle(i,3)+1,1);
    dataTriangle(i,8) = dataScenarioPoint3D(dataScenarioTriangle(i,3)+1,2);
    dataTriangle(i,9) = dataScenarioPoint3D(dataScenarioTriangle(i,3)+1,3);
    dataTriangle(i,10) = dataScenarioTriangle(i,5);
end
end

function p =StructToPoint3D(s)
p=[s.x,s.y,s.z];
end


function DrawOneTriangle(p1,p2,p3,color,transparency,edgealpha)
x=[p1(1),p2(1),p3(1),p1(1)];
y=[p1(2),p2(2),p3(2),p1(2)];
z=[p1(3),p2(3),p3(3),p1(3)];
hold on
h=fill3(x,y,z,color);
set(h,'edgealpha',edgealpha,'facealpha',transparency) 
end



