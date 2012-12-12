function data = getData(code)
if ischar(code)
    data = getData(double(getfield(getTagMap(), code)));
else
    e = getEvents(getFilename(), [code]);
    assertEvent(e);
    assertTrue(isscalar(e));
    assertInteger(code, e.event_code);
    assertInteger(code, e.time_us);
    data = e.data;
end
