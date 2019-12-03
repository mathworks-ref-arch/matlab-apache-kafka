function startup
    % startup Helper script to set paths and cd to examples
    
    % Copyright 2019 The MathWorks, Inc.

    if verLessThan('matlab', '9.4')
       error('This package only runs with MATLAB R2018a or newer\n'); 
    end
    install(false)
    here = fileparts(mfilename('fullpath'));
    cd(fullfile(here, 'examples'));
end
