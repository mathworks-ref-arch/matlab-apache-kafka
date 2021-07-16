function [buildStr, runStr, fullImageName] = generateDockerfileForCompilerApp(prjName)
    % generateDockerFile - Generate docker file for an app generated with compiler
    %
    
    % Copyright 2019 The MathWorks, Inc.
    
    
    S = getXMLInfo(prjName);
    
    dockerFile = fullfile(S.outDir, 'Dockerfile');
    SW = kafka.utils.StringWriter(dockerFile);
    tag = S.version;
    fullImageName = sprintf('mathworks/%s:%s', S.appName, tag);
    
    buildStr = sprintf('docker build -t %s .', fullImageName);
    runStr = sprintf('docker run --init --network host -it --rm %s', fullImageName);
    
    SW.pf('# Copyright 2019, The MathWorks, Inc.\n\n');
    SW.pf('# Build with:\n#\t%s\n', buildStr);
    SW.pf('# Run with:\n#\t%s\n', runStr);
    SW.pf('FROM mathworks/librdkafka-ubuntu_18.04:0.2.2 as base\n\n');
    SW.pf('RUN apt-get update && \\\n');
    SW.pf('    apt-get install -y \\\n');
    SW.pf('    libxt6 \\\n');
    SW.pf('    && rm -rf /var/lib/apt/lists/*\n\n');
    SW.pf('# The installer image, where we install the MATLAB application\n');
    SW.pf('FROM mathworks/librdkafka-ubuntu_18.04-dev:0.2.2 as installer\n\n');
    SW.pf('# Install packaged matlab installer and script\n');
    SW.pf('RUN apt-get update && \\\n');
    SW.pf('    apt-get install -y \\\n');
    SW.pf('    unzip \\\n');
    SW.pf('    && rm -rf /var/lib/apt/lists/*\n');
    SW.pf('COPY %s.install /tmp/\n\n', S.mcrName);
    SW.pf('WORKDIR /tmp\n\n');
    SW.pf('RUN ./%s.install -mode silent -agreeToLicense yes\n\n', S.mcrName);
    
    SW.pf('# The final image\n');
    SW.pf('FROM base as deployment\n\n');

    SW.pf('COPY --from=installer /usr/local/MATLAB /usr/local/MATLAB \n');
    SW.pf('COPY --from=installer /usr/MathWorks /usr/MathWorks\n');
    SW.pf('COPY --from=installer /usr/local/include  /usr/local/include\n');
    SW.pf('COPY --from=installer /usr/local/lib  /usr/local/lib\n\n');
    SW.pf('RUN ldconfig\n\n');
    SW.pf('# # Configure MATLAB runtime enviroment\n');
    SW.pf('ENV MATLAB_INSTALL=/usr/local/MATLAB/MATLAB_Runtime/v97\n');
    SW.pf('ENV LD_LIBRARY_PATH=$MATLAB_INSTALL/runtime/glnxa64:$MATLAB_INSTALL/bin/glnxa64:$MATLAB_INSTALL/sys/os/glnxa64:$MATLAB_INSTALL/sys/java/jre/glnxa64/jre/lib/amd64/native_threads:$MATLAB_INSTALL/sys/java/jre/glnxa64/jre/lib/amd64/server:$MATLAB_INSTALL/sys/java/jre/glnxa64/jre/lib/amd64\n\n');

    SW.pf('WORKDIR /usr/MathWorks/%s/application\n\n', S.appName);

    SW.pf('LABEL maintainer="%s (%s)"\n', S.authorName, S.authorEmail);
    SW.pf('LABEL organization="%s"\n', S.company);
    SW.pf('LABEL creationdate="%s"\n', datestr(now,31));
    v = ver('matlab');
    SW.pf('LABEL matlabversion="%s %s"\n\n', v.Version, v.Release);

    SW.pf('CMD ["./%s"]\n', S.appName);
    
end


function S = getXMLInfo(prjName)
    fc = fileread(prjName);
    S.appName = getValue(fc, '<param.appname>', '</param.appname>');
    S.mcrName = getValue(fc, '<param.package.mcr.name>', '</param.package.mcr.name>');
    S.authorName = getValue(fc, '<param.authnamewatermark>', '</param.authnamewatermark>');
    S.authorEmail = getValue(fc, '<param.email>', '</param.email>');
    S.company = getValue(fc, '<param.company>', '</param.company>');
    S.runtimeVersion = getValue(fc, '${LD_LIBRARY_PATH}:MR/', '/runtime');
    S.version = getValue(fc, '<param.version>', '</param.version>');
    S.projectRoot = fileparts(which(prjName));
    S.outDir = fullfile(S.projectRoot, getValue(fc, '<param.output>${PROJECT_ROOT}/', '</param.output>'));
end

function v = getValue(str, pre, post)
    rx1 = regexptranslate('escape', pre);
    rx2 = regexptranslate('escape', post);
    rx = [rx1, '(.+)', rx2];
    val = regexp(str, rx, 'tokens', 'once');
    v = val{1};
end
