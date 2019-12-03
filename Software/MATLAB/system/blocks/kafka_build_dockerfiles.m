function kafka_build_dockerfiles
    % kafka_build_dockerfiles Deprecated, due to bad name.
    %
    % Use kafka_build_dockerimages instead.
    
    % Copyright 2019 The MathWorks, Inc.
    
    fprintf('The file "kafka_build_dockerfiles" is deprecated. Please switch all usage to "kafka_build_dockerimages"\n');
    kafka_build_dockerimages();
end 