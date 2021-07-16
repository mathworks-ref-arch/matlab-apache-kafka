function consume_in_loop(timeout, brokers)
    % consume_in_loop Simple test function for listening with Kafka consumer
    
    % Copyright 2020 The MathWorks, Inc.
    
    if nargin < 2
        brokers = 'localhost:9092';
        if nargin < 1
            timeout = 10;
        end
    end
    C = kafka.Consumer(brokers, 'expo', 't-rex')
    n = 1;
    lastTime = tic;
    
    while true
        fprintf('.');
        [k,v,msg] = C.consume();
        if ~isempty(v)
            VI = min(length(v), 50);
            fprintf('#%02d: %s -- %s\n', n, k, v(1:VI));
            lastTime = tic;
        end
        if rem(n,50)==0
            fprintf(' - %d\n', n);
        end
        n = n + 1;
        if toc(lastTime) > timeout
            break;
        end
    end
    
    
    
end
