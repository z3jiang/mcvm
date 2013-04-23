function basicinterpretedtest()

% statements without ; are for sure interpreted

tic;
for i=1:2000000
  1
end
t = toc;

newline = sprintf('\n');
disp([newline, 'TIMING_nestedloop: ', num2str(t), newline]);

end
