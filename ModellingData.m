clear all
clc
load('Datosgopro_invertido.txt');
load('data_tx_GoPro.mat');
% load('Dataframe23Julio2014/dataframe1.txt');
% load('Dataframe23Julio2014/dataframe2.txt');
% 
% dataframe = [dataframe1 dataframe2];
inputRC = Datosgopro_invertido(:,1);
% Nframe =    [1:3000];% dataframe(1,:)';
% Tframe =    dataframe(2,:)';
% PitchCtrl = dataframe(3,:)';
% RollCtrl =  dataframe(4,:)';
% YawCtrl =   dataframe(5,:)';
% ZCtrl =     dataframe(6,:)';
% Altd =      dataframe(7,:)';
% RotXSnsr =  dataframe(8,:)';
% RotYSnsr =  dataframe(9,:)';
% RotZSnsr = dataframe(10,:)';
% AclXSnsr = dataframe(11,:)';
% AclYSnsr = dataframe(12,:)';
% AclZSnsr = dataframe(13,:)';
% VelXSnsr = dataframe(14,:)';
% VelYSnsr = dataframe(15,:)';
% VelZSnsr = dataframe(16,:)';
% aAff =     dataframe(17,:)';
% xAff =     dataframe(18,:)';
% yAff =     dataframe(19,:)';
% sAff =     dataframe(20,:)';

h=fdesign.lowpass('N,F3dB',5,0.10)
d = design(h,'butter');
txfil = filtfilt(d.sosMatrix,d.ScaleValues,double(tx));
% RotZSnsr_fil = filtfilt(d.sosMatrix,d.ScaleValues,double(RotZSnsr-RotZSnsr(1)));

% figure(1);
% subplot(2,2,1);
% plot(Nframe, RollCtrl, Nframe, -xAff_fil/5, Nframe, -0.5*RotZSnsr_fil);grid on;

%% Model
% Input: [RollCtrl RotZSnsr_fil]
% Output: xAff_fil
% y = G_1(s)u_1 + G_2(s)u_2
% G_1(s)
Kp =  5.0349e-05;
Tp1 = 3.8203;
Tp2 = 0.11363;
Tz = -7136.3175;
[A1,B1,C1,D1] = tf2ss([Kp*Tz Kp],[Tp1*Tp2 Tp1+Tp2 1])
% % G_2(s)
% Kp = 0.00046312;
% Tp1 = 4.4643;
% Tp2 = 4.6491;
% Tz = 3877.9;
% [A2,B2,C2,D2] = tf2ss([Kp*Tz Kp],[Tp1*Tp2 Tp1+Tp2 1])
% plot(txfil)