%% Parameters
num_nodes = 50;
tia = 200;
handshake_ways = 3;

%% Check folder
% Execute from 'macro_postprocessing' folder
path = '../';
if exist(path, 'dir')~= 7
   Message = sprintf('Error: The following folder does not exist:\n%s', path);
   uiwait(warndlg(Message));
   return;
end

%% Run

sname = sprintf('result_%uway_%un_%uus_1.txt', handshake_ways, num_nodes, tia);
filePattern = fullfile(path, sname);
fileID = fopen(filePattern);

formatSpec = '%i %i %f %i %i'; 
dims = [5 Inf];
data = fscanf(fileID, formatSpec, dims);

[thr, time, time_perm, Pdis] = computeMetrics(data,num_nodes);

throughput = thr                    % [bps]
average_packet_time = time_perm     % [ns]
discard_rate = Pdis



%% Compute metrics 
% data: Matrix of results for the node. 
% node: Number of nodes

function [throughput, time, time_perm, discard_rate] = computeMetrics(data, num_nodes)

data = data';
packet_size = data(1,2);
throughput = 0;
time = 0;

for j = 1:num_nodes
    th_aux = 0;
    time_aux = 0;
    node_data = data(data(:,1)==j,:);           % Select only data from node j
    succ_data = node_data(node_data(:,4)==1,:); % Select successfull packets 
    num_succ = length(succ_data(:,1));
    for i = 1:num_succ
        th = packet_size * 8 / (succ_data(i,3) * 1e-9);
        th_aux = th_aux + th;
        time_aux = time_aux + (succ_data(i,3) * 1e-3);
    end
    th_node = th_aux / num_succ;
    time = time + time_aux / num_succ;
    if num_succ == 0
        th_node = 0;
    end
    throughput = throughput + th_node;
end

% average time on permanent phase
succ = data(data(:,4)==1, :);
time_perm = mean(succ(round(length(succ)/10) : length(succ), 3)) * 1e-3;

throughput = throughput / num_nodes;
time = time / num_nodes;
discard_rate = length(data(data(:,5)==1,:)) / length(data);
end