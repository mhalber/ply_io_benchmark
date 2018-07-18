import sys
import argparse
import re
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

    method_set.add( method_name )
    model_set.add( model_name )

  method_names = sorted(list(method_set))
  model_names = sorted(list(model_set))
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
      # values = [float(v.rstrip()) for v in result_file.readlines()]
      values = [[float(x) for x in v.rstrip().split(" ")] for v in result_file.readlines()]
      result_file.close()
      vals_sum = [sum(x) for x in zip(*values)]
      avg_values = [ x/len(values) for x in vals_sum ]
      read_times.append(avg_values[0])
      write_times.append(avg_values[1])
    for i in range(0, len(read_times)): 
        print("%10.3f / %-10.3f |" % (read_times[i], write_times[i]), end="")
    print("")


  # for result_name in results_names:
    
    # result_file = open(path.join(results_folder,result_name), 'r')
    # values = [float(v.rstrip()) for v in result_file.readlines()]
    # result_file.close()

    # base_name = result_name.split('.')[0]
    # avg_value = sum(values)/len(values)
    # # print( base_name, avg_value )
    
    # tokens = re.split('\.|_', result_name)
    # method_name = tokens[0]
    # model_name = tokens[2]
    # if( len(tokens) > 4 ):
    #   model_name = '_'.join([tokens[2], tokens[3]])
    #   key = '_'.join(method_name, model_name)

    
  # n_tests = len(binaries_names) * len(models_names)
  # cur_test = 0;
  # for binary_name in binaries_names:
    # for model_name in models_names:
      # print( "%d/%d Testing %s with model %s" % (cur_test, n_tests, binary_name, model_name) )
      # run_test( binary_name, model_name, args.n_tries, args.results_folder )
      # cur_test = cur_test+1


def parse_arguments():
  parser = argparse.ArgumentParser(description='Run benchmarks')
  parser.add_argument('results_folder', help='Folder where to store result file', default="./results/")

  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  output_result_table(args.results_folder)