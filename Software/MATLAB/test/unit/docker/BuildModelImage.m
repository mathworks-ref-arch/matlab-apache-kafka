classdef BuildModelImage < matlab.unittest.TestCase
    % BuildModelImage Test cases for building docker images of model

    % Copyright 2019 The MathWorks, Inc.

    properties (TestParameter)
        baseImage = kafka.docker.getBaseImageNames();
    end


    properties
        MdlName = 'silly_test_name';
    end

    methods (TestMethodSetup)

        function addHelpers(testCase)
            import matlab.unittest.fixtures.TemporaryFolderFixture;
            import matlab.unittest.fixtures.CurrentFolderFixture;
            tempFolder = testCase.applyFixture(TemporaryFolderFixture);
            testCase.applyFixture(CurrentFolderFixture(tempFolder.Folder));
            removeDockerImage(testCase)
        end
    end

    methods (TestMethodTeardown)
        function tearDownMethod(this)
           removeDockerImage(this);
        end
    end

    methods (Test)
        function testBuilAndCreatDockerFile(this, baseImage)
            try
                load_system(this.MdlName);
                onFinish = onCleanup(@() bdclose(this.MdlName));
                set_param(this.MdlName, 'KafkaBuildDockerImage', 'on')
                set_param(this.MdlName, 'KafkaDockerBaseImage', baseImage)
                slbuild(this.MdlName);
                assertTrue(this, imageIsPresent(this), 'This should produce an image');
            catch ME
                assertTrue(this, false, 'Something wrong with building mex files.\n');
            end
        end

        function testBuilAndDontCreatDockerFile(this, baseImage)
            try
                load_system(this.MdlName);
                onFinish = onCleanup(@() bdclose(this.MdlName));
                set_param(this.MdlName, 'KafkaBuildDockerImage', 'off')
                set_param(this.MdlName, 'KafkaDockerBaseImage', baseImage)
                slbuild(this.MdlName);
                assertFalse(this, imageIsPresent(this), 'This should produce an image');
            catch ME
                assertTrue(this, false, 'Something wrong with building mex files.\n');
            end
        end

    end

    methods (Access = private)
        function removeDockerImage(this)
            sha1 = getSha1FromImage(this);
            if ~isempty(sha1)
                sysCmd = sprintf('docker rmi %s', sha1);
                [r,s] = system(sysCmd, '-echo') %#ok<NOPRT,ASGLU>
            end
        end

        function ret = imageIsPresent(this)
            sha1 = getSha1FromImage(this);
            ret = ~isempty(sha1);
        end

        function sha1 = getSha1FromImage(this)
            sysCmd = sprintf('docker images --filter=reference=''mathworks/*%s*'' -q', this.MdlName);
            [r,s] = system(sysCmd, '-echo');
            if r == 0
                sha1 = strip(s);
            else
                sha1 = '';
            end
        end

    end
end
