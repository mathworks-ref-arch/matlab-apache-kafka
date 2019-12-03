function install(savePaths)
    % install Install paths for MATLAB Kafka Interface
    %
    % Running this command will add the necessary paths to MATLAB and save
    % them to the profile.
    %
    %   install(false)
    %
    % will add the paths but don't save them to the profile.
    
    % Copyright 2019 The MathWorks, Inc.

    if nargin < 1
        savePaths = true;
    end
    here = fileparts(mfilename('fullpath'));
    addpath(fullfile(here, 'system'), '-end');
    addpath(fullfile(here, 'system', 'blocks'), '-end');
    addpath(fullfile(here, 'system', 'kafkatarget', 'kafkatarget'), '-end');
    
    if savePaths
        savepath();
    end
end
