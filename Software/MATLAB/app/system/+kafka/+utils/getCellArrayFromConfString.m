function C = getCellArrayFromConfString(str)
    % getCellArrayFromConfString
    %  Convert a text string with key=value pairs into a cell array
    %

    % Copyright 2019, The MathWorks Inc.

    C = {};
    if isempty(str)
        return;
    else
        lines = textscan(str, '%s', 'delimiter', '\n');
        lines = lines{1};
        for k=1:length(lines)
            L = strip(lines{k});
            if isempty(L) || L(1)=='#'
                continue;
            end
            
            R = regexp(L, '([^=]+)\s*=\s*(.+)\s*', 'tokens', 'once');
            if ~isempty(R)
                k = strip(R{1});
                v = strip(R{2});
                C = [C; {k;v}]; %#ok<AGROW>
            end
        end
    end
    
end

