classdef NextEventTest < TestBase
    methods
        function verifyFields(t, actualCode, actualTime, actualData, ...
                              expectedCode, expectedTime, expectedData)
            t.verifyEqual(actualCode, int32(expectedCode));
            t.verifyEqual(actualTime, int64(expectedTime));
            t.verifyEqual(actualData, expectedData);
        end

        function verifyStruct(t, evt, expectedCode, expectedTime, expectedData)
            t.verifyClass(evt, 'struct');
            t.verifyEqual(fieldnames(evt), {'event_code'; 'time_us'; 'data'});
            t.verifySize(evt, [1, 1]);
            t.verifyFields(evt.event_code, evt.time_us, evt.data, ...
                           expectedCode, expectedTime, expectedData);
        end

        function verifyEmptyFields(t, code, time, data)
            t.verifyFields(code, time, data, [], [], []);
        end

        function verifyEmptyStruct(t, evt)
            t.verifyClass(evt, 'struct');
            t.verifyEqual(fieldnames(evt), {'event_code'; 'time_us'; 'data'});
            t.verifySize(evt, [0, 0]);
        end
    end

    methods (Test)
        function testBadNumberOfOutputs(t)
            f = mworks.MWKFile(t.getFilename);

            function zeroOutputs
                f.nextEvent;
            end
            function twoOutputs
                [a, b] = f.nextEvent;
            end
            function fourOutputs
                [a, b, c, d] = f.nextEvent;
            end

            t.verifyError(@zeroOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@twoOutputs, 'MWorks:WrongNumberOfOutputs');
            t.verifyError(@fourOutputs, 'MWorks:WrongNumberOfOutputs');
        end

        function testNoSelection(t)
            f = mworks.MWKFile(t.getFilename);

            [code, time, data] = f.nextEvent;
            t.verifyEmptyFields(code, time, data);

            evt = f.nextEvent;
            t.verifyEmptyStruct(evt);
        end

        function testEmptySelection(t)
            f = mworks.MWKFile(t.getFilename);

            f.selectEvents(intmax('int32'));
            [code, time, data] = f.nextEvent;
            t.verifyEmptyFields(code, time, data);

            f.selectEvents([], intmax('int64'));
            evt = f.nextEvent;
            t.verifyEmptyStruct(evt);
        end

        function testSingleSelectedEvent(t)
            f = mworks.MWKFile(t.getFilename);
            tagMap = t.getTagMap();

            f.selectEvents(tagMap.int_neg);
            [code, time, data] = f.nextEvent;
            t.verifyFields(code, time, data, ...
                           tagMap.int_neg, tagMap.int_neg, int64(-2));
            [code, time, data] = f.nextEvent;
            t.verifyEmptyFields(code, time, data);

            f.selectEvents(tagMap.str_text);
            evt = f.nextEvent;
            t.verifyStruct(evt, tagMap.str_text, tagMap.str_text, 'foo bar');
            evt = f.nextEvent;
            t.verifyEmptyStruct(evt);
        end

        function testMutlipleSelectedEvents(t)
            f = mworks.MWKFile(t.getFilename);
            tagMap = t.getTagMap();

            f.selectEvents([tagMap.int_neg, tagMap.float_neg, tagMap.str_text]);
            [code, time, data] = f.nextEvent;
            t.verifyFields(code, time, data, ...
                           tagMap.int_neg, tagMap.int_neg, int64(-2));
            [code, time, data] = f.nextEvent;
            t.verifyFields(code, time, data, ...
                           tagMap.float_neg, tagMap.float_neg, -2.5);
            [code, time, data] = f.nextEvent;
            t.verifyFields(code, time, data, ...
                           tagMap.str_text, tagMap.str_text, 'foo bar');
            [code, time, data] = f.nextEvent;
            t.verifyEmptyFields(code, time, data);

            f.selectEvents([tagMap.int_min, tagMap.str_binary, ...
                            tagMap.list_simple]);
            evt = f.nextEvent;
            t.verifyStruct(evt, tagMap.int_min, tagMap.int_min, ...
                           intmin('int64'));
            evt = f.nextEvent;
            t.verifyStruct(evt, tagMap.str_binary, tagMap.str_binary, ...
                           uint8([102, 111, 111, 0, 98, 97, 114]));
            evt = f.nextEvent;
            t.verifyStruct(evt, tagMap.list_simple, tagMap.list_simple, ...
                           {int64(1)});
            evt = f.nextEvent;
            t.verifyEmptyStruct(evt);
        end
    end
end
