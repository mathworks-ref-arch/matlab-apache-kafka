function copyPMFiles
    % copyPMFiles - Helper function to produce physical modeling images

    % Copyright 2019 The MathWorks, Inc.

    here = fileparts(mfilename('fullpath'));
    oldD = cd(here);
    goBack = onCleanup(@() cd(oldD));


    args = getPMLibs();

    N = length(args);
    makeLines = '';
    SW = kafka.utils.StringWriter();
    for k = 1:3:N
        name = args{k};
        src = fullfile(args{k+1}, computer('arch'));
        dirName = fullfile(here, 'src', name);
        makeNewDir(dirName);
        makeLines = [makeLines, ...
            copySources(fullfile(src, '*.c'), dirName)]; %#ok<AGROW>
        copySources(fullfile(src, '*.h'), dirName);
        SW.pf('%%.c.o : %s/%%.c\n', name);
        SW.pf('\t$(CC) $(CFLAGS) -o "$@" "$<"\n');
        SW.pf('%%.cpp.o : %s/%%.cpp\n', name);
        SW.pf('\t$(CPP) $(CPPFLAGS) -o "$@" "$<"\n\n');
    end
    compRuleLines = SW.getString();
    libRuleLines = makeLibRuleLines(args(1:3:end));
    makeMakefileFromTemplate(makeLines, compRuleLines, libRuleLines, makeLibIncludes(args(1:3:end)));
end

function makeMakefileFromTemplate(makeLines, compLines, ruleLines, incLine)
    str = fileread(fullfile('src', 'pm.mk.template'));
    str = strrep(str, 'PHYSMOD_LIB_OBJ_PLACEHOLDER', makeLines);
    str = strrep(str, 'PHYSMOD_COMP_RULES_PLACEHOLDER', compLines);
    str = strrep(str, 'PHYSMOD_LIB_RULES_PLACEHOLDER', ruleLines);
    str = strrep(str, 'PHYSMOD_INCLUDES_PLACEHOLDER', incLine);
    fh = fopen(fullfile('src', 'pm.mk'), 'w');
    if fh < 0
        error('Problems opening file for writing.');
    end
    closeAfter = onCleanup(@() fclose(fh));
    fprintf(fh, '%s', str);

end

function LN = getLibName(plainName)
    LN = ['LIB', upper(plainName), '_STD_OBJS'];
end

function incs = makeLibIncludes(libNames)
    incs = sprintf('-I./%s ', libNames{:});
end

function ruleLines = makeLibRuleLines(libNames)
    SW = kafka.utils.StringWriter();
    for k=1:length(libNames)
        L = libNames{k};
        LN = getLibName(L);
        SW.pf('%s_std.a : $(%s)\n', L, LN);
        SW.pf('\t@echo "### Creating static library $@ ..."\n');
        SW.pf('\t$(AR) $(ARFLAGS)  $@ $(%s)\n\n', LN);
    end
    ruleLines = SW.getString();
end

function makeLine = copySources(srcPath, dirName)
    srcFiles = dir(srcPath);
    for k=1:length(srcFiles)
        copyfile(fullfile(srcFiles(k).folder, srcFiles(k).name), dirName);
    end
    if nargout > 0
        [~,plainName] = fileparts(dirName);
        libName = getLibName(plainName);
        srcNames = {srcFiles.name};
        makeLine = sprintf('%-30s=%s\n', libName, ...
            separateLines(srcNames, ' ./%s.o', 5));
%             sprintf(' ./%s.o', srcNames{:}));
    end
end

function str = separateLines(items, fmt, stepSize)
    SW = kafka.utils.StringWriter();
    N = length(items);
    spaces = '    ';
    SW.pf(' \\\n%s', spaces);
    for k=1:stepSize:N
        s1 = k;
        s2 = min(k+stepSize-1, N);
        SW.pf(fmt, items{s1:s2});
        if s2 < N
            SW.pf(' \\\n%s', spaces);
        else
            SW.pf('\n');
        end
    end
    str = SW.getString();
end

function makeNewDir(dirName)
    if exist(dirName, 'dir')
        rmdir(dirName, 's');
    end
    mkdir(dirName);

end

function args = getPMLibs()
    args = { 'ssc_sli', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'engine', 'sli', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'engine', 'sli', 'lib'), ...
        'ssc_core', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'engine', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'engine', 'core', 'lib'), ...
        'ssc_comp', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'compiler', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'simscape', 'compiler', 'core', 'lib'), ...
        'ne', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'network_engine', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'network_engine', 'lib'), ...
        'mc', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'math', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'math', 'core', 'lib'), ...
        'lang', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'lang', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'lang', 'core', 'lib'), ...
        'ex', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'external', 'library', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'external', 'library', 'lib'), ...
        'pm', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'foundation', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'common', 'foundation', 'core', 'lib'), ...
        'mech', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'mech', 'c' ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'mech', 'lib' ), ...
        'sm_ssci', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'sm', 'ssci', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'sm', 'ssci', 'lib'), ...
        'sm', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'sm', 'core', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'sm', 'core', 'lib'), ...
        'pm_math', ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'pm_math', 'c'  ), ...
        fullfile( matlabroot, 'toolbox', 'physmod', 'pm_math', 'lib') ...
        };
end
