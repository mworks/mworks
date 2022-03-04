function cN = codec_code2idx(codec, codeNum)
%CODEC_TAG2IDX (MW): given code number, find index in codec
%
%   cN = codec_code2idx(codec, codeNum)
%
% histed 100115: first ver
%
%$Id: codec_code2idx.m 54 2010-01-15 16:06:48Z histed $

names = { codec.tagname };

cNums = [codec.code];
cIx = cNums == codeNum;
nMatches = sum(cIx);
if nMatches == 0
    error('Code %d not found in codec', codeNum);
elseif nMatches > 1
    error('Multiple matches for code in codec: MW bug? %d', codeNum);
else
    cN = find(cIx);
end

