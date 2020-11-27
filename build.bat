@echo off

rem A simple batch file for compiling the binaries to run the benchmarks

rem dev_dir is a directory which stores the msh folder. You can get msh from: https://github.com/mhalber/msh

set opts_c=-D_CRT_SECURE_NO_WARNINGS -nologo -FC -EHa- -GR- -O2 -W3
set opts_cpp= -D_CRT_SECURE_NO_WARNINGS -nologo -FC -EHsc -O2 -W3
set dev_dir="..\..\..\dev"
set lib_dir="..\plylibs"
set tests_dir="..\tests"
set include_dirs=-I%dev_dir% -I%lib_dir% -I%tests_dir%
pushd bin

cl %opts_cpp% %include_dirs% %tests_dir%\happly_test.cpp -Fe"happly_test.exe"
cl %opts_cpp% %include_dirs% %tests_dir%\microply_test.cpp -Fe"microply_test.exe"
cl %opts_cpp% %include_dirs% %tests_dir%\nanoply_test.cpp -Fe"nanoply_test.exe"
cl %opts_cpp% %include_dirs% %tests_dir%\tinyply22_test.cpp -Fe"tinyply22_test.exe"
cl %opts_cpp% %include_dirs% %tests_dir%\tinyply23_test.cpp -Fe"tinyply23_test.exe"

cl %opts_cpp% %include_dirs% -I%lib_dir%\miniply %lib_dir%\miniply\miniply.cpp %tests_dir%\miniply_test.cpp -Fe"miniply_test.exe"
cl %opts_cpp% %include_dirs% -I%lib_dir%\plylib %lib_dir%\plylib\plylib.cpp %tests_dir%\plylib_test.cpp -Fe"plylib_test.exe"
cl %opts_cpp% %include_dirs% -I%lib_dir%\tinyply21 %lib_dir%\tinyply21\tinyply.cpp %tests_dir%\tinyply21_test.cpp -Fe"tinyply21_test.exe"

cl %opts_c% -wd4267 -wd4244 -wd4101 -wd4996 %include_dirs% -I%lib_dir%\turkply %lib_dir%\turkply\ply_io.c %tests_dir%\turkply_test.c -Fe"turkply_test.exe"
cl %opts_c% %include_dirs% -I%lib_dir%\rply %lib_dir%\rply\rply.c %tests_dir%\rply_test.c -Fe"rply_test.exe"
cl %opts_c% %include_dirs% %tests_dir%\mshply_test.c -Fe"mshply_test.exe"
popd