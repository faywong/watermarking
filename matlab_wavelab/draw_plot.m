function draw_plot(data, gsettings)
    curve_id = {'red-d','blue-s','black-*','magenta-x'};

    scale = gsettings.scale; % x-axis scale (JPEG quality, JPEG200 rate, etc.)
    miny = gsettings.miny; % min. of y-axis scaling
    tmp_dir = gsettings.tmp_dir; % plot eps in this directory
    qualifier = gsettings.qualifier; % signifies attacktype
    pfa = gsettings.pfa; % prob. of false alarm
    attack = gsettings.attack;
    dwr = gsettings.dwr;
    yaxisstr = gsettings.yax; % Labeling of Y-Axis
    imagedir = gsettings.imagedir; % directory of pgm images
        
    listing = dir(fullfile(imagedir,'*.pgm')); % only for imagenames
    ncurves = length(data); % number of curves to plot
    if (ncurves > 4) 
        error('Only four curves in one plot allowed');
    end
    nimages = length(listing); % number of images
    dwrlab = {}; scalew = {};
    for i=1:nimages
        figure % open figure
        for j=1:ncurves
            disp(sprintf('%s %0.2f %s', data{j}.detector, data{j}.dist_param, attack))
            pm = roc_compress_plots(tmp_dir, ...
                attack, ...
                data{j}.detector, ... 
                data{j}.dist_param, ...
                pfa, dwr, i); % i is the i-th image
            % pm is reversed (lowest compression rate first)
            semilogy(1:length(scale),pm,curve_id{j},'Markersize',4); 
            if (j == 1) 
                hold; 
            end % hold plot after plotting the first curve
            dwrlab{j} = data{j}.label;
        end
        for j=length(scale)-1:-1:1 %from 1.4 to 0.2
            scalew{length(scale)-1-j+1} = num2str(scale(j));
        end
        axis([2 8 miny 1]);
        set(gca,'XTickLabel',scalew);
        xlabel(yaxisstr);
        ylabel('Probability of Miss');
        legend(dwrlab,'Location','SouthEast');
        grid; % draw grid
        fname = strsplit('.',listing(i).name);
        ts = sprintf('%s',strcat(upper(fname{1}(1)),fname{1}(2:end)));
        title(ts);
        set(gcf, 'PaperPositionMode', 'manual');
        set(gcf, 'PaperType', 'A4');
        set(gcf, 'PaperUnits', 'centimeters');
        set(gcf, 'PaperPosition', [0 0 11 8]);
        s = sprintf('%s/%s-%s.eps',tmp_dir,listing(i).name,qualifier);
        print('-deps',s);
    end
end         
