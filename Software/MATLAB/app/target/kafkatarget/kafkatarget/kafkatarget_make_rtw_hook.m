function kafkatarget_make_rtw_hook(hookMethod,modelName,rtwroot,templateMakefile,buildOpts,buildArgs, buildInfo)
    % kafkatarget_make_rtw_hook - This is the standard ERT hook file for the build
    % process (make_rtw), and implements automatic configuration of the
    % models configuration parameters.  When the buildArgs option is specified
    % as 'optimized_fixed_point=1' or 'optimized_floating_point=1', the model
    % is configured automatically for optimized code generation.
    %
    % This hook file (i.e., file that implements various codegen callbacks) is
    % called for system target file ert.tlc.  The file leverages
    % strategic points of the build process.  A brief synopsis of the callback
    % API is as follows:
    %
    % ert_make_rtw_hook(hookMethod, modelName, rtwroot, templateMakefile,
    %                   buildOpts, buildArgs)
    %
    % hookMethod:
    %   Specifies the stage of the build process.  Possible values are
    %   entry, before_tlc, after_tlc, before_make, after_make and exit, etc.
    %
    % modelName:
    %   Name of model.  Valid for all stages.
    %
    % rtwroot:
    %   Reserved.
    %
    % templateMakefile:
    %   Name of template makefile.  Valid for stages 'before_make' and 'exit'.
    %
    % buildOpts:
    %   Valid for stages 'before_make' and 'exit', a MATLAB structure
    %   containing fields
    %
    %   modules:
    %     Char array specifying list of generated C files: model.c, model_data.c,
    %     etc.
    %
    %   codeFormat:
    %     Char array containing code format: 'RealTime', 'RealTimeMalloc',
    %     'Embedded-C', and 'S-Function'
    %
    %   noninlinedSFcns:
    %     Cell array specifying list of non-inlined S-Functions.
    %
    %   compilerEnvVal:
    %     String specifying compiler environment variable value, e.g.,
    %     D:\Applications\Microsoft Visual
    %
    % buildArgs:
    %   Char array containing the argument to make_rtw.  When pressing the build
    %   button through the Configuration Parameter Dialog, buildArgs is taken
    %   verbatim from whatever follows make_rtw in the make command edit field.
    %   From MATLAB, it's whatever is passed into make_rtw.  For example, its
    %   'optimized_fixed_point=1' for make_rtw('optimized_fixed_point=1').
    %
    %   This file implements these buildArgs:
    %     optimized_fixed_point=1
    %     optimized_floating_point=1
    %
    % You are encouraged to add other configuration options, and extend the
    % various callbacks to fully integrate ERT into your environment.
    
    % Copyright 1996-2019 The MathWorks, Inc.
    
    switch hookMethod
        case 'error'
            % Called if an error occurs anywhere during the build.  If no error occurs
            % during the build, then this hook will not be called.  Valid arguments
            % at this stage are hookMethod and modelName. This enables cleaning up
            % any static or global data used by this hook file.
            msg = DAStudio.message('RTW:makertw:buildAborted', modelName);
            disp(msg);
        case 'entry'
            % Called at start of code generation process (before anything happens.)
            % Valid arguments at this stage are hookMethod, modelName, and buildArgs.
            msg = DAStudio.message('RTW:makertw:enterRTWBuild', modelName);
            disp(msg);
            
            
        case 'before_tlc'
            % Called just prior to invoking TLC Compiler (actual code generation.)
            % Valid arguments at this stage are hookMethod, modelName, and
            % buildArgs
            
        case 'after_tlc'
            % Called just after to invoking TLC Compiler (actual code generation.)
            % Valid arguments at this stage are hookMethod, modelName, and
            % buildArgs
            %     lxFolderSrc = fullfile(matlabroot, 'toolbox', 'target', 'codertarget', 'rtos', 'src');
            %     lxFolderInc = fullfile(matlabroot, 'toolbox', 'target', 'codertarget', 'rtos', 'inc');
            %     lxName = 'linuxinitialize.c';
            % %     lxFull = fullfile(lxFolder, lxName);
            %     buildInfo.addSourcePaths(lxFolderSrc);
            %     buildInfo.addIncludePaths(lxFolderInc);
            %     buildInfo.addSourceFiles(lxName, lxFolderSrc);
            
            % Next line needed to get in struct timespec and nanonsleep */
            buildInfo.addDefines('-D_POSIX_C_SOURCE=199309L');
            buildInfo.addDefines('-DSTACK_SIZE=64');
            
            
            useThreads = strcmp('on', get_param(modelName, 'KafkaUseThreadsForTiming'));
            if useThreads
                coderTargetLib = fullfile(matlabroot, ...
                    'toolbox', 'target', 'codertarget', 'rtos');
                
                buildInfo.addSourceFiles('linuxinitialize.c', fullfile(coderTargetLib, 'src'));
                buildInfo.addSourcePaths(fullfile(coderTargetLib, 'src'));
                
                buildInfo.addIncludeFiles('linuxinitialize.h', fullfile(coderTargetLib, 'inc'));
                buildInfo.addIncludePaths(fullfile(coderTargetLib, 'inc'));
            end
            
            
            
            
            mwKafkaSrcDir = kafka.getRoot('app', 'sfun', 'src');
            mwKafkaIncDir = kafka.getRoot('app', 'sfun', 'inc');
            buildInfo.addSourceFiles('mw_kafka_utils.c');
            buildInfo.addSourceFiles('sl_jansson_funs.c');
            buildInfo.addIncludeFiles('mw_kafka_utils.h', mwKafkaIncDir);
            buildInfo.addSourceFiles('ec_kafka_utils.c');
            buildInfo.addIncludeFiles('ec_kafka_utils.h', mwKafkaIncDir);
            buildInfo.addIncludePaths(mwKafkaSrcDir);
            buildInfo.addIncludePaths(mwKafkaIncDir);
            buildInfo.addSourcePaths(mwKafkaSrcDir);
            
            buildInfo.addSysLibs('jansson');
            buildInfo.addSysLibs('rdkafka');
            buildInfo.addSysLibs('pthread');
            buildInfo.addSysLibs('z');
            buildInfo.addSysLibs('rt');
            buildInfo.addSysLibs('ssl');
            buildInfo.addSysLibs('crypto');
            
            
        case 'before_make'
            % Called after code generation is complete, and just prior to kicking
            % off make process (assuming code generation only is not selected.)  All
            % arguments are valid at this stage.
            % pi
        case 'after_make'
            % Called after make process is complete. All arguments are valid at
            % this stage.
            [buildStr, runStr, fullImageName] = kafka.docker.generateDockerFile(modelName, buildInfo);
            changeMakefileEntries([modelName, '.mk']);
            doBuild = strcmp('on', get_param(modelName, 'KafkaBuildDockerImage'));
            if doBuild
                fprintf('### Building docker image "%s"...\n', fullImageName);
                [r,s] = system(buildStr, '-echo');
                disp(s);
                if r
                    error('Issues building docker image\n');
                else
                    % Show the size of the image
                    system(sprintf('docker images %s', fullImageName), '-echo')
                end
            end
            fprintf('### Build/rebuild Kafka image with:\n\t%s\n', buildStr);
            fprintf('### Run Kafka image with:\n\t%s\n', runStr);
            
        case 'exit'
            % Called at the end of the build process.  All arguments are valid
            % at this stage.
            if strcmp(get_param(modelName,'GenCodeOnly'),'off')
                msgID = 'RTW:makertw:exitRTWBuild';
            else
                msgID = 'RTW:makertw:exitRTWGenCodeOnly';
            end
            msg = DAStudio.message(msgID,modelName);
            disp(msg);
    end
    
end

function changeMakefileEntries(fileName)
    
    [fnP, fnN, fnE] = fileparts(fileName);
    fileNameOut = fullfile(fnP, 'modelfiles', [fnN, fnE]);
    
    mlStart = regexptranslate('escape', ...
        fullfile(matlabroot, 'toolbox', 'physmod'));
    if ispc
        ext = regexptranslate('escape', '.lib');
    else
        ext = regexptranslate('escape', '.a');
    end
    fs = regexptranslate('escape', filesep);
    c1 = {['(', ... path token
        mlStart, '[^ ]*', fs, ... The path
        '[^', fs, ']+', ext, ...
        ')' ... path token end
        ], ...
        ['(', ... path token
        mlStart, '[^ ]*', fs, ... The path
        ')', ... path token end
        '(', ... filename token
        '[^', fs, ']+', ext, ...
        ')' ... filename token end
        ]};
    
    %     fc = fileread(fileName);
    changeLibEntriesInFile(fileName, fileNameOut, {c1});
    changeHelperFileMakeRules(fileNameOut)
end

function ret = isCPP(mdl)
    ret = strcmp('C++', get_param(mdl, 'TargetLang'));
end

function changeHelperFileMakeRules(fileName)
    fc = fileread(fileName);
    sfunDir = fileparts(which('sl_kafka_consumer'));
    srcDir = fullfile(sfunDir, 'src');
    rx = regexptranslate('escape', [srcDir, filesep]);
    fc2 = regexprep(fc, rx, '');
    fh = fopen(fileName, 'w');
    if fh < 0
        error('Couldn''t open %s for writing.\n', fileName);
    end
    closeAfter = onCleanup(@() fclose(fh));
    fprintf(fh, '%s\n', fc2);
end

function changeLibEntriesInFile(inFileName, outFileName, changeList)
    % changeEntriesInFile Change entries in a file according to regex rules
    
    fc = fileread(inFileName);
    
    fh = fopen(outFileName, 'w');
    if fh < 0
        error('Uh oh\n');
    end
    %     dstDir = fileparts(outFileName);
    closeAfter = onCleanup(@() fclose(fh));
    C = changeList{1};
    tok = regexp(fc, C{1}, 'tokens');
    %     files = unique([tok{:}]);
    
    fc2 = regexprep(fc, C{2}, '$2');
    fprintf(fh, '%s', fc2);
    
    
end
