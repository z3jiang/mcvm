function helperfunc(val)

if val > 0
  % test recursive, self
  helperfunc(val-1);
end

fprintf('.');


end
