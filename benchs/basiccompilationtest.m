
helperfunc(0);

tic;
chelper();
t1 = toc;

tic;
chelper();
t2 = toc;

tic;
chelper();
t3 = toc;

newline = sprintf('\n');

disp([newline, ...
  'TIMING_compilation:', num2str(t1-t2), newline, ...
  'TIMING_confirmation:', num2str(t2-t3), newline, ...
  ]);
