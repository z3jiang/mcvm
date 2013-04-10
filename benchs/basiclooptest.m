function basiclooptest()

newline = sprintf('\n');

tic;
for i=1:1000000000
end
t_empty_loop = toc;

tic;
for i=1:100000
  for j=1:10000
  end
end
t_nested_loop = toc;

M = rand(5);
for i=1:1000000
  M = M*M;
end
t_matmult_loop = toc;



disp([newline,...
  'TIMING_empty_loop: ', num2str(t_empty_loop), newline,...
  'TIMING_nested_loop: ', num2str(t_nested_loop), newline,...
  'TIMING_matmult_loop: ', num2str(t_matmult_loop), newline,...
  newline]);

end
