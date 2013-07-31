
%% Set Path and dependencies
addpath('D:\DSP09-dtcwt-wm-r121');
addpath('D:\DSP09-dtcwt-wm-r121\dep\dtcwt_toolbox4_3');
addpath('D:\DSP09-dtcwt-wm-r121\dep\QualityAssessment');

%% Global
options.tmp_dir = 'D:\DSP09-dtcwt-wm-r121\tmp';
options.base_dir = 'images';

%% embedding to perform at decomposition level
options.embed_level = 2;

options.seed = 5431;
options.im_dim = 256;
options.dwrs = [12 16 20]; 

%% embedding orientation (1 .. 6)
options.orientation = 4; 

%% number of random watermarks to test
%% set this to 1000 to obtain reasonable experimental results
options.num_random = 5;

%% 'hernandez' for LRT-GGD, 'lc' for linear correlation, 'rao' for Rao-GGD detector
options.detectortype = 'lc';

%% EST .. use Minh Do estimation, FIT .. use stupid fitting
GGD_EST = 0;
GGD_FIT = 1;
options.est_fit = GGD_EST;

JPEG_ATTACK = 1;
J2K_ATTACK = 2;
selectattack = JPEG_ATTACK;

%% Test runs
jpeg_rates = 10:10:90;
j2k_rates = 0.2:0.2:1.6;
ggdshapes = [-1; 0.8; 1.0; 1.2; 0.51];

for i=1:length(ggdshapes)
    options.ggdshape = ggdshapes(i);
    fprintf('Testing GGD shape %d (-1 .. estimate)\n',options.ggdshape);
    switch selectattack
        case JPEG_ATTACK
            for j=1:length(jpeg_rates)
                fprintf('Testing JPEG Q=%d\n', jpeg_rates(j));
                options.jpeg_rate = jpeg_rates(j);
                options.j2k_rate = -1;
                options.attacktype = JPEG_ATTACK;
                collect = attack_test(options);
                save_string = sprintf('%s/%s_%0.2f_jpeg_%d.mat', options.tmp_dir, options.detectortype, options.ggdshape, options.jpeg_rate);
                save(save_string,'collect');
                clear collect;
            end
        case J2K_ATTACK
            for j=1:length(j2k_rates)
                fprintf('Testing JPEG2000 bpp=%d\n', j2k_rates(j));            
                options.jpeg_rate = -1;
                options.j2k_rate = j2k_rates(j);
                options.attacktype = J2K_ATTACK;
                collect = attack_test(options);
                save_string = sprintf('%s/%s_%0.2f_j2k_%0.2f.mat',options.tmp_dir, options.detectortype, options.ggdshape, options.j2k_rate);
                save(save_string,'collect');
                clear collect;
            end
    end
end


