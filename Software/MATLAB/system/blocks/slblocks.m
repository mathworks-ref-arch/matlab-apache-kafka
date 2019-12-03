function blkStruct = slblocks
    % slblocks Register blocks for Simulink Library Browser

    %  Copyright 2019 The MathWorks, Inc.


    % This function specifies that the library should appear
    % in the Library Browser
    % and be cached in the browser repository

    Browser.Library = 'sl_kafka_lib';
    % 'mylib' is the name of the library

    Browser.Name = 'Kafka Library';
    % 'My Library' is the library name that appears
    % in the Library Browser

    blkStruct.Browser = Browser;
end
