function buildBaseImages()
    % buildBaseImages Helper function to build docker base images
    
    % Copyright 2019 The MathWorks, Inc.
    
    BASES = kafka.docker.getBaseImageNames();
    
    tag = kafka.docker.getVersion();
    
    imgBaseDir = kafka.getRoot('app', 'system', 'dockerfiles');

    % Copy physical modeling files for fresh files
    old = cd(fullfile(imgBaseDir, 'pmlibrdkafka'));
    goBack = onCleanup(@() cd(old));
    copyPMFiles();
    clear('goBack');
    
    relName = kafka.utils.getReleaseName();
    
    for b = 1:length(BASES)
        B = BASES{b};
        BN = kafka.docker.getGeneratedImageName(B);
        BI = [BN, ':', tag];
        BDEV = [BN, '-dev:', tag];
        BPM = [BN, '-pm-', relName, ':', tag]; 
        if kafka.docker.isDebianBased(B)
            imgDir = fullfile(imgBaseDir, 'debian');
        elseif kafka.docker.isAlpineBased(B)
            imgDir = fullfile(imgBaseDir, 'alpine');
        else
            error('Cannot handle base image %s\n', B);
        end
        
        runDockerBuild( ...
            fullfile(imgDir, 'baselibrdkafka'), ...
            BI, {'BASEIMAGE', B});

        runDockerBuild( ...
            fullfile(imgDir, 'devlibrdkafka'), ...
            BDEV, {'BASEIMAGE', BI});

        % Build Physical Modeling libraries too
        runDockerBuild( ...
            fullfile(imgBaseDir, 'pmlibrdkafka'), ...
            BPM, {'BASEIMAGE', BI, 'BASEDEVIMAGE', BDEV});
        
    end
    
end

function runDockerBuild(folder, imageName, buildArgs)
    old = cd(folder);
    goBack = onCleanup(@() cd(old));
    N = length(buildArgs);
    BA = {};
    for k=1:2:N
       BA = [BA, ...
           '--build-arg', [buildArgs{k}, '=', buildArgs{k+1}]];
    end
    elems = [ ...
        'docker', 'build', ...
        '-t', imageName, ...
        BA, ...
        '.' ...
        ];

    sysCmd = sprintf('%s ', elems{:});
    [r,s] = system(sysCmd, '-echo');
end




