function kafka_build_sfuns
    % kafka_build_sfuns Helper function to build Kafka s-functions

    %  Copyright 2019 The MathWorks, Inc.

    % Dependency folders
    jDir = kafka.getRoot('..' ,'CPP', 'jansson');
    
    % S-function paths
    here = kafka.getRoot('app', 'sfun');

    srcDir = fullfile(here, 'src');
    incDir = fullfile(here, 'inc');
    old = cd(srcDir);
    goBack = onCleanup(@() cd(old));

    % Arguments
    common_args = { ...
        ...'-v', ...
        '-g', ...
        ['-I', incDir], ...
        '-lfixedpoint', ...
        '-outdir', '..' };
    kafkaArgs = kafka.utils.getMexLibArgs();
    if ismac
        error('OSX support not yet added to this package.\n');
    elseif isunix
        jansson = {...
            ['-I"', fullfile(jDir, 'src'), '"'], ...
            ...['-L"', fullfile(jDir, 'src','.libs'), '"'], ...
            '-ljansson',...
            };
    elseif ispc
%         is64 = strcmp('PCWIN64', computer);
%         if is64
%             jDir = getenv('PROGRAMFILES');
%         else
%             jDir = getenv('ProgramFiles(x86)');
%         end
        jansson = { ...
            ['-I"', fullfile(jDir, 'src'), '"'], ...
            ['-I"', fullfile(jDir, 'build.win64', 'include' ), '"'], ...
            ['-L"', fullfile(jDir, 'lib'), '"'], ...
            '-ljansson.lib'};
    else
        error('Unknown platform\n');
    end

    sfuns = { ...
        {'sl_kafka_producer.c', 'mw_kafka_utils.c', 'mx_kafka_utils.c'}, ...
        {'sl_kafka_consumer.c', 'mw_kafka_utils.c', 'mx_kafka_utils.c'}, ...
        {'sf_decode_flat_json_object.cpp', jansson{:}} ...
        }; %#ok<CCAT>

    for k=1:length(sfuns)
        sfun = sfuns{k};
        if iscell(sfun)
            fprintf('Compiling %s ...\n', sfun{1});
            mex(common_args{:}, kafkaArgs{:}, sfun{:});
        else
            fprintf('Compiling %s ...\n', sfun);
            mex(common_args{:}, kafkaArgs{:}, sfun);
        end
    end

end
