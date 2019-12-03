function kafka_build_mex
    % kafka_build_mex Helper function to build mex files
    
    %  Copyright 2019 The MathWorks, Inc. */
    
    here = fileparts(mfilename('fullpath'));
    
    srcDir = fullfile(here, 'src');
    incDir = fullfile(here, 'inc');
    old = cd(fullfile(here, 'src'));
    goBack = onCleanup(@() cd(old));
    
    
    common_args = { ...
        ... '-v', ...
        '-g', ...
        ['-I', incDir], ...
        '-R2018a', ...
        '-outdir', '..' };
    kafkaArgs = kafka.utils.getMexLibArgs();
    
    mexfuns = { ...
        {'mx_kafka_producer.cpp', 'mw_kafka_utils.c', 'mx_kafka_utils.c'}, ...
        };
    
    for k=1:length(mexfuns)
        curFun = mexfuns{k};
        if iscell(curFun)
            fprintf('Compiling %s ...\n', curFun{1});
            mex(common_args{:}, kafkaArgs{:}, curFun{:});
        else
            fprintf('Compiling %s ...\n', curFun);
            mex(common_args{:}, kafkaArgs{:}, curFun);
        end
    end
end
