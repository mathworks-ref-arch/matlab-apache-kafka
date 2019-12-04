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
            catch ME
                assertTrue(this, false, 'Something wrong with building mex files.\n%s\n', ME.message);
            end
        end

        function testBuildSFuns(this)
            try
                kafka_build_sfuns();
            catch ME
                assertTrue(this, false, 'Something wrong with building s-functions.\n%s\n', ME.message);
            end
        end

        function testBuildDockerfiles(this)
            try
                kafka_build_dockerimages();
            catch ME
                assertTrue(this, false, 'Something wrong with building docker images.\n%s\n', ME.message);
            end
        end

    end
end
