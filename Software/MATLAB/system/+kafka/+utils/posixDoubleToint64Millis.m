function millis64 = posixDoubleToint64Millis(pTime)
    % posixDoubleToint64Millis Convert posixtime to milliseconds version
    % 
    % Example:
    % >> d1=datetime('now', 'TimeZone', 'utc')
    % d1 =
    %   datetime
    %    10-Jan-2020 08:07:57
    % >> p1 = posixtime(d1)
    % p1 =
    %           1578643677.37003
    % >> p1millis = kafka.utils.posixDoubleToint64Millis(p1)
    % p1millis =
    %   int64
    %    1578643677370
   
    % Copyright 2020, The MathWorks Inc.

   seconds = floor(pTime);
   millis = floor(1000*(pTime - seconds));
   millis64 = int64(seconds)*1000 + int64(millis);
    
end