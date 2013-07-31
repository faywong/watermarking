function stat = wm_detect_lc(image,W,scales,levelONE,levelREMAIN,or)
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
       stat(cnt) = 1/length(wm) * sum(signal .*wm); 
       cnt = cnt + 1;
    end
end
