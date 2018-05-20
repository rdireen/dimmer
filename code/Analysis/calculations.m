%% Clock
Clk = 16e6;
dT = 1/Clk; %625 ns
disp(['dt = ',num2str(dT*1e9), ' ns'])

halfCycle = 1/120; % 8.3 ms, the period of a 60 Hz half cycle

%% Timer 1
% 16-bit timer 
% 
disp('***TIMER1***')
T1prescale = 1024;

T1max = 2^16*T1prescale/Clk;
disp(['T1max = ',num2str(T1max*1e3), ' ms'])

dtT1 = T1prescale/Clk;
disp(['dtT1 = ',num2str(dtT1*1e6), ' us'])

disp(['Largest Count Value = ',num2str(floor(halfCycle/dtT1))])


%% Timer 2
% 8-bit timer 
% 

disp('***TIMER2***')
T2prescale = 1024;

T2max = 2^8*T2prescale/Clk; 
disp(['T2max = ',num2str(T2max*1e3), ' ms'])

dtT2 = T2prescale/Clk;
disp(['dtT2 = ',num2str(dtT2*1e6), ' us'])

disp(['Largest Count Value = ',num2str(floor(halfCycle/dtT2))])

%% Power delivered to bulb

td = linspace(0,8.3e-3);
f0 = 60

Pa = 1/2 - td*f0 + sin(4*pi*f0*td)/(4*pi);

plot(td*1e3, Pa)
xlabel('t_d [ms]')
ylabel('normalized average P')
title('Power Delivered to Bulb vs t_d')

%% Log Power

I = 10*log10(Pa);

plot(td*1e3, I)
xlabel('t_d [ms]')
ylabel('normalized average I [dB]')
title('Power Delivered to Bulb vs t_d')

%%%% Purpose
% 5/3/2018
%
% Need to make sure I can control the light using input percentage of 
% power in such a way that increasing percentage increases the brightness
% of the bulb linearly
% 

%% Power delivered to bulb
% This is the power delivered to the bulb vs the delay time t_d
f0 = 60;
Td2 = 1/f0/2;
close all
td = linspace(0,8.33e-3);


% Equation of average power delivered vs delay time
Pa = 1/2 - td*f0 + sin(4*pi*f0*td)/(4*pi);

subplot(211)
plot(td*1e3, Pa)
xlabel('t_d [ms]')
ylabel('Pa')
title('Average Power Delivered to Bulb vs t_d')
subplot(212)
I = 10*log10(Pa);
plot(td*1e3, I)
xlabel('t_d [ms]')
ylabel(' I(t_d) [dB]')
title('Power Delivered to Bulb vs t_d')


%% Solve for td(eta) so we know how to linearly increase Intensity
close all
minV = -30;
maxV = -3;
FF = @(td, eta) sin(4*pi*f0*td)/(4*pi) - f0*td + 1/2 - 10^(((maxV-minV)*(eta/100) + minV)/10);
eta = 0:100;
tdvec = zeros(1,length(eta));
for n = 1:length(eta)
    tdvec(n) = fzero(@(td) FF(td, eta(n)), 1);
end

subplot(211)
plot(eta, tdvec*1e3)
title('td(\eta)')
ylabel('t_d [ms]')
xlabel('percentage \eta')
subplot(212)
Iv = 10*log10(1/2 - tdvec*f0 + sin(4*pi*f0*tdvec)/(4*pi));
plot(eta, Iv)
title('Verify \eta vs I(\eta) linearity')
ylabel('I(\eta)')
xlabel('percentage \eta')

%% Display the map of eta vs time delay in ms
disp(tdvec*1e6)

%% Resolution 8-bit
% Because I am using 2 8-bit timers I only have 130 different values for
% t_d
%
% I don't know how low in brightness I can go before the light looks almost
% off, once I figure that out I can determine whether 8 bits is good enough
% for this project.
MinDimLocation = 55;

close all
dt = 1/(16e6/1024); % 640 us
Tmx = 2^8*dt; % max time 16.4 ms
maxVal = floor(2^8*Td2/Tmx); % This is 130 in this case
vals = round(tdvec/Td2*maxVal);
subplot(211)
plot(eta, vals)
title('ticks(\eta) with 8-bit resolution')
ylabel('ticks')
xlabel('percentage \eta')
hold on
plot([MinDimLocation MinDimLocation], [0 150], 'r')
ylim([0 150])
subplot(212)

tdvec8bit = vals*Td2/maxVal
Iv = 10*log10(1/2 - tdvec8bit*f0 + sin(4*pi*f0*tdvec8bit)/(4*pi));
plot(eta, Iv)
title('Verify \eta vs I(\eta) linearity 8-bit')
ylabel('I(\eta) 8-bit')
xlabel('percentage \eta')
ylim([-100 0])
xlim([0 100])
hold on
plot([MinDimLocation MinDimLocation], [-100 0], 'r')
ylim([-100 0])
disp(round(tdvec/Td2*maxVal))
csvwrite('table8bit.csv', vals);

%% Resolution 16-bit
% If I use a 16-bit timer
% t_d



close all
dt = 1/(16e6/8); % 500 ns
Tmx = 2^16*dt; % max time 32.7 ms
maxVal = floor(2^16*Td2/Tmx); % This is 130 in this case
vals = round(tdvec/Td2*maxVal);
subplot(211)
plot(eta, vals)
title('ticks(\eta) with 16-bit resolution')
ylabel('ticks')
xlabel('percentage \eta')
hold on
plot([MinDimLocation MinDimLocation], [0 2*10^4], 'r')
ylim([0 2*10^4])
subplot(212)

tdvec16bit = vals*Td2/maxVal;
Iv = 10*log10(1/2 - tdvec16bit*f0 + sin(4*pi*f0*tdvec16bit)/(4*pi));
plot(eta, Iv)
title('Verify \eta vs I(\eta) linearity 16-bit')
ylabel('I(\eta) 16-bit')
xlabel('percentage \eta')
ylim([-100 0])
hold on
plot([MinDimLocation MinDimLocation], [-100 0], 'r')
ylim([-100 0])
disp(round(tdvec/Td2*maxVal))
csvwrite('table16bit.csv', vals); 








