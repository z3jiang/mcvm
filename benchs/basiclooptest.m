function basiclooptest()

newline = sprintf('\n');

tic;
for i=1:1000000000
end
t_empty_loop = toc;

M = rand(5);
for i=1:1000000
  M = M*M;
end
t_matmult_loop = toc;



disp([newline,...
  'TIMING_empty_loop: ', num2str(t_empty_loop), newline,...
  'TIMING_matmult_loop: ', num2str(t_matmult_loop), newline,...
  newline]);

end
