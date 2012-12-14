function testGetCodecs
codec = getCodecs(getFilename());

assertTrue(isvector(codec));
assertEqual(1, length(codec));
assertTrue(isstruct(codec));
assertEqual(2, length(fieldnames(codec)));

assertTrue(isfield(codec, 'time_us'));
assertInteger(0, codec.time_us);

assertTrue(isfield(codec, 'codec'));
codec = codec.codec;

assertTrue(isvector(codec));
assertEqual(20, length(codec));
assertTrue(isstruct(codec));
assertEqual(sort({'code', 'tagname', 'logging', 'defaultvalue', ...
                  'shortname', 'longname', 'editable', 'nvals', ...
                  'domain', 'viewable', 'persistant', 'groups'})', ...
            sort(fieldnames(codec)));

for item = codec
    for f = {'code', 'logging', 'editable', 'nvals', 'domain', ...
             'viewable', 'persistant'}
        value = getfield(item, f{1});
        assertTrue(isa(value, 'int64'));
        assertTrue(isscalar(value));
    end
    
    for f = {'tagname', 'shortname', 'longname'}
        value = getfield(item, f{1});
        assertTrue(ischar(value));
        assertFalse(isempty(value));
    end
    
    groups = item.groups;
    assertTrue(iscellstr(groups));
    assertFalse(isempty(groups));
end
