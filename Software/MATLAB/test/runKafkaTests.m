function R = runKafkaTests()
    % runKafkaTests Run all unittests for the Apache Kafka package

    % Copyright 2019 The MathWorks, Inc.

    here = fileparts(mfilename('fullpath'));
    old = cd(here);
    goBack = onCleanup(@() cd(old));

    R = runtests('.', 'IncludeSubfolders', true);

end
