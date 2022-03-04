function code = codec_tag2code(codec, tagnames)
%CODEC_TAG2CODE (MW): given a codec and variable names, return codes
%
%   code = codec_tag2code(codec, tagnames)
%
% histed 100115: first ver
%
%$Id: codec_tag2code.m 54 2010-01-15 16:06:48Z histed $

%code = codec(codec_tag2idx(codec,tagname)).code;
ns = codec_tag2idx(codec, tagnames);
codeList = [codec.code];
code = codeList(ns);

