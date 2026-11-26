clear; clc; close all;

%% Parametros
port  = "COM3";
Ts    = 0.01;      % periodo de muestreo [s] (period/1000 del firmware)
ppr   = 200;       % pulsos por revolucion
% %ESCALON
% tStep = 1.0;       % instante del escalon [s] (T_STEP_MS/1000 del firmware)
% tEnd  = 3.0;       % fin del experimento [s]: MATLAB corta aqui aunque no llegue END
%PRBS9
tStart = 1.0;       
Tb     = 0.1;                % duracion de un bit [s]    (T_BIT_MS/1000)
Nprbs  = 511;                % longitud de la PRBS9
tEnd   = tStart + Nprbs*Tb;  % fin del experimento [s]


%% Adquisicion
s = serialport(port, 115200);
configureTerminator(s, "CR/LF");
s.Timeout = 5;
flush(s);

disp("Presiona RESET en la placa para iniciar el escalon...");
data = [];
tAcq = tic;
while true
    % %ESCALON
    % if toc(tAcq) > 30                        % limite de seguridad
    %     warning('Se supero el tiempo maximo de adquisicion.');
    %     break;
    % end
    %PRBS9
    if toc(tAcq) > tEnd + 15                 % limite de seguridad
        warning('Se supero el tiempo maximo de adquisicion.');
        break;
    end

    raw = readline(s);
    if isempty(raw)                          % timeout: no llegan datos
        warning('Timeout: el puerto dejo de enviar datos (sin END).');
        break;
    end
    line = strtrim(raw);
    if contains(line, "END")                 % END de una corrida vieja se ignora
        if isempty(data), continue; else, break; end
    end

    v = sscanf(line, '%f,%f,%f');
    if numel(v) ~= 3
        fprintf('Linea ignorada: %s\n', line);
        continue;
    end

    % Ignorar lo que llegue de una corrida ya en marcha: solo se empieza a
    % guardar cuando aparece una muestra con t pequeno (placa recien reiniciada)
    if isempty(data) && v(1) > 200
        continue;
    end

    data(end+1,:) = v';
    % %ESCALON
    % if mod(size(data,1), 50) == 0
    %PRBS9
    if mod(size(data,1), 100) == 0
        fprintf('t = %.2f s | duty = %g %% | pulsos = %g\n', v(1)/1000, v(2), v(3));
    end
    if v(1)/1000 >= tEnd, break; end         % fin del experimento
end
clear s;

if isempty(data)
    error('No se capturo el inicio del experimento. Corre el script y presiona RESET cuando aparezca el mensaje.');
end

% %ESCALON
% save("escalon.mat", "data");
%PRBS9
save("prbs.mat", "data");

%% Limpieza de datos
data = data(~any(isnan(data),2), :);
t = data(:,1)/1000;                  % s
u = data(:,2);                       % duty [%]
y = data(:,3) * 60 / (Ts*ppr);       % RPM

% %ESCALON
% %% Modelo de primer orden: y = K*U*(1 - exp(-(t-tStep)/tau))
% Ustep = max(u);
% n20   = max(round(0.2*numel(y)), 1);
% yss   = mean(y(end-n20+1:end));
% 
% if yss <= 0
%     error('yss = 0: el motor no giro o pulse_count quedo en 0. Revisa los datos con plot(t,y).');
% end
% 
% m  = t >= tStep;
% K  = yss / Ustep;
% 
% % tau: tiempo en alcanzar el 63.2 % de yss despues del escalon
% ys  = movmean(y, 3);
% idx = find(ys >= 0.632*yss & t > tStep, 1);
% if isempty(idx)
%     error('La salida nunca alcanzo el 63.2 %% de yss.');
% end
% tau = max(t(idx) - tStep, Ts);               % evita tau = 0

%PRBS9
%% Identificacion por minimos cuadrados (ARX de primer orden)
%   y[k] = a*y[k-1] + b0*u[k] + b1*u[k-1]
% Se usan solo las muestras con la PRBS aplicada.
k = find(t >= tStart);
k = k(k >= 2);

Phi   = [y(k-1), u(k), u(k-1)];      % regresores
theta = Phi \ y(k);                  % solucion de minimos cuadrados
a  = theta(1);
b0 = theta(2);
b1 = theta(3);

if a <= 0 || a >= 1
    error('Polo discreto a = %.3f fuera de (0,1): no se puede obtener tau. Revisa los datos.', a);
end

K   = (b0 + b1) / (1 - a);           % ganancia estatica [RPM/%]
tau = -Ts / log(a);                  % constante de tiempo [s]

ym    = zeros(size(t));                      % RPM = 0 antes del escalon
% %ESCALON
% ym(m) = K*Ustep*(1 - exp(-(t(m) - tStep)/tau));
%PRBS9
for i = 2:numel(t)
    ym(i) = a*ym(i-1) + b0*u(i) + b1*u(i-1);
end

e    = y - ym;                                        % error [RPM]
fit  = 100*(1 - norm(y - ym)/norm(y - mean(y)));      % NRMSE fit [%]
rmse = sqrt(mean(e.^2));                              % error cuadratico medio [RPM]
fprintf("K = %.3f RPM/%%   tau = %.4f s   fit = %.1f %%   RMSE = %.2f RPM\n", K, tau, fit, rmse);

%% Colores y estilo
cDatos  = [0.255 0.169 0.58];
cModelo = [0.58 0.169 0.255];
cGrid   = [0.90 0.90 0.90];

fig = figure('Color','w','Position',[100 100 900 720]);
tl  = tiledlayout(fig, 3, 1, 'TileSpacing','compact','Padding','compact');

%% Panel superior: PWM
ax1 = nexttile(tl);
stairs(ax1, t, u, 'Color', cDatos, 'LineWidth', 1.8);
ylabel(ax1, 'Duty Cycle (PWM) (%)');
ylim(ax1, [-3, max(u)*1.15]);
xlim(ax1, [0 max(t)]);
ax1.XTickLabel = [];

%% Panel central: RPM
ax2 = nexttile(tl);
hold(ax2, 'on');
p1 = plot(ax2, t, y,  '-',  'Color', cDatos,  'LineWidth', 1.5);
p2 = plot(ax2, t, ym, '-', 'Color', cModelo, 'LineWidth', 2.0);
ylabel(ax2, 'RPM');
xlim(ax2, [0 max(t)]);
ylim(ax2, [0 max(y)*1.1]);
ax2.XTickLabel = [];

legend(ax2, [p1 p2], ...
    {'Datos experimentales', sprintf('Modelo (fit %.1f %%)', fit)}, ...
    'Orientation','horizontal', 'Location','northoutside', 'Box','off');

%% Panel inferior: error (datos - modelo)
ax3 = nexttile(tl);
hold(ax3, 'on');
yline(ax3, 0, '-', 'Color', [0.6 0.6 0.6], 'LineWidth', 0.8);
plot(ax3, t, e, '-', 'Color', [0.85 0.25 0.20], 'LineWidth', 1.3);
xlabel(ax3, 'Tiempo (s)');
ylabel(ax3, 'Error (RPM)');
xlim(ax3, [0 max(t)]);
emax = max(abs(e))*1.15;
ylim(ax3, [-emax emax]);
text(ax3, 0.99, 0.92, sprintf('RMSE = %.2f RPM', rmse), 'Units', 'normalized', ...
    'HorizontalAlignment', 'right', 'FontSize', 10);

%% Formato comun
for ax = [ax1 ax2]
    ax.Box        = 'off';
    ax.XGrid      = 'on';
    ax.YGrid      = 'on';
    ax.GridColor  = cGrid;
    ax.GridAlpha  = 1;
    ax.LineWidth  = 0.8;
    ax.FontSize   = 10;
    ax.TickDir    = 'out';
end