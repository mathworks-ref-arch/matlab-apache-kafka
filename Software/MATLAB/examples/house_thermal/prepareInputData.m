%% Import json data
AmbientTT = readInData('AmbientTemperature.json');
ForwardTT = readInData('ForwardFlowTemperature.json');
ReturnTT = readInData('ReturnFlowTemperature.json');


%% Prepare data as Simulink input
ForwardTTSim = prepareInput(ForwardTT);
AmbientTTSim = prepareInput(AmbientTT);
ReturnTTSim = prepareInput(ReturnTT);


% %% Data up to Dec 7th, 10:00
% idx = ForwardTT.TimeStep <= datetime(2018,12,10,10,0,0, 'TimeZone','UTC');
% tEnd = seconds(max(ForwardTTSim.TimeStep(idx)-min(ForwardTTSim.TimeStep(idx))));


%%
function tt = prepareInput(tt)
tStep = tt.TimeStep;
dur = tStep-tStep(1);
tt.TimeStep = dur;
end
