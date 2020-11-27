# A simple makefile for compiling the binaries to run the benchmarks

CC=gcc
CPP=g++
CFLAGS=-O3 -march=native -std=c11
CPPFLAGS=-O3 -march=native -std=c++11

MSH_DIR=../../dev
LIB_DIR=plylibs
TESTS_DIR=tests
INCLUDE_DIRS=-I${MSH_DIR} -I${LIB_DIR} -I${TESTS_DIR}

all:
	$(CC)  ${INCLUDE_DIRS} $(CFLAGS)   ${TESTS_DIR}/mshply_test.c -o bin/mshply_test
	$(CC)  ${INCLUDE_DIRS} -I${LIB_DIR}/turkply/ $(CFLAGS) ${LIB_DIR}/turkply/ply_io.c ${TESTS_DIR}/turkply_test.c -o bin/turkply_test
	$(CC)  ${INCLUDE_DIRS} -I${LIB_DIR}/rply/ $(CFLAGS) ${LIB_DIR}/rply/rply.c ${TESTS_DIR}/rply_test.c -o bin/rply_test
	$(CPP) ${INCLUDE_DIRS} $(CPPFLAGS) ${TESTS_DIR}/happly_test.cpp -o bin/happly_test
	$(CPP) ${INCLUDE_DIRS} $(CPPFLAGS) ${TESTS_DIR}/microply_test.cpp -o bin/miniply_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/nanoply/ $(CPPFLAGS) ${TESTS_DIR}/nanoply_test.cpp -o bin/nanoply_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/tinyply21/ $(CPPFLAGS) ${LIB_DIR}/tinyply21/tinyply.cpp ${TESTS_DIR}/tinyply21_test.cpp -o bin/tinyply21_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/tinyply22/ $(CPPFLAGS) ${TESTS_DIR}/tinyply22_test.cpp -o bin/tinyply22_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/tinyply23/ $(CPPFLAGS) ${TESTS_DIR}/tinyply23_test.cpp -o bin/tinyply23_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/plylib/ $(CPPFLAGS) ${LIB_DIR}/plylib/plylib.cpp ${TESTS_DIR}/plylib_test.cpp -o bin/plylib_test
	$(CPP) ${INCLUDE_DIRS} -I${LIB_DIR}/miniply/ $(CPPFLAGS) ${LIB_DIR}/miniply/miniply.cpp ${TESTS_DIR}/miniply_test.cpp -o bin/miniply_test
