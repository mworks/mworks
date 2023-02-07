function tests = testNextEvent
    tests = functiontests(localfunctions);
end


function verifyFields(testCase, actualCode, actualTime, actualData, ...
                      expectedCode, expectedTime, expectedData)
    verifyEqual(testCase, actualCode, int32(expectedCode));
    verifyEqual(testCase, actualTime, int64(expectedTime));
    verifyEqual(testCase, actualData, expectedData);
end


function verifyStruct(testCase, evt, expectedCode, expectedTime, expectedData)
    verifyTrue(testCase, isstruct(evt));
    verifyEqual(testCase, fieldnames(evt), {'event_code'; 'time_us'; 'data'});
    verifySize(testCase, evt, [1, 1]);
    verifyFields(testCase, evt.event_code, evt.time_us, evt.data, ...
                 expectedCode, expectedTime, expectedData);
end


function verifyEmptyFields(testCase, code, time, data)
    verifyFields(testCase, code, time, data, [], [], []);
end


function verifyEmptyStruct(testCase, evt)
    verifyTrue(testCase, isstruct(evt));
    verifyEqual(testCase, fieldnames(evt), {'event_code'; 'time_us'; 'data'});
    verifySize(testCase, evt, [0, 0]);
end


function testBadNumberOfOutputs(testCase)
    f = mworks.MWKFile(getFilename);

    function zeroOutputs
        f.nextEvent;
    end
    function twoOutputs
        [a, b] = f.nextEvent;
    end
    function fourOutputs
        [a, b, c, d] = f.nextEvent;
    end

    verifyError(testCase, @zeroOutputs, 'MWorks:WrongNumberOfOutputs');
    verifyError(testCase, @twoOutputs, 'MWorks:WrongNumberOfOutputs');
    verifyError(testCase, @fourOutputs, 'MWorks:WrongNumberOfOutputs');
end


function testNoSelection(testCase)
    f = mworks.MWKFile(getFilename);

    [code, time, data] = f.nextEvent;
    verifyEmptyFields(testCase, code, time, data);

    evt = f.nextEvent;
    verifyEmptyStruct(testCase, evt);
end


function testEmptySelection(testCase)
    f = mworks.MWKFile(getFilename);

    f.selectEvents(intmax('int32'));
    [code, time, data] = f.nextEvent;
    verifyEmptyFields(testCase, code, time, data);

    f.selectEvents([], intmax('int64'));
    evt = f.nextEvent;
    verifyEmptyStruct(testCase, evt);
end


function testSingleSelectedEvent(testCase)
    f = mworks.MWKFile(getFilename);
    tagMap = getTagMap(testCase);

    f.selectEvents(tagMap.int_neg);
    [code, time, data] = f.nextEvent;
    verifyFields(testCase, code, time, data, ...
                 tagMap.int_neg, tagMap.int_neg, int64(-2));
    [code, time, data] = f.nextEvent;
    verifyEmptyFields(testCase, code, time, data);

    f.selectEvents(tagMap.str_text);
    evt = f.nextEvent;
    verifyStruct(testCase, evt, tagMap.str_text, tagMap.str_text, 'foo bar');
    evt = f.nextEvent;
    verifyEmptyStruct(testCase, evt);
end


function testMutlipleSelectedEvents(testCase)
    f = mworks.MWKFile(getFilename);
    tagMap = getTagMap(testCase);

    f.selectEvents([tagMap.int_neg, tagMap.float_neg, tagMap.str_text]);
    [code, time, data] = f.nextEvent;
    verifyFields(testCase, code, time, data, ...
                 tagMap.int_neg, tagMap.int_neg, int64(-2));
    [code, time, data] = f.nextEvent;
    verifyFields(testCase, code, time, data, ...
                 tagMap.float_neg, tagMap.float_neg, -2.5);
    [code, time, data] = f.nextEvent;
    verifyFields(testCase, code, time, data, ...
                 tagMap.str_text, tagMap.str_text, 'foo bar');
    [code, time, data] = f.nextEvent;
    verifyEmptyFields(testCase, code, time, data);

    f.selectEvents([tagMap.int_min, tagMap.str_binary, tagMap.list_simple]);
    evt = f.nextEvent;
    verifyStruct(testCase, evt, tagMap.int_min, tagMap.int_min, ...
                 intmin('int64'));
    evt = f.nextEvent;
    verifyStruct(testCase, evt, tagMap.str_binary, tagMap.str_binary, ...
                 uint8([102, 111, 111, 0, 98, 97, 114]));
    evt = f.nextEvent;
    verifyStruct(testCase, evt, tagMap.list_simple, tagMap.list_simple, ...
                 {int64(1)});
    evt = f.nextEvent;
    verifyEmptyStruct(testCase, evt);
end
