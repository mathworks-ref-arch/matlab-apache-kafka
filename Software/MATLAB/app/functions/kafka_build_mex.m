function kafka_build_mex
    % kafka_build_mex Helper function to build mex files
    
    %  Copyright 2019 The MathWorks, Inc. */
    
    here = kafka.getRoot('app', 'sfun');
    
    clearKafkaInBaseWorkspace();
    srcDir = fullfile(here, 'src');
    incDir = fullfile(here, 'inc');
    old = cd(fullfile(here, 'src'));
    goBack = onCleanup(@() cd(old));
    
    
    common_args = { ...
        ... '-v', ...
        '-g', ...
        ['-I', incDir], ...
        '-outdir', '..' };
    kafkaArgs = kafka.utils.getMexLibArgs();
    
    mexfuns = { ...
        {'mx_kafka_producer.cpp', 'mw_kafka_utils.c', 'mx_kafka_utils.c'}, ...
        {'mx_kafka_consumer.cpp', 'mw_kafka_utils.c', 'mx_kafka_utils.c'}, ...
        };
    
    % Display the build command
    for k=1:length(mexfuns)
        curFun = mexfuns{k};
        if iscell(curFun)
            fprintf('Compiling %s ...\n', curFun{1});
            buildArgs = join([common_args, kafkaArgs, curFun{1}],' ');
            fprintf('mex %s \n', buildArgs{1});
            mex(common_args{:}, kafkaArgs{:}, curFun{:});
        else
            fprintf('Compiling %s ...\n', curFun);
            buildArgs = join([common_args, kafkaArgs, curFun],' ');
            fprintf('mex %s \n', buildArgs{1});
            mex(common_args{:}, kafkaArgs{:}, curFun);
        end
    end
end

function clearKafkaInBaseWorkspace()
   S = evalin('base', 'whos');
   idxP = strcmp('kafka.Producer', {S.class});
   idxC = strcmp('kafka.Consumer', {S.class});
   idx = idxP | idxC;
   if any(idx)
       fprintf('### Clearing kafka.Producer and kafka.Consumer objects in base workspace\n');
   clearStr = sprintf('clear %s', sprintf('%s ', S(idx).name));
   evalin('base', clearStr);
   end
end

