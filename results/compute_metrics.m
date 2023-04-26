% Script to compute the performance metrics

clc
clear

%% Parameters
handshake_ways = 3; % 0, 1, 2 or 3 way handshake (0: CSMA, 1: ADAPT-1, 2: CSMA/CA, 3: ADAPT-3)
nodeNum = 50;       % Number of client nodes
Tia = 200;          % [us] Mean inter-arrival time

%% Load simulation results
filename = sprintf('result_%uway_%un_%uus_1.txt',handshake_ways,nodeNum,Tia);
fileID = fopen(filename,'r');
data = fscanf(fileID,'%u %u %u %u %u',[5 Inf]);
fclose(fileID);

%% Compute metrics
data = data.';

% Throughput calculation
throughput_node = zeros(1,nodeNum); % [Gbps] Throughput of each node
for n = 1:nodeNum
    % Select only data from node n and succesfull packets
    succ_data = data(data(:,1) == n & data(:,4) == 1,:);
    throughput_node(n) = mean(succ_data(:,2)*8./(succ_data(:,3)));
    if size(succ_data,1) == 0
        throughput_node(n) = 0;
    end
end
throughput = mean(throughput_node); % [Gbps] Average throughput

% Discard rate
discard_rate = sum(data(:,5))/size(data,1);

% [us] Average packet time
succ_data = data(data(:,4) == 1,:);
average_packet_time = mean(succ_data(round(end/10):end,3))*1e-3;

%% Print out results
fprintf('Throughput = %.2f Gbps\n',throughput)
fprintf('Discard rate = %.2f\n',discard_rate)
fprintf('Average packet time = %.2f us\n',average_packet_time)
