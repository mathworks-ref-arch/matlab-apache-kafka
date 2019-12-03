function E = makeFPCEntry(useTime)
    % makeFPCEntry A helper function for testing   simple Kafka functions
    
    % Copyright 2019 The MathWorks, Inc.
    if nargin < 1
        useTime = false;
    end
    f = 30+randn(1)*5;
    p = 40+randn(1)*10;
    c = max(0,6+randn(1));
    if useTime
        T = now;
        nodays = 86400*(T-floor(T));
        secParts = nodays-floor(nodays);
        M = round(secParts*1000);
        E = struct('TS', T, 'M', M, 'Flow', f, 'Pressure', p, 'Current', c);
    else
        E = struct('Flow', f, 'Pressure', p, 'Current', c);
    end
end