import subprocess
import sys
from os import path
import argparse


def run_test( binary_name, model_name, n_tries, results_folder ):
    binary_base_name = path.splitext( path.basename( binary_name ) )[0]
    model_base_name = path.splitext( path.basename( model_name ) )[0]
    result_filename = results_folder + binary_base_name + "_" + model_base_name + ".txt"

    result_file = open(result_filename, "w")
    
    for _ in range(0, n_tries):
      proc = subprocess.Popen( [binary_name, model_name, "-o", "test.ply"], stdout=subprocess.PIPE )
      output = proc.stdout.read().decode('utf-8').rstrip()
      result_file.write( "%s\n" % output )

    result_file.close()
    

def parse_arguments():
  parser = argparse.ArgumentParser(description='Run single test')
  parser.add_argument('binary_name', help='Path to binary we will be executing')
  parser.add_argument('model_name', help='Name of ply file we wish to test on')
  parser.add_argument('--n_tries', type=int, help='Number of tries we wish to record', default=10)
  parser.add_argument('--results_folder', help='Folder where to store result file', default="./results/")
  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  run_test( args.binary_name, args.model_name, args.n_tries, args.results_folder )