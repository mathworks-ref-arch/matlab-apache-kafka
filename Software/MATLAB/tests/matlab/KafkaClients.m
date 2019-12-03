classdef KafkaClients < matlab.unittest.TestCase
    % KafkaClients  Test cases for MATLAB Kafka client

    % Copyright 2019 The MathWorks, Inc.

    properties
        Broker = 'localhost:29092';
    end

    methods (TestClassSetup)
        function classSetup(this)
            [r, s] = system('docker-compose up -d', '-echo');
            if r ~= 0
                error('Error starting Kafka.\n%s\n', s);
            end
            fprintf('Giving Kafka/Zookeeper a few seconds to start ...');
            pause(3);
            fprintf(' done!\n');
        end
    end
    methods (TestClassTeardown)
        function classTeardown(this)
            system('docker-compose down', '-echo');
        end
    end

    methods (TestMethodSetup)
        function addHelpers(testCase)
            import matlab.unittest.fixtures.TemporaryFolderFixture;
            import matlab.unittest.fixtures.CurrentFolderFixture;
            tempFolder = testCase.applyFixture(TemporaryFolderFixture);
            testCase.applyFixture(CurrentFolderFixture(tempFolder.Folder));
        end
    end

    methods (Test)
        function testSend(this)
            try
                P = kafka.Producer(this.Broker, 'test1');
                data = struct('a', pi, 'b', magic(3));
                P.publish(datestr(now,31), jsonencode(data));
            catch ME
                assertTrue(this, false, 'Problems with Kafka.\n%s\n', ME.message);
            end
        end

    end
end
