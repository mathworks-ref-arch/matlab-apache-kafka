function name = getReleaseName()
    % getReleaseName - Helper function to get the release name in lowercase
    
    % Copyright 2019, The MathWorks Inc.

    r = ver('matlab');
    n = regexp(r.Release, '\(([^)]+)\)', 'tokens', 'once');
    name = lower(n{1});
    
end
