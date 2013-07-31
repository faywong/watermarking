
%% Global Settings
clear glob;

glob.scale = 0.2:0.2:1.6;
glob.miny = 1e-40;
glob.qualifier = 'j2k1e-10';
glob.yax = 'JPEG2000 Rate [bpp]';
glob.pfa = 1e-10;
glob.imagedir = 'images';
glob.attack = 'jpeg';
glob.tmp_dir = '/tmp';
glob.dwr = 2;

clear data;

%% Data 
clear x;
x.label = 'Rao (\beta=1.0)';
x.detector = 'rao';
x.dist_param = 1.0;
data{1} = x;

clear x;
x.label = 'LRT (\beta=1.0)';
x.detector = 'hernandez';
x.dist_param = 1.0;
data{2} = x;

clear x;
x.label = 'LC';
x.detector = 'lc';
x.dist_param = 1.0; %% not needed for LC
data{3} = x;

%% Call to plot
draw_plot(data, glob);

