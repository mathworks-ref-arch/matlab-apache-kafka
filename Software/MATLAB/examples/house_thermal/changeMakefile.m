function changeMakefile(fileName)
   
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
    kafka.utils.changeEntriesInFile(fileName, fileNameOut, {c1});
    
end

%     c1 = {['(', ... path token
%         mlStart, '[^ ]*', fs, ... The path
%         ')', ... path token end
%         '(', ... filename token
%         '[^', fs, ']+', ext, ... 
%         ')' ... filename token end 
%         ], ...
%         '$2'};
%     c1 = {['(', mlStart, '.*', fs, '[^', fs, ']+', ext, ')'], ...
%         '$1'};
