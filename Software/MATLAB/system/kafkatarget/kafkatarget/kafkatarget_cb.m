function kafkatarget_cb(hDlg, hSrc, action)
    % kafkatarget_cb Callback for Simulink Config settings
    %
    % Copyright 2019 The MathWorks, Inc.
    
    fprintf('%s: %s\n', mfilename, action);
    switch action
        case 'select'
            setve('ModelReferenceCompliant', 'on', false);
            setve('ERTCustomFileTemplate', 'kafkatarget_process.tlc', false);
            setve('TargetLangStandard', 'C99 (ISO)', false);
            setve('EnableMultiTasking', 'off', false);
            setve('ProdLongLongMode', 'on', false);
            enableToolchainCompliant();
        case 'activate'
        case 'postapply'
        otherwise
            error('Unknown action: "%s"\n', action);
    end
    
    function enableToolchainCompliant()
        % The following parameters enable toolchain compliance.
        slConfigUISetVal(hDlg, hSrc, 'UseToolchainInfoCompliant', 'on');
        slConfigUISetVal(hDlg, hSrc, 'GenerateMakefile','on');
        
        % The following parameters are not required for toolchain compliance.
        % But, it is recommended practice to set these default values and
        % disable the parameters (as shown).
        setve('RTWCompilerOptimization', 'off', false);
        setve('MakeCommand', 'make_rtw', false);
    end
    
    function setve(attrib, val, enab)
        slConfigUISetEnabled(hDlg, hSrc, attrib, true);
        slConfigUISetVal(hDlg, hSrc, attrib, val);
        slConfigUISetEnabled(hDlg, hSrc, attrib, enab);
    end
end
