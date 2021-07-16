function ret = isDebianBased(baseImage)
    % isDebianBased
    
    % Copyright 2019 The MathWorks, Inc.

    ret = strncmp(baseImage, 'ubuntu', 6) || strncmp(baseImage, 'debian', 6);
end
    