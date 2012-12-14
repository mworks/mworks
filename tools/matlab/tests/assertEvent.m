function assertEvent(evt)
assertTrue(isstruct(evt));
assertTrue(isvector(evt));
assertEqual(3, length(fieldnames(evt)));
assertTrue(all(arrayfun(@(e) isa(e.event_code, 'int32'), evt)));
assertTrue(all(arrayfun(@(e) isa(e.time_us, 'int64'), evt)));
assertTrue(isfield(evt, 'data'));
