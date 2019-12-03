classdef BuildKafkaModel < matlab.unittest.TestCase
    % BuildKafkaModel  Test cases for Simulink/Embedded Coder

    % Copyright 2019 The MathWorks, Inc.

    properties
        Broker = 'localhost:29092';
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
        function testBuild1(this)
            try
                mdl = 'KP_01';
                load_system(mdl);
                slbuild(mdl);
            catch ME
                assertTrue(this, false, ...
                    sprintf('Problems build model "%s" with Kafka blocks.\n%s\n', mdl, ME.message));
            end
            bdclose(mdl);
        end

    end
end
