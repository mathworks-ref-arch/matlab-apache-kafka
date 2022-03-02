function install(savePaths)
    % install Install paths for MATLAB Kafka Interface
    %
    % Running this command will add the necessary paths to MATLAB and save
    % them to the profile.
    %
    %   install(false)
    %
    % will add the paths but don't save them to the profile.

    % Copyright 2019-2022 The MathWorks, Inc.

    if nargin < 1
        savePaths = true;
    end
    here = fileparts(mfilename('fullpath'));

    old = cd(here);
    goBack = onCleanup(@() cd(old));

    % Call local startup
    startup;
    
    if savePaths
        savepath();
    end
end
