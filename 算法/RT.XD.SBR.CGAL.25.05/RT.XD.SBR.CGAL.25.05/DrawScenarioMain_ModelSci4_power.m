clc;
close all;
clear all;
tic;

delay_data_Rx1_list = ReadCsvFile500000("Out.room.10GHz.05.m.0.e15.d0.s0.r0.t10.1\\Tx[0]\\frequency[10000].MHz\\Rx[1].csv");
delay_col = 2;
power_col = 3;
delay_Rx1 = delay_data_Rx1_list(:,delay_col);
power_Rx1 = delay_data_Rx1_list(:,power_col);

minPower = -120;

DrawDelayPower2(delay_Rx1,power_Rx1,minPower,1,'Tx1-Rx1');



toc;

function DrawDelayPower2(delay,power,minPower,nnn,legendStr)

% subplot(2,4,nnn+1)
figure()
hold on 

for i=1:length(delay)
    x=[delay(i),delay(i)];
    y=[minPower,power(i)];
    plot(x,y,'Color',[hex2dec('4C'),hex2dec('62'),hex2dec('9C')]./255,'LineWidth',1);
end

xlimArray=[0,82];
ylimArray=[minPower,-0];
% xlim(xlimArray);
ylim(ylimArray);
% 
xticks([0,40,80]);
xticklabels({'0','40','80'});
% yticks([-150,-100,-50]);
% yticklabels({'-150','-100','-50'});
xlabel('Delay(ns)');
ylabel('Power(dBm)');
% legend(legendStr,'box','off');
set(gca,'FontSize',22,'fontname','Times New Roman');
end

function data = ReadCsvFile500000(filename)
fid = fopen(filename,'r');
data=[];
fgetl(fid);
count=500000;
while ~feof(fid)
    str = fgetl(fid);
    strs = regexp(str,',','split');
    curData = DoWithStrsCell(strs);
    data=[data;curData];
    count=count-1;
    if count<0
        break;
    end
end
end



function num = DoWithStrsCell(strs)
num=[];
for i=1:length(strs)
    num=[num str2num(char(strs{i})) ];
end
end

