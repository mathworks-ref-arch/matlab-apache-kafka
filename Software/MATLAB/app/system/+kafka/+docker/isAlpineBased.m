function ret = isAlpineBased(baseImage)
    % isAlpineBased Check if a build is based on alpine
    
    % Copyright 2019 The MathWorks, Inc.
    ret = strncmp(baseImage, 'alpine', 6);
end
    