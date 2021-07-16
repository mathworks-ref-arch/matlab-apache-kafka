function [out] = rdRoot(varargin)
    % rdRoot Return the root folder of the installation
    
    % (c) 2020 MathWorks, Inc.
    
    out = fileparts(fileparts(fileparts(mfilename('fullpath'))));
    
    if nargin > 0
        out = fullfile(out, varargin{:});
    end
end

