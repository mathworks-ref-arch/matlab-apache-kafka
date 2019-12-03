function varargout = sl_kafka_producer_cb(varargin)
    % sl_kafka_producer_cb Callbacks for mask

    %  Copyright 2019 The MathWorks, Inc.

    if nargout == 0
        feval(varargin{:});
    else
       [varargout{1:nargout}] = feval(varargin{:});
    end
end

function [conf, topicConf, confCombinedStr] = initialize(blk) %#ok<DEFNU>
   setDispString(blk);
   conf = kafka.utils.getCellArrayFromConfString(get_param(blk, 'sfpConf'));
   topicConf = kafka.utils.getCellArrayFromConfString(get_param(blk, 'sfpTopicConf'));
   confCombinedStr = kafka.utils.getRTWStringFromCellArray([conf;topicConf]);

end

function setDispString(blk)
    dstr = 'fprintf(''Brokers: %s\nTopic: %s\n';
    argstr = 'sfpBrokers, sfpTopic';
    useExtKey = strcmpi('on', get_param(blk, 'sfpKeyAsInput'));
    if ~useExtKey
        dstr = [dstr, 'Key: %s\n'];
        argstr = [argstr, ', sfpKey'];
    end
    dstr = [dstr, 'TS: %s'];
    argstr = [argstr, ', sfpTS'];

    D = [dstr, ''', ...', newline,  argstr, ');', newline];
    D = [D, ...
        'port_label(''input'', 1, ''Msg'');', newline];
    if useExtKey
        D = [D, ...
            'port_label(''input'', 2, ''Key'');', newline];
    end
    oldD = get_param(blk, 'MaskDisplay');
    if ~strcmp(oldD, D)
        set_param(blk, 'MaskDisplay', D);
    end

end

function toggleInputKey(blk) %#ok<DEFNU>
    pn = 'sfpKeyAsInput';
    useInput =get_param(blk, pn);
    idx = getMaskParamIndex(blk, pn);
    if ~isscalar(idx)
        error('Bad parameter name (%s) for block %s\n', pn, blk);
    end
    me = get_param(blk, 'MaskEnables');
    idx2 = getMaskParamIndex(blk, 'sfpKey');
    idx3 = getMaskParamIndex(blk, 'sfpKeyAsInputLen');
    if strcmp(useInput, me{idx2})
        % If they're the same, we must change the setting for the other.
        if strcmp(useInput, 'on')
            me{idx2} = 'off';
            me{idx3} = 'on';
        else
            me{idx2} = 'on';
            me{idx3} = 'off';
        end
        set_param(blk, 'MaskEnables', me);
    end

end

function idx = getMaskParamIndex(blk, prmName)
   mn = get_param(blk, 'MaskNames');
   idx = find(strcmp(mn, prmName));
end
