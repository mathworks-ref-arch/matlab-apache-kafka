function zipName = kafka_create_offline_installer
    % kafka_create_offline_installer Create offline installer
    %
    % This function will package the MATLAB Kafka Client solution in a form
    % that can easily be installed on a second computer.
    
    % Copyright 2021 The MathWorks, Inc.
   
    root = fileparts(fileparts(kafka.getRoot));
    zipName = sprintf('%s_%s.zip', root, datestr(now,30));
    
    old = cd(root);
    goBack = onCleanup(@() cd(old));
    files = getZipFiles();

    check_binaries();
    
    zip(zipName, files);
    
    if nargout == 0
        fprintf('Created zip file "%s"\n', zipName);
    end
end

function check_binaries()
    libs = ["jansson", "rdkafka"];
    dlls = ["rdkafka"];
    if ispc
        libExt = ".lib";
        dllExt = ".dll";
    elseif isunix
        libExt = ".a";
        dllExt = ".so";
    end
    libs = libs + libExt;
    dlls = dlls + dllExt;
    
    check_files(kafka.getRoot('app', 'sfun', 'lib'), libs);
end

function check_files(folder, files)
   
    for k=1:numel(files)
        fullname = fullfile(folder, files(k));
       if exist(fullname, 'file') == 0
           error("Kafka:Error", "The file %s is missing", fullname);
       end
    end
end

function files = getZipFiles()
       files = dir(".");
 
       files = string({files.name});
       keepIdx = [];
       for k=1:length(files)
           F = files(k);
           if F == "." || F == ".."
               continue;
           end
           if F == "Internal"
               continue;
           end
           if startsWith(F, ".git")
               continue;
           end
           keepIdx(end+1)=k; %#ok<AGROW>
       end
       files = files(keepIdx);
end



    