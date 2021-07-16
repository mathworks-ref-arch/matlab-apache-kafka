classdef StringWriter < handle
    % StringWriter - Helper class for writing to files or temporary strings
    
    % Copyright 2019, The MathWorks Inc.
    properties(SetAccess=private)
        FH = []
        FileName
        String
        IsTemp
        TabLength = 0;
        Tab = '';
        PS = '';
    end
    properties
        RunIndent = false
        IncludeProtection = false
    end
    
    methods
        function this = StringWriter( fileName, varargin )
            if nargin > 0
                this.FileName = fileName;
                this.IsTemp = false;
            else
                this.FileName = tempname;
                this.IsTemp = true;
            end
            N = length(varargin);
            if rem(N,2) ~= 0
                error('Additional arguments must come in pairs');
            end
            for k=1:2:N
                f = varargin{k};
                v = varargin{k+1};
                this.(f) = v;
            end

            this.FH = fopen( this.FileName, 'w' );
            if this.FH < 0
                error('Couldn''t open %s for writing\n', this.FileName );
            end
            if this.IncludeProtection
                this.PS = getProtectString(this);
                this.pf('#ifndef %s\n#define %s\n\n', this.PS, this.PS);
            end
        end
        
        function str = getString( this )
            if isempty( this.String )
                closeFile( this );
            end
            str = this.String;
        end
        
        function insertFile(this, fileName)
           this.pf('%s', fileread(fileName));            
        end
        
        function fprintf( this, varargin )
            if isempty( this.FH )
                error('This StringWriter has already been closed.' );
            end
            fprintf( this.FH, varargin{:} );
        end
        
        function pf( this, varargin )
            % Short-hand for printf
            if isempty( this.FH )
                error('This StringWriter has already been closed.' );
            end
            fprintf( this.FH, varargin{:} );
        end
        
        function nl( this )
           this.pf( '\n' );
           this.tab();
        end
        
        function tab( this, num )
            if nargin == 1
                this.pf( '%s', this.Tab );
            else
                this.TabLength = this.TabLength + num;
                if this.TabLength < 0
                    this.TabLength = 0;
                end
                this.Tab = repmat( ' ', 1, this.TabLength );
            end
        end
        
        function delete(this)
            closeFile( this );
        end
        
        
    end
    methods (Access = private)
        function closeFile( this )
            if ~isempty( this.FH )
                if this.IncludeProtection
                    this.pf('#endif /* %s */\n\n', this.PS);
                end
                fclose( this.FH );
                this.FH = [];
                if this.RunIndent
                   c_indent(this.FileName); 
                end
                this.String = fileread( this.FileName );
                if this.IsTemp
                    delete( this.FileName );
                end
                this.FileName = '';
            end
        end
        
        function str = getProtectString(this)
           str = sprintf('_%s_', upper(strrep(this.FileName, '.', '_')));
        end
    end
end
