CC=gcc
CPP=g++
CFLAGS=-O3 -std=c11
CPPFLAGS=-O3 -std=c++11
MSH_DIR=../
all:
	$(CC) -I$(MSH_DIR) -Iturkply/ $(CFLAGS) turkply/ply_io.c turkply_test.c -o bin/turkply_test
	$(CC) -I$(MSH_DIR) -Irply/ $(CFLAGS) rply/rply.c rply_test.c -o bin/rply_test
	$(CC) -I$(MSH_DIR) -Ibourkeply/ $(CFLAGS) bourkeply/plyfile.c bourkeply_test.c -o bin/bourkeply_test
	$(CC) -I$(MSH_DIR) $(CFLAGS)  mshply_test.c -o bin/mshply_test
	$(CPP) -I$(MSH_DIR) -Inanoply/ $(CPPFLAGS) nanoply_test.cpp -o bin/nanoply_test
	$(CPP) -I$(MSH_DIR) -Itinyply/ $(CPPFLAGS) tinyply/tinyply.cpp tinyply_test.cpp -o bin/tinyply_test
	$(CPP) -I$(MSH_DIR) -Iplylib/ $(CPPFLAGS) plylib/plylib.cpp plylib_test.cpp -o bin/plylib_test