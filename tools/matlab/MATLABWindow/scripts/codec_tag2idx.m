function codeN = codec_tag2idx(codec, tagnames)
%CODEC_TAG2IDX (MW): given variable names, return codec struct indices
%
%   function codeN = codec_tag2idx(codec, tagnames)
%
% histed 100115: first ver
%
%$Id: codec_tag2idx.m 54 2010-01-15 16:06:48Z histed $

names = { codec.tagname };

if ischar(tagnames), tagnames = {tagnames}; end
nTagnames = length(tagnames);

codeN = repmat(NaN, [1 nTagnames]);
for iN = 1:nTagnames
    tTag = tagnames{iN};
    
    cIx = strcmp(names, tTag);
    nMatches = sum(cIx);
    if nMatches == 0
        error('Tagname %s not found in codec', tTag);
    elseif nMatches > 1
        error('Multiple matches for tagname in codec: MW bug? %s', tTag);
    else
        codeN(iN) = find(cIx);
    end
end

