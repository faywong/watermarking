function stat = wm_detect_rao(image,W,scales,levelONE,levelREMAIN,or,beta,est_fit)
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
       y = concat_v_bandhost;
       stat(cnt) = rc_ggd(y,concat_v_bandwm,beta,est_fit);
       cnt = cnt + 1;
    end
end


