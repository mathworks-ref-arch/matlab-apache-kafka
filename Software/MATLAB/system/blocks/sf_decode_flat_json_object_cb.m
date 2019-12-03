function varargout = sf_decode_flat_json_object_cb(varargin)
    % sf_decode_flat_json_object_cb Callbacks for mask

    %  Copyright 2019 The MathWorks, Inc.

    if nargout == 0
        feval(varargin{:});
    else
        [varargout{1:nargout}] = feval(varargin{:});
    end
end


function fieldString = init(blk, sfpDirection, sfpFields ) %#ok<DEFNU>
    fieldString = getRTWStringFromVector(sfpFields);
    setDispString(blk, sfpDirection);
    setEnables(blk, sfpDirection);
end

function str = getRTWStringFromVector(fields)
    q = cellfun(@quoteStr, fields, 'Uni', false);
    q2 = join(q, ',');
    str = ['[', q2{1}, ']'];
end

function D = setDispString(blk, sfpDirection)

%     D = 'fprintf(''Fields: %s'', get_param(gcb, ''sfpFields''));';
    if sfpDirection == 1
        % DECODE
        D = 'disp(''JSON DECODE'');';
        DS = 'input';
        DD = 'output';
        if strcmp('on', get_param(blk, 'sfpInLength'))
            LastLabel = sprintf('port_label(''input'', 2, ''MsgLEn'')\n');
        else
            LastLabel = '';
        end
    else
        % ENCODE
        D = 'disp(''JSON ENCODE'');';
        DS = 'output';
        DD = 'input';
        if strcmp('on', get_param(blk, 'sfpOutLength'))
            LastLabel = sprintf('port_label(''output'', 2, ''MsgLEn'')\n');
        else
            LastLabel = '';
        end
    end
    D = sprintf('%s\nport_label(''%s'', 1, ''Msg'')\n', D, DS);
    fields = eval(get_param(blk, 'sfpFields'));
    N = length(fields);
    for k=1:N
        D = sprintf('%sport_label(''%s'', %d, ''%s'')\n', D, DD, k, fields{k});
    end
    D = [D, LastLabel];

    oldD = get_param(blk, 'MaskDisplay');
    if ~strcmp(oldD, D)
        set_param(blk, 'MaskDisplay', D);
    end
%
end

function setEnables(blk, sfpDirection)
    idx = findParamIndex(blk, {'sfpOutLength', 'sfpInLength'});
    visOld = get_param(blk, 'MaskVisibilities');
    visNew = visOld;
    if sfpDirection == 1
        % DECODE
        visNew{idx(1)} =   'off';
        visNew{idx(2)} =   'on';
    else
        % ENCODE
        visNew{idx(1)} =   'on';
        visNew{idx(2)} =   'off';
    end
    if ~isequal(visOld, visNew)
        set_param(blk, 'MaskVisibilities', visNew);
    end
%     set_param(blk, 'MaskEnables', {'on';'on';'on';'on';'on'});


end

function idx = findParamIndex(blk, name)
    maskNames = get_param(blk, 'MaskNames');
    if ischar(name)
        name = {name};
    end
    N = length(name);
    idx = -1*ones(1,N);
    for k=1:N
       idx(k) = find(strcmp(name{k}, maskNames));
    end


end
%
% function toggleSettings(blk)
%
% end
