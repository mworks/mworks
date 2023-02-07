function tests = testMWKFile
    tests = functiontests(localfunctions);
end


function testUnopenedFile(testCase)
    f = mworks.MWKFile;
    function nextEvent()
        evt = f.nextEvent;
    end
    function getEvents()
        evts = f.getEvents;
    end

    verifyEqual(testCase, f.Filename, '');
    % Setting filename after construction does *not* open the file
    f.Filename = getFilename;

    for i = 1:2
        verifyEqual(testCase, f.Filename, getFilename);

        verifyEqual(testCase, f.Codec, []);
        verifyEqual(testCase, f.ReverseCodec, []);

        verifyError(testCase, @() f.getNumEvents, 'MWorks:MWKFileError');
        verifyError(testCase, @() f.getMinTime, 'MWorks:MWKFileError');
        verifyError(testCase, @() f.getMaxTime, 'MWorks:MWKFileError');
        verifyError(testCase, @() f.selectEvents, 'MWorks:MWKFileError');

        verifyError(testCase, @nextEvent, 'MWorks:MWKFileError');

        verifyError(testCase, @getEvents, 'MWorks:MWKFileError');

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


function testFilenameNotText(testCase)
    verifyError(testCase, @() mworks.MWKFile(3), ...
                'MATLAB:validators:mustBeText');
end


function testFilenameNotChar(testCase)
    f = mworks.MWKFile;
    f.Filename = string(getFilename);
    % open will convert filename to char before calling mworks.mwfeval
    f.open;
end


function testNonexistentFile(testCase)
    verifyError(testCase, @() mworks.MWKFile('not_a_file.mwk'), ...
                'MWorks:MWKFileError');
end


function testAggregateProperties(testCase)
    f = mworks.MWKFile(getFilename);
    expectedNumEvents = length(fieldnames(getTagMap(testCase))) + 2;

    verifyEqual(testCase, f.getNumEvents, uint64(expectedNumEvents));
    verifyEqual(testCase, f.getMinTime, int64(0));
    verifyEqual(testCase, f.getMaxTime, int64(expectedNumEvents - 1));
end
