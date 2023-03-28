classdef TypeConversionTest < TestBase
    methods (Test)
        function testUndefined(t)
            u = t.getData('undefined');
            t.verifyEqual(u, []);
        end

        function testBoolean(t)
            % Neither Scarab nor SQLite has a boolean type, so booleans come out
            % as integers
            t.verifyInteger(t.getData('bool_true'), 1);
            t.verifyInteger(t.getData('bool_false'), 0);
        end

        function testInteger(t)
            t.verifyInteger(t.getData('int_zero'), 0);
            t.verifyInteger(t.getData('int_pos'), 1);
            t.verifyInteger(t.getData('int_neg'), -2);
            t.verifyInteger(t.getData('int_max'), intmax('int64'));
            t.verifyInteger(t.getData('int_min'), intmin('int64'));
        end

        function testFloat(t)
            t.verifyFloat(t.getData('float_zero'), 0.0);
            t.verifyFloat(t.getData('float_pos'), 1.0);
            t.verifyFloat(t.getData('float_neg'), -2.5);
        end

        function testFloatInf(t)
            f = t.getData('float_inf');
            t.verifyClass(f, 'double');
            t.verifyTrue(isinf(f));
        end

        function testFloatNan(t)
            f = t.getData('float_nan');
            t.verifyClass(f, 'double');
            if endsWith(t.getFilename(), '.mwk2')
                % SQLite stores NaN as NULL
                t.verifyEqual(f, []);
            else
                t.verifyTrue(isnan(f));
            end
        end

        function testString(t)
            t.verifyString(t.getData('str_empty'), '');
            t.verifyString(t.getData('str_text'), 'foo bar');
            b = t.getData('str_binary');
            t.verifyClass(b, 'uint8');
            t.verifyEqual(b, uint8([102, 111, 111, 0, 98, 97, 114]));
        end

        function testList(t)
            t.verifyList(t.getData('list_empty'), {});
            t.verifyList(t.getData('list_simple'), {int64(1)});
            t.verifyList(t.getData('list_complex'), ...
                         {int64(1), 2.0, 'three', struct('four', int64(4))});
            t.verifyList(t.getData('list_nested'), ...
                         {int64(1), ...
                          {int64(2), {int64(3), {int64(4), int64(5)}}}});
        end

        function testDictionary(t)
            t.verifyDict(t.getData('dict_empty'), struct());
            t.verifyDict(t.getData('dict_simple'), struct('a', int64(1)));
            t.verifyDict(t.getData('dict_complex'), ...
                         struct('one', int64(1), 'Two', 2.0, 'ThReE_3', '3.0'));
        end

        function testDictionaryAsMap(t)
            t.verifyDictAsMap(t.getData('dict_int_key'), ...
                             {int64(1)}, {'a'});
            t.verifyDictAsMap(t.getData('dict_empty_str_key'), ...
                              containers.Map({''}, {int64(1)}).keys(), ...
                              {int64(1)})
            t.verifyDictAsMap(t.getData('dict_str_key_starts_with_non_alpha'), ...
                              {'1a'}, {int64(1)});
            t.verifyDictAsMap(t.getData('dict_str_key_contains_non_alphanum'), ...
                              {'foo.1'}, {int64(1)});
        end

        function testDictionaryAsFallbackStruct(t)
            t.verifyDictAsFallbackStruct(t.getData('dict_mixed_keys'), ...
                                         {int64(1), 'two'}, {'a', int64(2)});
            t.verifyDictAsFallbackStruct(t.getData('dict_binary_str_key'), ...
                                         {uint8([97, 0, 98])}, {int64(1)});
        end
    end
end
