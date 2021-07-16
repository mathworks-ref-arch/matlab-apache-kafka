classdef Consumer < handle
    % Consumer MATLAB class for consuming messages from Apache Kafka

    % Copyright 2019, The MathWorks Inc.

    properties (Hidden, SetAccess = private)
        RK
    end
    properties (SetAccess = private)
        Brokers
        Topic
        Group
    end

    methods
        function this = Consumer(brokers, topic, group, conf, topicConf)
            % Constructor
            if nargin < 5
                topicConf = {};
                if nargin < 4
                    conf = {};
                end
            end
            this.RK = mx_kafka_consumer('init', brokers, topic, group, conf, topicConf);
            this.Brokers = brokers;
            this.Topic = topic;
            this.Group = group;
        end

        function [key, val, errMsg] = consume(this)
            [key, val, errMsg] = mx_kafka_consumer('consume', this.RK);
        end

        function delete(this)
            if ~isempty(this.RK)
                mx_kafka_consumer('term', this.RK);
            end
        end

    end


end
