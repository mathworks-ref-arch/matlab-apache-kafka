function R = getRoot(varargin)
    % getRoot - Get root of Kafka tools
    
    % Copyright 2019, The MathWorks Inc.
    here = fileparts(mfilename('fullpath'));
    R = fileparts(fileparts(fileparts(here)));
    
    if nargin > 0
       R = fullfile(R, varargin{:});
    end
       
end
