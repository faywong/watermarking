function stat = wm_detect_ggdclassic(image,dwr,W,scales,levelONE,levelREMAIN,or,shape,est_fit)
[Yl_host,Yh_host] = dtwavexfm2(double(image),scales,levelONE,levelREMAIN);
[Yl_wm,Yh_wm] = dtwavexfm2(double(W),scales,levelONE,levelREMAIN);
cnt = 1;
for level=scales % select levels
    for direction=or % select orientations
       band_host = Yh_host{level}(:,:,direction);
       band_wm = Yh_wm{level}(:,:,direction);
       v_bandwm = band_wm(:); 
       v_bandhost = band_host(:);
       concat_v_bandhost = [real(v_bandhost);imag(v_bandhost)];
       concat_v_bandwm = [real(v_bandwm);imag(v_bandwm)];
       signal = concat_v_bandhost;
       wm = concat_v_bandwm;
       alpha = getEnergy(signal,dwr) * (1/var(concat_v_bandwm));
       if (shape == -1)
          if est_fit == 0
              [mhat,ahat,bhat] = ggmle(signal); 
              ck = bhat; beta = ahat;
              stat(cnt) = sum(1/(beta^ck)*(abs(signal).^ck - abs(signal-alpha*wm).^ck));   
          else
              ck = fitGGauss(signal, 0.3, 2.1);
              ck
              beta = sqrt(gamma(3/ck)/gamma(1/ck))/std(signal);
              stat(cnt) = sum((beta^ck)*(abs(signal).^ck - abs(signal-alpha*wm).^ck));   
          end
       else
          ck = shape;
          beta = sqrt(gamma(3/ck)/gamma(1/ck))/std(signal);
          stat(cnt) = sum(beta^ck*(abs(signal).^ck-abs(signal-alpha*wm).^ck));
       end
       cnt = cnt + 1;
    end
end


