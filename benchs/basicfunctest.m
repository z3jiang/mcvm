function ret = basictest()

% test single call (jit compilation time)
tic;
helperfunc(0);
helperfunc2('abc');
helperfunc2(123);
helperfunc2(123.0);
t_compilation = toc;

% test plain call many times (non recursive)
tic;
for i = 1:5000000
  helperfunc(0);
end
t_simple_5000k = toc;

% recursive calls
tic;
for i = 1:50000
  helperfunc(100);
end
t_simple_50k_r100 = toc;


% call embedded with matrix stuff
ret = rand(100);

tic;

for i = 1:2000
  ret = ret * ret;
  helperfunc(10);
end

t_mixed_2k_r10 = toc;

newline = sprintf('\n');

% display times
disp([newline, ...
  'TIMING_compilation:', num2str(t_compilation), newline, ...
  'TIMING_simple_5000k:', num2str(t_simple_5000k), newline, ...
  'TIMING_simple_50k_r100:', num2str(t_simple_50k_r100), newline, ...
  'TIMING_mixed_r10:', num2str(t_mixed_2k_r10), newline, ...
  ]);
end
