function tests = testGetCodecs
tests = functiontests(localfunctions);

function testAll(testCase)
codec = getCodecs(getFilename());

verifyTrue(testCase, isvector(codec));
verifyEqual(testCase, length(codec), 1);
verifyTrue(testCase, isstruct(codec));
verifyEqual(testCase, length(fieldnames(codec)), 2);

verifyTrue(testCase, isfield(codec, 'time_us'));
verifyInteger(testCase, codec.time_us, 0);

verifyTrue(testCase, isfield(codec, 'codec'));
codec = codec.codec;

verifyTrue(testCase, isvector(codec));
verifyEqual(testCase, length(codec), 20);
verifyTrue(testCase, isstruct(codec));
verifyEqual(testCase, sort(fieldnames(codec)), ...
            sort({'code', 'tagname', 'logging', 'defaultvalue', ...
                  'shortname', 'longname', 'editable', 'nvals', ...
                  'domain', 'viewable', 'persistant', 'groups'})');

for item = codec
    for f = {'code', 'logging', 'editable', 'nvals', 'domain', ...
             'viewable', 'persistant'}
        value = getfield(item, f{1});
        verifyTrue(testCase, isa(value, 'int64'));
        verifyTrue(testCase, isscalar(value));
    end
    
    for f = {'tagname', 'shortname', 'longname'}
        value = getfield(item, f{1});
        verifyTrue(testCase, ischar(value));
        verifyFalse(testCase, isempty(value));
    end
    
    groups = item.groups;
    verifyTrue(testCase, iscellstr(groups));
    verifyFalse(testCase, isempty(groups));
end
