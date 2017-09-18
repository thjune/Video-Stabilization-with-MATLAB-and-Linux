%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Video Stabilization
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

clear all
clc

%% Processing of Frames
frameB = im2single(rgb2gray(imread('Frames vuelo dron/frames utilizados/frameskalman/frame1.jpg')));
ii = 1;
Hcumulative = eye(3);
scumulative = [];
angcumulative = [];
tcumulative = [];
costcumulative = [];
while  ii <= 3740
    ii % Number of Frame
    
    % New Frame of Reference (Frame A)
    frameA = frameB;
    % New Frame for Processing (Frame B)
    dirorg = sprintf('Frames vuelo dron/frames utilizados/frameskalman/frame%d.jpg',ii);
    frameBreal = im2single(imread(dirorg));
    frameB = im2single(rgb2gray(frameBreal));
    
    %% Transform Estimation between Frame A and B, Escala(s), Rotacion(R), y Traslacion(t)
    Hest = cvexEstStabilizationTform(frameA,frameB);
    Hest = Hest';
    
    % Extraction of Rotation R, Traslation t, angle ang, and Scale s
    R = Hest(1:2,1:2);
    t = Hest(1:2,3);
    ang = mean([atan2(R(2),R(1)) atan2(-R(3),R(4))]);
    s = mean(R([1 4])/cos(ang));
    % Save s, ang, t
    scumulative = [scumulative s];
    angcumulative = [angcumulative ang];
    tcumulative = [tcumulative t];
    
    R = [cos(ang) -sin(ang); sin(ang) cos(ang)];
    Haff = [s*R t; 0 0 1];
    Hcumulative = Hcumulative * Haff;
%     Hcumulative = Haff;
    
    % Compensation of Frame B
    frameB_c = im2single(rgb2gray(imwarp(frameBreal,affine2d(Hcumulative'),'OutputView',imref2d(size(frameBreal)))));
    costcumulative = [costcumulative sum(sum(imabsdiff(frameB_c, frameA)))];

    ii = ii+1;
end

figure(1);
subplot(2,2,1);plot(scumulative);grid on;title('Evolucion de la Escala');ylabel('Escala');xlabel('Nro. Fotograma');
subplot(2,2,2);plot(angcumulative);grid on;title('Evolucion del angulo');ylabel('angulo');xlabel('Nro. Fotograma');
subplot(2,2,3);plot(tcumulative(2,:));grid on;title('Evolucion de la Traslacion en X');ylabel('X');xlabel('Nro. Fotograma');
subplot(2,2,4);plot(tcumulative(1,:));grid on;title('Evolucion de la Traslacion en Y');ylabel('Y');xlabel('Nro. Fotograma');

% save _VSData;