function r = rc_ggd(y,wm,shape, est_fit)
    if (shape == -1)
        if est_fit == 0
            [mhat,ahat,bhat] = ggmle(y);
        else
            bhat = fitGGauss(y, 0.5, 2.1);
            bhat
        end
        if (bhat < 0.5)
            bhat = 0.5;
        end
        beta = bhat;
    else
        beta = shape;
    end
    absy = abs(y);
    expo = beta-1;
    expo2 = 2*beta-2;
    p = find(abs(y) ~= 0);
    absy = absy(p);
    wm = wm(p);
    y = y(p);
    z = (absy).^expo;
    z2 = (absy).^expo2;
    s3 =  sum(sign(y).*wm.*z)^2; % nenner
    v1 = (wm.^2) * sum(z2); 
    s1 = 1/length(wm) * sum(v1); % zaehler
    r = s3/s1;
end
