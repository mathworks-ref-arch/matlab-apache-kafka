




function dataTT = readInData(fName)

fid = fopen(fName);
raw = fread(fid,inf);
str = char(raw');
fclose(fid);
val = jsondecode(str);

Data = zeros(size(val.results.series.values));
TimeStep = strings(size(val.results.series.values));

for i = 1:size(val.results.series.values)
    Data(i) = val.results.series.values{i,1}{2,1};
    TimeStep(i) = val.results.series.values{i,1}{1,1};
end

TimeStep = datetime(TimeStep, 'InputFormat','yyyy-MM-dd''T''HH:mm:ssXXX','TimeZone','UTC');
dataTT = timetable(TimeStep,Data);

end