function ret = hasPhysModLibraries(buildInfo)
    % hasPhysModLibraries
    % Checks if the buildinfo object contains information pertaining to
    % physical modeling.

    % Copyright 2019 The MathWorks, Inc.

    if isempty(buildInfo.LinkObj)
        ret = false;
    else
        pm=regexp({buildInfo.LinkObj.Path}, 'toolbox/physmod', 'once');pm=[pm{:}];

        ret = ~isempty(pm);
    end
end
