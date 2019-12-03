function ret = isImageAvailable(imgName)
    % isImageAvailable Check if a docker image exists
    
    % Copyright 2019 The MathWorks, Inc.
    
    str = sprintf('docker images -q %s', imgName);
    [r,s] = system(str, '-echo');
    if r
        error('Problem talking to docker daemon\n\t%s\n', s);
    end
    ret = ~isempty(s);
        
end
    