function data = getData(code)
if ischar(code)
    data = getData(double(getfield(getTagMap(), code)));
else
    e = getEvents(getFilename(), [code]);
    assertEvent(e);
    assertTrue(isscalar(e));
    assertTrue(isa(e.event_code, 'int32'));
    assertEqual(int32(code), e.event_code);
    assertInteger(code, e.time_us);
    data = e.data;
end
