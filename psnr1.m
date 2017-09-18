clear all; close all; clc

folder = 'C:\Program Files\MATLAB\R2014b\bin\Frames\framesdespuesgimbalnoestabilizado';

inicio=1;
fin = 1897;
for i=inicio+1:fin
    ii = i-inicio
    iii= i-inicio+1
    
    baseFileName = sprintf ('frame%d.jpg',ii);
    fullFileName = fullfile(folder, baseFileName);
    imageArray = imread(fullFileName);
        
    %ref = rgb2gray (imageArray);
    
    baseFileName2 = sprintf ('frame%d.jpg',iii);
    fullFileName2 = fullfile(folder, baseFileName2);
    imageArray2 = imread(fullFileName2);
    
    %ref2 = rgb2gray (imageArray2);
    
    %error = immse (ref2, ref)
    [peaksnr, snr] = psnr (imageArray2, imageArray);
    filename = 'C:\Users\Thomas\Documents\mse antes y despues\itf gimbal no estabilizada.xlsx';
    xlswrite(filename,peaksnr,1,['C' num2str(ii)]);
end