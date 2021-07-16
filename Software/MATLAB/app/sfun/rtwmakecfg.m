function makeInfo = rtwmakecfg()
    %RTWMAKECFG Add include and source directories to RTW make files.
    %
    %  MAKEINFO = RTWMAKECFG returns a structured array containing
    %  following fields:
    %
    %     makeInfo.includePath - cell array containing additional include
    %                            directories. Those directories will be
    %                            expanded into include instructions of rtw
    %                            generated make files.
    %
    %     makeInfo.sourcePath  - cell array containing additional source
    %                            directories. Those directories will be
    %                            expanded into rules of rtw generated make
    %                            files.
    %
    %     makeInfo.library     - structure containing additional runtime library
    %                            names and module objects.  This information
    %                            will be expanded into rules of rtw generated make
    %                            files.
    %
    % Copyright 2013-2019 The MathWorks, Inc.
    
    
    srcDir = kafka.getRoot('app', 'sfun', 'src');
    rdkafkaSrcDir = kafka.getRoot('..', 'CPP', 'librdkafka', 'src');
    janssonSrcDir = kafka.getRoot('..', 'CPP', 'jansson', 'src');
    makeInfo.sourcePath = { ...
        srcDir, ...
        };
    makeInfo.includePath = { ...
        srcDir, ...
        rdkafkaSrcDir, ...
        janssonSrcDir, ...
        };
    
    makeInfo.sources = { ...
        };
    
    makeInfo.linkLibsObjs = { ...
        };
    
end




