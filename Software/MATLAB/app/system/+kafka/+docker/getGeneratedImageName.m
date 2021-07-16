function name = getGeneratedImageName(baseImage)
    % getGeneratedImageName Return the name of the image
    
    % Copyright 2019 The MathWorks, Inc.

    basename = kafka.docker.getBaseName();
    name = sprintf('%s-%s', basename, strrep(baseImage, ':', '_'));
end
    