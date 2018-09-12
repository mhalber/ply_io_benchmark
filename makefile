# A trivial makefile for producing the binaries to run the benchmarks

CC=gcc
CPP=g++
CFLAGS=-O2 -march=native -std=c11
CPPFLAGS=-O2 -march=native -std=c++11

MSH_DIR=../dev/

all:
	$(CC)  -I$(MSH_DIR) -Iturkply/ $(CFLAGS) turkply/ply_io.c turkply_test.c -o bin/turkply_test
	$(CC)  -I$(MSH_DIR) -Irply/ $(CFLAGS) rply/rply.c rply_test.c -o bin/rply_test
	$(CC)  -I$(MSH_DIR) -Ibourkeply/ $(CFLAGS) bourkeply/plyfile.c bourkeply_test.c -o bin/bourkeply_test
	$(CC)  -I$(MSH_DIR) $(CFLAGS)  mshply_test.c -o bin/mshply_test
	$(CPP) -I$(MSH_DIR) -Inanoply/ $(CPPFLAGS) nanoply_test.cpp -o bin/nanoply_test
	$(CPP) -I$(MSH_DIR) -Ihapply/ $(CPPFLAGS) happly_test.cpp -o bin/happly_test
	$(CPP) -I$(MSH_DIR) -Itinyply2.1/ $(CPPFLAGS) tinyply2.1/tinyply.cpp tinyply21_test.cpp -o bin/tinyply21_test
	$(CPP) -I$(MSH_DIR) -Itinyply2.2/ $(CPPFLAGS) tinyply22_test.cpp -o bin/tinyply22_test
	$(CPP) -I$(MSH_DIR) -Iplylib/ $(CPPFLAGS) plylib/plylib.cpp plylib_test.cpp -o bin/plylib_test