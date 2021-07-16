function varargout = sl_kafka_consumer_cb(varargin)
    % sl_kafka_consumer_cb Callbacks for mask

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

    D = ['fprintf(''Brokers: %s\nTopic: %s\nGroup: %s\nTS: %s'', ...', ...
        newline, 'sfpBrokers, sfpTopic, sfpGroup, sfpTS)'];
    labels = {'fcn()', 'Msg', 'Msg len', 'Key', 'Key len'};

    if strcmpi('on', get_param(blk, 'sfpOutputTimestamp'))
        labels{end+1} = 'Timestamp';
    end
    for k=1:length(labels)
        D = [D, ...
            sprintf('\nport_label(''output'', %d, ''%s'');', k, labels{k})];
    end
    oldD = get_param(blk, 'MaskDisplay');
    if ~strcmp(oldD, D)
        set_param(blk, 'MaskDisplay', D);
    end

end


function idx = getMaskParamIndex(blk, prmName)
   mn = get_param(blk, 'MaskNames');
   idx = find(strcmp(mn, prmName));
end
