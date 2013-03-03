function ret = basictest()

% test single call
helperfunc(0);

tic;
ret = rand(100);

for i = 1:1000
  ret = ret * ret;
  % test call in loops
  helperfunc(2);
end

t = toc;
disp(['time ', num2str(t)]);

end
