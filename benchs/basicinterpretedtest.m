function basicinterpretedtest()

% statements without ; are for sure interpreted

helperfunc(0)
helperfunc(1)

tic;
for i=1:1000
  helperfunc(1)

  for j=1:500
    helperfunc(2)
  end
end
t = toc;

newline = sprintf('\n');
disp([newline, 'TIMING_nestedloop: ', num2str(t), newline]);

end
