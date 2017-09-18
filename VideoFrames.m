clear all
clc

Videoframes = VideoReader('estatica no estabilizado despues.wmv');
 
% numframes = Videoframes.NumberOfFrames
% vidheight = Videoframes.Height
% vidwidth = Videoframes.Width

inicio =1;
fin = 281474976710655;

for i=inicio+1:fin
    ii = i-inicio
    dirorg = sprintf('framesdespuesestaticanoestabilizado/frame%d.jpg',ii);
    imwrite(read(Videoframes,i),dirorg);
end