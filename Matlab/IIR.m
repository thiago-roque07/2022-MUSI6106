% Authors: P. Dutilleux, U Zölzer
%
%--------------------------------------------------------------------------
% This source code is provided without any warranties as published in 
% DAFX book 2nd edition, copyright Wiley & Sons 2011, available at 
% http://www.dafx.de. It may be used for educational purposes and not 
% for commercial applications without further permission.
%--------------------------------------------------------------------------
filename = 'C:/Users/thiag/Documents/Git-repos/2022-MUSI6106/audio/audio_sine440.wav';

[x,Fs] = audioread(filename);
delayInSec = 0.01;
delayInSample = delayInSec * Fs;
g=1;

Delayline=zeros(2*delayInSample,1);% memory allocation 
for n=1:length(x)
	y(n)=x(n)+g*Delayline(delayInSample);
	Delayline=[y(n);Delayline(1:delayInSample-1)];
end

FilteredFile = 'C:/Users/thiag/Documents/Git-repos/2022-MUSI6106/Matlab/audio_sine440.wav_delay.txt';
fileID = fopen(FilteredFile, 'r');
formatSpec = '%f';
sizeA = [2 Inf];
A = transpose(fscanf(fileID,formatSpec, sizeA));
A = A(:,1);

figure;
plot(x(1:3000,1));
figure;
plot(y(1:3000));
title('Matlab Implementation - IIR Comb Filter')
figure;
plot(A(1:3000));
title('c++ Implementation - IIR Comb Filter')

B = transpose(y(1:length(A)));

diff = A - B;

figure;
plot(diff(1:3000));
title('IIR difference')