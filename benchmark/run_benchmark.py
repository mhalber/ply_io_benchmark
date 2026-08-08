import sys
import argparse
from os import path
from os import listdir
from run_test import run_test
from os.path import isfile, join


def run_benchmark(args):
  meshes_names = [path.join(args.mesh_folder, f) for f in listdir( args.mesh_folder ) if isfile(join(args.mesh_folder, f)) ]
  binaries_names = [path.join(args.binary_folder, b) for b in listdir( args.binary_folder ) if isfile(join(args.binary_folder, b)) and path.splitext(b)[1] == ".exe" ]

  n_tests = len(binaries_names) * len(meshes_names)
  cur_test = 0
  for binary_name in binaries_names:
    for mesh_name in meshes_names:
        print(f"{cur_test:4d}/{n_tests:4d} Testing {binary_name} with mesh {mesh_name}" )
        run_test( binary_name, mesh_name, args.n_tries, args.results_folder )
        cur_test = cur_test+1


def parse_arguments():
  parser = argparse.ArgumentParser(description='Run benchmarks')
  parser.add_argument('mesh_folder', help='Folder where all meshes are stored')
  parser.add_argument('binary_folder', help="Folder where all binaries are stored")
  parser.add_argument('--n_tries', type=int, help="Number of tries we will run for each test", default=10)
  parser.add_argument('--results_folder', help='Folder where to store result file', default="./results/")

  return parser.parse_args()

if __name__ == "__main__":
  args = parse_arguments()
  run_benchmark(args)
