function pm = roc_compress_plots(base,attacktype,detectortype,dist_param,pfa,dwr,image)
    filespec = sprintf('%s_%0.2f_%s_*.mat', detectortype, dist_param, attacktype);
    files = dir(fullfile(base,filespec));
    for i=1:size(files,1)
        filename = fullfile(base,files(i).name);
        tmp = load(filename);
        data{i} = tmp.collect;
    end
    switch detectortype
        case 'rao'
            for i=size(data,2):-1:1
                pm(i) = compute_pm_rao(data{i},image,dwr,pfa);
            end
        case 'lc'
            size(data,2)
            for i=size(data,2):-1:1
                pm(i) = compute_pm_lc(data{i},image,dwr,pfa);
            end
        case 'hernandez'
           for i=size(data,2):-1:1
                pm(i) = compute_pm_hernandez(data{i},image,dwr,pfa);
           end
    end
    pm = wrev(pm);
end


function pm = compute_pm_hernandez(data,image,dwr,pfa)
    h1 = 2; h0 = 1;
    muh1 = mean(data{h1,image,dwr}.stat);
    sh1 = std(data{h1,image,dwr}.stat); % required for computing the normcdf
    muh0 = mean(data{h0,image,dwr}.stat);
    s2h0 = var(data{h0,image,dwr}.stat); %
    thr = sqrt(2*s2h0) * erfcinv(2*pfa) + muh0;
    pm = normcdf(thr, muh1,sh1);
end

function pm = compute_pm_lc(data,image,dwr,pfa)
    h1 = 2; h0 = 1;
    muh1 = mean(data{h1,image,dwr}.stat);
    sh1 = std(data{h1,image,dwr}.stat); % required for computing the normcdf
    muh0 = mean(data{h0,image,dwr}.stat);
    s2h0 = var(data{h0,image,dwr}.stat); 
    thr = sqrt(2*s2h0) * erfcinv(2*pfa) + muh0;
    pm = normcdf(thr, muh1,sh1);
end

function pm = compute_pm_rao(data,image,dwr,pfa)
    h1 = 2;
    lambda = power(mean(sqrt(data{h1,image,dwr}.stat)),2);
    T = power(qfuncinv(pfa/2),2);
    pm = ncx2cdf(T,1,lambda);
end

function wmseq = extract(W,level,direction) 
    [Yl_wm,Yh_wm] = dtwavexfm2(double(W),level,'near_sym_b','qshift_b');
    band_wm = Yh_wm{level}(:,:,direction);
    v_bandwm = band_wm(:); 
    wmseq = [real(v_bandwm);imag(v_bandwm)];
end

