import sys
import argparse
from os import path
from os import listdir
from run_test import run_test
from os.path import isfile, join


def run_benchmark(args):
  models_names = [path.join(args.model_folder, f) for f in listdir( args.model_folder ) if isfile(join(args.model_folder, f))]
  binaries_names = [path.join(args.binary_folder, b) for b in args.binary_names ]
  n_tests = len(binaries_names) * len(models_names)
  cur_test = 0;
  for binary_name in binaries_names:
    for model_name in models_names:
      print( "%d/%d Testing %s with model %s" % (cur_test, n_tests, binary_name, model_name) )
      run_test( binary_name, model_name, args.n_tries, args.results_folder )
      cur_test = cur_test+1


def parse_arguments():
  parser = argparse.ArgumentParser(description='Run benchmarks')
  parser.add_argument('model_folder', help='Folder where all models are stored')
  parser.add_argument('binary_folder', help="Folder where all binaries are stored")
  parser.add_argument('binary_names', help='Names of binaries we will be executing', nargs="*")
  parser.add_argument('--n_tries', type=int, help="Number of tries we will run for each test", default=10)
  parser.add_argument('--results_folder', help='Folder where to store result file', default="./results/")

  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  run_benchmark(args)