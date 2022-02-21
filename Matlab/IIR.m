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

% x=zeros(100,1);x(1)=1; % unit impulse signal of length 100
g=0.5;
Delayline=zeros(100,1);% memory allocation for length 10
for n=1:length(x)
	y(n)=x(n)+g*Delayline(100);
	Delayline=[y(n);Delayline(1:100-1)];
end

FilteredFile = 'C:/Users/thiag/Documents/Git-repos/2022-MUSI6106/audio/audio_sine440.wav_delay';
fileID = fopen(FilteredFile, 'w');
formatSpec = '%f';
sizeA = [2 Inf];
A = fscanf(fileID,formatSpec, sizeA);

diff = transpose(A(1:length(A))) - y(1:length(A));
xAxis = 1:length(A);

plot(diff, 'LineWidth', 1);
title('FIR difference')