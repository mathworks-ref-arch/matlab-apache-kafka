function [buildStr, runStr, fullImageName] = generateDockerFile(modelName, buildInfo)
    % generateDockerFile - Generate docker file for kafkatarget.
    %

    % Copyright 2019 The MathWorks, Inc.

    rtwDirs = RTW.getBuildDir(modelName);
    buildDir = rtwDirs.BuildDirectory;
    old = cd(buildDir);
    goBack = onCleanup(@() cd(old));

    if nargin < 2
        load('buildInfo.mat', 'buildInfo');
    end
    
    zipName = 'modelfiles.zip';
    tic,buildInfo.packNGo({'packType', 'flat', 'fileName', zipName}),toc
    mdlFileDir = fullfile('.', 'modelfiles');
    if exist(mdlFileDir, 'dir')
        rmdir(mdlFileDir, 's');
    end
    
    tic,unzip(fullfile('..', zipName), mdlFileDir);toc
    copyfile('rtw_proj.tmw', mdlFileDir);

    SW = kafka.utils.StringWriter('Dockerfile');
    
    distImageBase = get_param(modelName, 'KafkaDockerBaseImage');
    fullDistImageBase = kafka.docker.getGeneratedImageName(distImageBase);

    imageVersion = get_param(modelName, 'KafkaDockerImageVersion');
    fullImageName = sprintf('%s-%s:%s', fullDistImageBase, lower(modelName), imageVersion);

    baseTag = kafka.docker.getVersion();
    
    fullBaseImage = sprintf('%s:%s', fullDistImageBase, baseTag);
    fullDevImage = sprintf('%s-dev:%s', fullDistImageBase, baseTag);
    
    buildStr = sprintf('docker build -t %s .', fullImageName);
    runStr = sprintf('docker run --init --network host -it --rm %s', fullImageName);
    
    SW.pf('# Dockerfile for model %s\n', modelName);
    SW.pf('# Build:\n#\t%s\n', buildStr);
    SW.pf('# Run:\n#\t%s\n\n', runStr);
    
    SW.pf('FROM %s AS installer\n\n', fullDevImage);
    
    SW.pf('ENV BUILD_DIR %s\n\n', buildDir);
    SW.pf('RUN mkdir -p $BUILD_DIR\n\n');
    SW.pf('COPY modelfiles $BUILD_DIR\n\n');
    
    if kafka.docker.hasPhysModLibraries(buildInfo)
        fullPMImage = sprintf('%s-pm-%s:%s', fullDistImageBase, kafka.utils.getReleaseName, baseTag);        
        SW.pf('COPY --from=%s  /pmlibs $BUILD_DIR\n\n', fullPMImage);
    end
    
    SW.pf('WORKDIR $BUILD_DIR\n\n');
    
    SW.pf('RUN make -f %s.mk\n\n', modelName);
    
    SW.pf('RUN mkdir /model && \\ \n\tmv $BUILD_DIR/../%s /model/\n\n', modelName);

    SW.pf('FROM %s\n\n', fullBaseImage);

    SW.pf('LABEL maintainer="Application Deployment (mwlab@mathworks.com)"\n\n');
    SW.pf('LABEL organization="The MathWorks, Inc."\n\n');
    SW.pf('LABEL creationdate="%s"\n\n', datestr(now,31));

    [u,e]=kafka.docker.getGitUserNameEmail();
    if ~isempty(u) && ~isempty(e)
        SW.pf('LABEL generatedby="%s (%s)"\n\n', u, e);
    end
    v = ver('matlab');
    SW.pf('LABEL matlabversion="%s %s"\n\n', v.Version, v.Release);
    
    SW.pf('COPY --from=installer /model /model\n\n');
    %     useThreads = strcmp('on', get_param(modelName, 'KafkaUseThreadsForTiming'));
    
    if kafka.docker.isDebianBased(distImageBase)        
        SW.pf('COPY --from=installer /usr/local/include  /usr/local/include\n\n');
        SW.pf('COPY --from=installer /usr/local/lib  /usr/local/lib\n\n');        
        SW.pf('RUN ldconfig\n\n');
    end
    SW.pf('WORKDIR /model\n\n');
    
    SW.pf('CMD ["/model/%s"]\n\n', modelName);
    
    
    clear('closeAfter');
        
    
end