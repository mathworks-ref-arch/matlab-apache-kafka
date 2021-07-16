classdef Producer < handle
    % kafka.Producer MATLAB class for publishing messages to Apache Kafka
    
    % Copyright 2019, The MathWorks Inc.
    
    properties (Hidden, SetAccess = private)
        RK
        RK_TOPIC
    end
    properties (SetAccess = private)
        Brokers
        Topic
    end
    
    methods
        function this = Producer(brokers, topic, conf, topicConf)
            % Constructor
            if nargin < 4
                topicConf = {};
                if nargin < 3
                    conf = {};
                end
            end
            [this.RK, this.RK_TOPIC] = mx_kafka_producer('init', brokers, topic, conf, topicConf);
            this.Brokers = brokers;
            this.Topic = topic;
        end
        
        function [ret, msg] = publish(this, key, value)
            [ret, msg] = mx_kafka_producer('publish', this.RK, this.RK_TOPIC, int8(key), int8(value));
        end
        function [ret, msg] = publishWithTimestamp(this, key, value, posixTS)
            [ret, msg] = mx_kafka_producer('publish', this.RK, this.RK_TOPIC, int8(key), int8(value), posixTS);
        end
        
        function delete(this)
            if ~isempty(this.RK)
                mx_kafka_producer('term', this.RK, this.RK_TOPIC);
            end
        end
        
    end
    
    
end

