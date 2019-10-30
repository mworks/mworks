function verifyEvent(testCase, evt)
verifyTrue(testCase, isstruct(evt));
verifyTrue(testCase, isvector(evt));
verifyEqual(testCase, length(fieldnames(evt)), 3);
verifyTrue(testCase, all(arrayfun(@(e) isa(e.event_code, 'int32'), evt)));
verifyTrue(testCase, all(arrayfun(@(e) isa(e.time_us, 'int64'), evt)));
verifyTrue(testCase, isfield(evt, 'data'));
