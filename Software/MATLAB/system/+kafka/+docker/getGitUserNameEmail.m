function [user, email] = getGitUserNameEmail()
    % getGitUserNameEmail Returns user and email for labels  in docker
    
    % Copyright 2019 The MathWorks, Inc.

    [status, result] = system('git config --get user.name');
    if status == 0
        user = strip(result);
    else
        user = '';
    end
    [status, result] = system('git config --get user.email');
    if status == 0
        email = strip(result);
    else
        email = '';
    end

end
    