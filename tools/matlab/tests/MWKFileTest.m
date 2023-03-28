classdef MWKFileTest < TestBase
    methods (Test)
        function testUnopenedFile(t)
            f = mworks.MWKFile;
            function nextEvent()
                evt = f.nextEvent;
            end
            function getEvents()
                evts = f.getEvents;
            end

            t.verifyEqual(f.Filename, '');
            % Setting filename after construction does *not* open the file
            f.Filename = t.getFilename;

            for i = 1:2
                t.verifyEqual(f.Filename, t.getFilename);

                t.verifyEqual(f.Codec, []);
                t.verifyEqual(f.ReverseCodec, []);

                t.verifyError(@() f.getNumEvents, 'MWorks:MWKFileError');
                t.verifyError(@() f.getMinTime, 'MWorks:MWKFileError');
                t.verifyError(@() f.getMaxTime, 'MWorks:MWKFileError');
                t.verifyError(@() f.selectEvents, 'MWorks:MWKFileError');

                t.verifyError(@nextEvent, 'MWorks:MWKFileError');

                t.verifyError(@getEvents, 'MWorks:MWKFileError');

                % OK to close unopened file (no-op)
                f.close;

                % Open file
                f.open;

                % OK to open opened file (no-op)
                f.open;

                % Re-close file, then try tests again
                f.close;
            end
        end

        function testFilenameNotText(t)
            t.verifyError(@() mworks.MWKFile(3), ...
                          'MATLAB:validators:mustBeText');
        end

        function testFilenameNotChar(t)
            f = mworks.MWKFile;
            f.Filename = string(t.getFilename);
            % open will convert filename to char before calling mworks.mwfeval
            f.open;
        end

        function testNonexistentFile(t)
            t.verifyError(@() mworks.MWKFile('not_a_file.mwk'), ...
                          'MWorks:MWKFileError');
        end

        function testAggregateProperties(t)
            f = mworks.MWKFile(t.getFilename);
            expectedNumEvents = length(fieldnames(t.getTagMap())) + 2;

            t.verifyEqual(f.getNumEvents, uint64(expectedNumEvents));
            t.verifyEqual(f.getMinTime, int64(0));
            t.verifyEqual(f.getMaxTime, int64(expectedNumEvents - 1));
        end
    end
end
