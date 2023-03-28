classdef GetCodecsTest < TestBase
    methods (Test)
        function testAll(t)
            codec = getCodecs(t.getFilename());

            t.verifyClass(codec, 'struct');
            t.verifySize(codec, [1 1])
            t.verifyLength(fieldnames(codec), 2);

            t.verifyTrue(isfield(codec, 'time_us'));
            t.verifyInteger(codec.time_us, 0);

            t.verifyTrue(isfield(codec, 'codec'));
            codec = codec.codec;

            t.verifyClass(codec, 'struct');
            t.verifySize(codec, [1 20]);
            t.verifyEqual(sort(fieldnames(codec)), ...
                          sort({'code', 'tagname', 'logging', 'defaultvalue', ...
                                'shortname', 'longname', 'editable', 'nvals', ...
                                'domain', 'viewable', 'persistant', 'groups'})');

            for item = codec
                for f = {'code', 'logging', 'editable', 'nvals', 'domain', ...
                         'viewable', 'persistant'}
                    value = getfield(item, f{1});
                    t.verifyClass(value, 'int64');
                    t.verifySize(value, [1 1]);
                end

                for f = {'tagname', 'shortname', 'longname'}
                    value = getfield(item, f{1});
                    t.verifyClass(value, 'char');
                    t.verifyNotEmpty(value);
                end

                groups = item.groups;
                t.verifyTrue(iscellstr(groups));
                t.verifyNotEmpty(groups);
            end
        end
    end
end
