function [db,hostwm] = wm_embed(image,W,scales,levelONE,levelREMAIN,dwr,levels,or)

[Yl_host,Yh_host,Yscale_host] = dtwavexfm2(double(image),scales,levelONE,levelREMAIN);
[Yl_wm,Yh_wm,Yscale_wm] = dtwavexfm2(double(W),scales,levelONE,levelREMAIN);

Yl_hostwm = Yl_host;
Yh_hostwm = Yh_host;

for level=levels
    for direction=or
        band_host = Yh_host{level}(:,:,direction);
        band_wm = Yh_wm{level}(:,:,direction);
        
        sz = size(band_wm);
        v_bandwm = band_wm(:); 
        v_bandhost = band_host(:);
        
        alpha = getEnergy([real(v_bandhost);imag(v_bandhost)],dwr) * (1/var([real(v_bandwm);imag(v_bandwm)]));
        v_bandhostwm = alpha*[real(v_bandwm);imag(v_bandwm)] + [real(v_bandhost);imag(v_bandhost)];
       
        real_v_bandhostwm = v_bandhostwm(1:length(v_bandhostwm)/2);
        imag_v_bandhostwm = v_bandhostwm(length(v_bandhostwm)/2+1:end);
        real_bandhostwm = reshape(real_v_bandhostwm,sz(1),sz(2));
        imag_bandhostwm = reshape(imag_v_bandhostwm,sz(1),sz(2));
        band_hostwm = real_bandhostwm + sqrt(-1)*imag_bandhostwm;
        Yh_hostwm{level}(:,:,direction) = band_hostwm;
    end
end
hostwm = dtwaveifm2(Yl_hostwm,Yh_hostwm,levelONE,levelREMAIN);
hostwm = max(0,min(quant(hostwm,1),255));  
hostwm = uint8(hostwm);
[MSE,PSNR,AD,SC,NK,MD,LMSE,NAE] = iq_measures(double(image),double(hostwm));
db = PSNR;
