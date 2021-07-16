classdef BuildBinaries < matlab.unittest.TestCase
    % BuildBinaries Test cases for building binary parts

    % Copyright 2019 The MathWorks, Inc.

    methods (TestMethodSetup)

        function addHelpers(testCase)
            import matlab.unittest.fixtures.TemporaryFolderFixture;
            import matlab.unittest.fixtures.CurrentFolderFixture;
            tempFolder = testCase.applyFixture(TemporaryFolderFixture);
            testCase.applyFixture(CurrentFolderFixture(tempFolder.Folder));
        end
    end

    methods (Test)
        function testBuildMexFuns(this)
            try
                kafka_build_mex();
            catch ME %#ok<NASGU>
                assertTrue(this, false);
            end
        end

        function testBuildSFuns(this)
            try
                kafka_build_sfuns();
            catch ME
                assertTrue(this, false);
            end
        end

        function testBuildDockerfiles(this)
            try
                kafka_build_dockerimages();
            catch ME
                assertTrue(this, false);
            end
        end

    end
end
