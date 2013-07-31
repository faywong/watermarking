function g = getEnergy(x,dwr)
%
%GETENERGY Watermark embedding energy.
%   G = GETENERGY(X,DWR) use Document-to-Watermark Ratio (DWR) to calculate
%   embedding energy for signal X.

s2_x = var(x(:));                       % get the current variance
g = sqrt(s2_x/exp((log(10)*dwr)/10));   % solve DWR equation for g