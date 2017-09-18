ModellingData
clc

I = eye(2,2);
T = 0.0691244; % Sample Time
A = expm(A1*T); B = inv(A1)*(A-eye(2,2))*B1; C = C1; % Model

% Covariance
% G1 = [1;-1]; G = inv(A1)*(A-eye(2,2))*G1; V = 0;
% Q = B*V*B';
% R = 0.05;
% Q = 0.0009;
% Initial Data
u = inputRC; % Input
zk = txfil; % Output
% xpr = [0;0]; xps = xpr;
% Pk = B*Q*B';
% ze = C*xps;
Q = 100;
R = 0.00007;
P=B*Q*B';         % Initial error covariance
x=[0;0];     % Initial condition on the state
ze = C*x;

%% Kalman Filter
n = 465;
for k=2:n
  % Measurement update
  yk = zk(k)-C*x;
  Kk = P*C'/(C*P*C'+R);
  x = x + Kk*yk;  % x[n|n]
  P = (I-Kk*C)*P;     % P[n|n]

  ze(k) = C*x;  

  % Time update
  x = A*x + B*u(k-1);        % x[n+1|n]
  P = A*P*A' + B*Q*B';     % P[n+1|n]
end
% for k = 2:n
%     % Predict
%     xpr(:,k) = A*xpr(:,k-1)+B*u(k-1);
% %     Pk = A*P*A'+B*Q*B';%+Q;
%     
%     % Update
%     yk = zk(k)-C*xpr(:,k);
%     Kk=(Pk*C')*inv(C*Pk*C'+R);
%     xps(:,k)=xpr(:,k)+(Kk*yk); 
%     Pk = A*Pk*A' + B*Q*B';
% %     P=(I-Kk*C)*(Pk)*(I-Kk*C)'+Kk*yk*Kk';    
%     
%     ze(k)=C*xps(:,k);
% end


% h=fdesign.lowpass('N,F3dB',5,0.07)
% d = design(h,'butter');
% xAff_fil = filtfilt(d.sosMatrix,d.ScaleValues,double(xAff));


%% Plot
figure(3);
% plot([1:n],xAff_fil(1:n),[1:n],ze(1:n),[1:n],u(1:n));grid on;axis([1300 1700 -20 20]);
plot([1:n],ze(1:n),'r',[1:n],tx(1:n),'g',[1:n],txfil(1:n),'b');ylabel('X');xlabel('Frame');grid on;