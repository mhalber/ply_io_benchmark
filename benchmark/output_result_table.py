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


def output_result_table(results_folder):
  results_names = [r for r in listdir( results_folder ) if isfile(join(results_folder, r))]
  
  # a bit of roundabout way of ding this but its late and I want to be done
  # Essentially we need to figure out what is the number of methods and method of models
  method_set = set()
  model_set = set()
  for result_name in results_names:
    base_name = result_name.split('.')[0]
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

  method_names = sorted(list(method_set))
  model_names = sorted(list(model_set))
  all_read_times = []
  print( "|%-10s |" % "", end="" )
  for j in range(0, len(model_names)):
    print( "%23s |" % model_names[j], end="" )
  print("")
  print("|", end="")
  print( "-" * 10, end="" )
  print( ":|", end="" )
  for j in range(0, len(model_names)):
    print( "-"*23, end="" )
    print( ":|", end="" )
  print("")
  for i in range(0, len(method_names)):
    method = method_names[i]
    print( "|%-10s |" % method, end="" )
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
      read_times.append(avg_values[0])
      write_times.append(avg_values[1])
    all_read_times.append( read_times )
    for i in range(0, len(read_times)): 
        print("%10.3f / %-10.3f |" % (read_times[i], write_times[i]), end="")
    print("")

  read_times_table = np.array( all_read_times ).transpose()
  n_models = read_times_table.shape[0]
  n_methods = read_times_table.shape[1]

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

  step = (1.0 / n_models) * 0.9
  xmax = 100000.0
  # fig = plt.figure()
  for i in range(n_models):
    color = cmap(i/n_models)
    ax = plt.barh(index-0.35 + i * step, read_times_table[i], step,
            alpha=1.0,
            color=color,
            label=model_names[i])
    rects = ax.patches

    for j in range(len(rects)):
      rect = rects[j]
      read_time = read_times_table[i][j]
      if read_time == 0:
        label = 'N/A'
      else:
        label = '  %-7.2f' % (read_times_table[i][j])
      w = min( rect.get_width(), xmax - 1500 )
      h = rect.get_y() + rect.get_height() * 0.45
      if w == 0:
        w = 0.32
      cur_text = plt.text(w, h, label, ha='left', va='center', color="black", fontproperties=font0)
      # cur_text.set_path_effects([path_effects.Stroke(linewidth=1, foreground='black'), path_effects.Normal()])

  plt.ylabel('Library', fontproperties=font1)
  plt.xlabel('Time (log ms)', fontproperties=font1)
  # plt.title("", fontproperties=font2)
  plt.legend()
  plt.xscale('log')
  plt.xlim(0.3, xmax)
  plt.yticks(np.arange(n_methods), method_names)
  plt.tight_layout()
  plt.show()


def parse_arguments():
  parser = argparse.ArgumentParser(description='Run benchmarks')
  parser.add_argument('results_folder', help='Folder where to store result file', default="./results/")

  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  output_result_table(args.results_folder)