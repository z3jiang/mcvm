
function ret = recursiontest()

tic;
for i = 1:2000000000
  helperfunc(1);
end
t_r1 = toc;

tic;
for i = 1:250000000
  helperfunc(10);
end
t_r10 = toc;

tic;
for i = 1:25000000
  helperfunc(100);
end
t_r100 = toc;

tic;
for i = 1:2500000
  helperfunc(1000);
end
t_r1000 = toc;

tic;
for i = 1:250000
  helperfunc(10000);
end
t_r10000 = toc;

newline = sprintf('\n');

% display times
disp([newline, ...
  'TIMING_r1:', num2str(t_r1), newline, ...
  'TIMING_r10:', num2str(t_r10), newline, ...
  'TIMING_r100:', num2str(t_r100), newline, ...
  'TIMING_r1000:', num2str(t_r1000), newline, ...
  'TIMING_r10000:', num2str(t_r10000), newline, ...
  ]);
end
