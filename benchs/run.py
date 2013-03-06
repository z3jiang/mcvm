#!/usr/bin/python


# author: Sam J
# usage: ./run matlabfile

# the matlab file should print to stdout its timings in the format of 
# TIMING_name: seconds
# where name can be anything without spaces, and seconds can be a float
# (leading and trailing white spaces are trimmed)
# 
# an additional end to end time is always recorded

#############################

import sys
import os
import subprocess as sp
import numpy
import time

if len(sys.argv) < 2:
  raise Exception("Please specify test file to execute")

iterations = 5

testfile = sys.argv[1]


#mcvm doesn't like the .m extension
if testfile.endswith('.m'):
  testfile = testfile[:-2]

mcvm = '../mcvm'
mcvmstock = '../mcvmstock'

if not os.access(mcvm, os.X_OK):
  raise Exception("Cannot find executable " + mcvm)

if not os.access(mcvmstock, os.X_OK):
  raise Exception("Cannot find executable " + mcvmstock)

timings = {}
timings_stock = {}

def bench(executable, testfile, ret):
  print("Executing test " + testfile + " using " + executable)

  t_end2end = time.time()
  p = sp.Popen([executable, testfile], stdout=sp.PIPE) # don't capture stdout

  while True:
    for line in p.stdout.readlines():
      line = line.strip()

      colon = line.find(':')

      if colon < 0 or (not line.startswith('TIMING_')):
        continue

      name = line[ len('TIMING_') : colon ];
      t = float(line[colon+1:])

      if name in ret:
        ret[name].append(t)
      else:
        ret[name] = [t]

    if p.poll() is not None:
      break

  # add end to end time
  t_end2end = time.time() - t_end2end
  key = 'end_to_end'
  if key in ret:
    ret[key].append(t_end2end)
  else:
    ret[key] = [t_end2end]

  print("Found timings: " + str(ret.keys()))


def stats(lst):
  return [min(lst), max(lst), numpy.mean(lst), numpy.std(lst)]

"""
does t1 - t2 element wise, with percentage
"""
def diff(t1, t2):
  assert len(t1) == len(t2)
  ret = []
  for i in range(len(t1)):
    delta = t1[i] - t2[i]
    percentage = 0 if t2[i] == 0 else delta / t2[i]
    ret.append( [delta, percentage] )
 
  return ret

        
for i in range(iterations):
  print("Iteration: " + str(i+1))
  bench(mcvm, testfile, timings)
  bench(mcvmstock, testfile, timings_stock)

names = timings.keys()
names.sort()
for name in names:
  stock = stats(timings_stock[name])
  new = stats(timings[name])

  comp = diff(new, stock)

  print("Timing: " + name)

  print("  Stock: mean %.2f\t\tmin %.2f\t\tmax %.2f\t\tstd %.2f" % 
      (stock[2], stock[0], stock[1], stock[3]))
  print("  New:   mean %.2f\t\tmin %.2f\t\tmax %.2f\t\tstd %.2f" % 
      (new[2], new[0], new[1], new[3]))

  print("  Diff:  mean %.2f(%.2f%%)\tmin %.2f(%.2f%%)\
      \tmax %.2f(%.2f%%)\t\tstd %.2f(%.2f%%)" % 
      (comp[2][0], 100*comp[2][1], 
       comp[0][0], 100*comp[0][1], 
       comp[1][0], 100*comp[1][1], 
       comp[3][0], 100*comp[3][1]))

