function data = getData(testCase, code)
if ischar(code)
    data = getData(testCase, double(getfield(getTagMap(testCase), code)));
else
    e = getEvents(getFilename(), [code]);
    verifyEvent(testCase, e);
    verifyTrue(testCase, isscalar(e));
    verifyTrue(testCase, isa(e.event_code, 'int32'));
    verifyEqual(testCase, e.event_code, int32(code));
    verifyInteger(testCase, e.time_us, code);
    data = e.data;
end
