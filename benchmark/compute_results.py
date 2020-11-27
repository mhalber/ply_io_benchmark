import sys
import argparse
import re
import numpy as np
import math
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.patheffects as path_effects
from os import path
from os import listdir
from run_test import run_test
from os.path import isfile, join

def print_full_table( model_names, method_names, values, filename=None ):
  with open(filename, "w") as f:
    print( "|%-10s |" % "", end="", file=f )
    for j in range(0, len(model_names)):
      print( "%23s |" % model_names[j], end="", file=f )
    print("", file=f )
    print("|", end="", file=f )
    print( "-" * 10, end="", file=f )
    print( ":|", end="", file=f )
    for j in range(0, len(model_names)):
      print( "-"*23, end="", file=f )
      print( ":|", end="", file=f )
    print("", file=f )

    for i in range(0, len(method_names)):
      method = method_names[i]
      print( "|%-10s |" % method, end="", file=f )
      for j in range(0, len(model_names)):
        value = values[i][j]
        print("%10.3f |" % (value), end="", file=f )
      print("", file=f )
 
def print_average_table( model_names, method_names, values, filename = None ):
  print("TEST")
  with open(filename, "w") as f:
    print( "|%-10s |%-10s |" % ("Method", "Time(ms)"), file=f)
    print( "-" * 10, end="", file=f)
    print( ":|", end="", file=f)
    print( "-" * 10, end="", file=f)
    print( ":|", file=f)

    print("-------------------", len(method_names))
    method_average_times = {}
    for i in range(0, len(method_names)):
      print(method_names[i], len(method_names), i, len(values))
      method = method_names[i]
      cur_values = [v for v in values[i] if v > 0.0]
      average = 1000.0
      if len(cur_values):
        average = sum(cur_values) / len(cur_values)
      method_average_times[method] = average
    
    min_method = min( method_average_times, key=method_average_times.get)
    min_time = method_average_times[min_method]
    for i in range(0, len(method_names)):
      method = method_names[i]
      time = method_average_times[method]
      if method == min_method:
        print("|%-10s | *%10.3f(%2.1fx)*|" % (method, time, time/min_time), file=f)
      else:
        print("|%-10s | %10.3f(%2.1fx)|" % (method, time, time/min_time), file=f)

def create_results_figure( model_names, method_names, values, filename ):
  results_table = np.array( values ).transpose()

  n_models = results_table.shape[0]
  n_methods = results_table.shape[1]

  index = np.arange(n_methods)
  cmap = plt.get_cmap('tab20')
  x = np.linspace(0.0, 1.0, 100)

  font0 = FontProperties()
  font0.set_weight('medium')
  font0.set_size('medium')
  font1 = font0.copy()
  font1.set_size('x-large')
  font2 = font0.copy()
  font2.set_size('xx-large')

  dpi = 70
  step = (1.0 / n_models) * 0.9
  xmax = 100000.0
  plt.figure(figsize=(1000/dpi, 2500/dpi), dpi=dpi)
  for i in range(n_models):
    color = cmap(i/n_models)
    ax = plt.barh(index-0.35 + i * step, results_table[i], step,
            alpha=1.0,
            color=color,
            label=model_names[i])
    rects = ax.patches

    for j in range(len(rects)):
      rect = rects[j]
      read_time = results_table[i][j]
      if read_time == 0:
        label = 'N/A'
      else:
        label = '  %-7.2f' % (results_table[i][j])
      w = min( rect.get_width(), xmax - 1500 )
      h = rect.get_y() + rect.get_height() * 0.45
      if w == 0:
        w = 0.32

  plt.ylabel('Library', fontproperties=font1)
  plt.xlabel('Time (log ms)', fontproperties=font1)
  plt.legend(bbox_to_anchor=(1.1,1.0))
  plt.xscale('log')
  plt.xlim(0.3, xmax)
  plt.yticks(np.arange(n_methods), method_names)
  plt.tight_layout()
  # plt.show()
  plt.savefig(filename, dpi=dpi)

def compute_results(results_folder, output_base_name):
  results_names = [r for r in listdir( results_folder ) if isfile(join(results_folder, r))]
  
  # a bit of roundabout way of doing this but its late and I want to be done
  # Essentially we need to figure out what is the number of methods and method of models
  method_set = set()
  model_set = set()
  for result_name in results_names:
    base_name, ext = result_name.split('.')
    if ext!="txt": 
      continue
    tokens = base_name.split('_')
    method_name = tokens[0]
    model_name = tokens[2]
    if( len(tokens) > 3 ):
      model_name = '_'.join([tokens[2], tokens[3]])
    if( len(tokens) > 4 ):
      model_name = '_'.join([tokens[2], tokens[3], tokens[4]])
    if( len(tokens) > 5 ):
      model_name = '_'.join([tokens[2], tokens[3], tokens[4], tokens[5]])

    method_set.add( method_name )
    model_set.add( model_name )

  # sort the methods and setup variables to store all results
  method_names = sorted(list(method_set))
  model_names = sorted(list(model_set))
  all_read_times = []
  all_write_times = []
  
  # read in the result data
  for i in range(0, len(method_names)):
    method = method_names[i]
    read_times = []
    write_times = []
    for j in range(0, len(model_names)):
      model = model_names[j]
      result_name = ('_').join([method, "test", model]) +".txt"
      result_file = open(path.join(results_folder,result_name), 'r')
      result_lines = [ x.rstrip() for x in result_file.readlines() if len(x) > 1 ]
      if len(result_lines) > 0:
        values = [[float(x) for x in v.split(" ")] for v in result_lines]
      else:
        values = [[0.0, 0.0]] # did not get any results
      result_file.close()
      vals_sum = [sum(x) for x in zip(*values)]
      avg_values = [ x/len(values) for x in vals_sum ]
      read_times.append( avg_values[0] )
      write_times.append( avg_values[1] )
    all_read_times.append( read_times )
    if (method != "miniply") and (method != "microply"):
      all_write_times.append( write_times )
  
  avg_read_table_filename = output_base_name + "_read_avg_table.md"
  full_read_table_filename = output_base_name + "_read_full_table.md"
  read_fig_filename = output_base_name + "_read_fig.png"

  print_average_table( model_names, method_names, all_read_times, avg_read_table_filename )
  print_full_table( model_names, method_names, all_read_times, full_read_table_filename )
  create_results_figure( model_names, method_names, all_read_times, read_fig_filename )
  
  method_names.remove("miniply")
  method_names.remove("microply")


  avg_write_table_filename = output_base_name + "_write_avg_table.md"
  full_write_table_filename = output_base_name + "_write_full_table.md"
  write_fig_filename = output_base_name + "_write_fig.png"

  print_average_table( model_names, method_names, all_write_times, avg_write_table_filename )
  print_full_table( model_names, method_names, all_write_times, full_write_table_filename )
  create_results_figure( model_names, method_names, all_write_times, write_fig_filename )

def parse_arguments():
  parser = argparse.ArgumentParser(description='Run benchmarks')
  parser.add_argument('results_folder', help='Folder where to store result file', default="./results/")
  parser.add_argument('output_base_name', help='Base name of the result files', default=None )

  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  compute_results( args.results_folder, args.output_base_name )