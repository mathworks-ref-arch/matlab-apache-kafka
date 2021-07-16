function rtwString = getRTWStringFromCellArray(fields)
    % getRTWStringFromCellArray
    %  Convert a cell array of strings to a string suitable as a TLC array
    %

    % Copyright 2019, The MathWorks Inc.

    if isempty(fields)
        rtwString = '[]';
    else
        q = cellfun(@quoteIt, fields, 'Uni', false);
        q2 = join(q, ',');
        rtwString = ['[', q2{1}, ']'];
    end
end

function s = quoteIt(s)
    s = ['"', s, '"'];
end