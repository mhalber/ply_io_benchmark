# Ply File I/O Benchmark

This repository contains code for comparison of various libraries for input and output of PLY files. [PLY file format](https://en.wikipedia.org/wiki/PLY_(file_format)) has been developed at Stanford University by [Greg Turk](https://www.cc.gatech.edu/~turk/) as a part of the [real-world object digitization project](http://graphics.stanford.edu/data/3Dscanrep/) undertaken by Stanford University in mid-90s.

## Task
The task is to read and write a basic triangle mesh stored in a PLY. The triangle mesh is represented as a list of vertices and a list of triangles, indicating which triplets of vertices make a triangle. 

The data structure we wish to populate is:
~~~~
typedef struct vec3f
{
  float x,y,z;
} Vec3f;

typedef struct tri
{
  int index1, index2, index3;
} Tri;

typedef struct triangle_mesh
{
  int n_verts;
  int n_faces;
  Vec3f* vertices;
  Tri* faces;
} TriMesh;
~~~~

The meshes we are testing were processed to contain only the position attribute and only triangle faces. As such this task aims to measure the speed at which each library is able to exchange simple triangular mesh data. Each of the meshes is tested using both binary and text(ASCII) formats, as supported by PLY file format. For binary we use little-endian.

### Mesh size vs. number of meshes

This benchmark focuses on rather large meshes (15k - 28 million triangles). The use case this benchmark analyzes is to minimize the time taken to load such large meshes. If your task is to read a lot of smaller .ply files, then this benchmark might not be reflective of your situation.

For an alternative task, where a large number of smaller meshes is parsed, and where meshes might have more varied per-vertex attribute list, please see the excellent [ply-parsing-perf](https://github.com/vilya/ply-parsing-perf) benchmark by Vilya Harvey.  

### Known list size
Given that in our task we process only triangular meshes, it would be good to let the application know this information. 
Some libraries (see below) allow passing the expected size of list properties, leading to non-negligible speed-up in parsing. As such, where applicable, this feature has been enabled.

### Test Models

The table below lists models used for this benchmark, along with the source.

|     Model Name                | N. Vertices |  N. Tris | Source |
|:-----------------------------:|:-----------:|:--------:|:------:|
| suzanne                       |        7958 |    15744 | Blender
| scannet_scene0402_00          |       93834 |   177518 | [Scannet](http://www.scan-net.org/)
| angel                         |      237018 |   474048 | [Large Geometric Models Archvive](https://www.cc.gatech.edu/projects/large_models/)
| blade                         |      882954 |  1765388 | [Large Geometric Models Archvive](https://www.cc.gatech.edu/projects/large_models/)
| hand                          |      327323 |   654666 | [Large Geometric Models Archvive](https://www.cc.gatech.edu/projects/large_models/)
| horse                         |      48485  |    96966 | [Large Geometric Models Archvive](https://www.cc.gatech.edu/projects/large_models/)
| armadillo                     |      172974 |   345944 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| bunny                         |       35947 |    69451 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| dragon                        |      437645 |   871414 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| happy_buddha                  |      543652 |  1087716 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| lucy                          |    14027872 | 28055742 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| xyzrgb_dragon                 |     3609600 |  7219045 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| xyzrgb_statuette              |     4999996 | 10000000 | [Stanford 3D Scaning Repository](http://graphics.stanford.edu/data/3Dscanrep/)
| bust_of_sappho                |      140864 |   281724 | [Thingiverse](https://www.thingiverse.com/thing:14565)
| statue                        |      999517 |  1999038 | [Sketchfab](https://sketchfab.com/3d-models/william-huskisson-statue-ee9ce7c99f2d4b40aa46aaffb02bf21d)
| speeder_bike                  |     1473341 |  2947046 | [Sketchfab](https://sketchfab.com/3d-models/speeder-bike-from-star-wars-galaxys-edge-dcddea22a0674737b4201a025a27a94d)
| armchair                      |       11558 |    23102 | [Sketchfab](https://sketchfab.com/3d-models/lounger-armchair-e9d9d87c32f144e2873765e66814f727)
| bust_of_angelique_dhannetaire |      250000 |   500000 | [Sketchfab](https://sketchfab.com/3d-models/bust-of-angelique-dhannetaire-26c23265310a4e45aaa296d02db83cb2)

## Libraries

Below is a list of libraries used in this benchmark:

| Library | Author| Language | Known list size | Notes | 
|--------:|------------:|---------:|-------------------:|-:|
| [turkply](https://people.sc.fsu.edu/~jburkardt/c_src/ply_io/ply_io.html) | [Greg Turk](https://www.cc.gatech.edu/~turk/) | c |:x: | Original PLY library [link](https://www.cc.gatech.edu/projects/large_models/ply.html)
| [rply](http://w3.impa.br/~diego/software/rply/) | [Diego Nehab](http://w3.impa.br/~diego/index.html) | c | :x: | 
| [msh_ply](https://github.com/mhalber/msh) | [Maciej Halber](https://github.com/mhalber) | c | :heavy_check_mark: |
| [happly](https://github.com/nmwsharp/happly) | [Nicolas Sharp](https://github.com/nmwsharp) | c++ |  :x: |
| [miniply](https://github.com/vilya/miniply) |  [Vilya Harvey](https://github.com/vilya) | c++ | :heavy_check_mark: | Only supports reading PLY files|
| [microply](https://github.com/maluoi/header-libs) | [Nick Klingensmith](https://github.com/maluoi)   | c++ | :x: | Only supports reading ASCII PLY files  | 
| [nanoply](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/nanoply) | [vcglib](https://github.com/cnr-isti-vclab/vcglib) | c++ | :x: |
| [plylib](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/ply) | [vcglib](https://github.com/cnr-isti-vclab/vcglib)  | c++ |  :x: | PLY reading/writing used by Meshlab(?)
| [tinyply](https://github.com/ddiakopoulos/tinyply) | [Dimitri Diakopoulos](https://github.com/ddiakopoulos) | c++ |  :heavy_check_mark: | This benchmark includes versions 2.1, 2.2 and 2.3 of this library. 

For the usage examples, as well as some additional comments about each of the libraries please check the tests/*_test.c(pp) files.

## Results

Below we present results for parsing PLY files storing data in both ASCII and binary format (little-endian). Times are given in milliseconds. Highlighted numbers indicate the best method in each category.
As noted before, where applicable, a known list size is passed to the library.

The benchmark was compiled using MSVC 19.28.29334 with \O2 optimization flag, using AMD Ryzen 3900XT and Samsung 970 EVO PLUS.

To run the test, we run a separate program for each file that attempts to read and write the input file, and reports time taken to do so. Program for each library is run 10 times and the results are averaged. 
The averaged time taken for each model is used to compute the overall average time it took to process all the models.

* Disclaimer: I am the author of msh_ply library. If you see any deficiencies in code for other libraries, don't hesitate to let me know - I hope to make this benchmark as fair as possible. *

### Average Read Times

|Method     | ASCII             |           Binary   |
-----------:|------------------:|-------------------:|
|happly     |  19104.671(75.3x) |     589.435(16.4x) |
|microply   |    1131.752(4.5x) |              N/A   |
|miniply    | **253.671(1.0x)** |   **35.935(1.0x)** |
|mshply     |    2009.957(7.9x) |       40.885(1.1x) |
|nanoply    |   8003.712(31.6x) |      106.312(3.0x) |
|plylib     |   3157.350(12.4x) |      338.514(9.4x) |
|rply       |    1731.580(6.8x) |      327.164(9.1x) |
|tinyply21  |  11583.986(45.7x) |    1844.445(51.3x) |
|tinyply22  |   7561.799(29.8x) |      318.069(8.9x) |
|tinyply23  |   7500.844(29.6x) |      294.265(8.2x) |
|turkply    |    2086.552(8.2x) |      549.367(15.3x)|


### Average Write Times

|Method      |           ASCII    |            Binary  |
|-----------:|-------------------:|-------------------:|
|happly      |    11534.080(3.8x) |    1454.963(19.8x) |
|mshply      |     4178.405(1.4x) |   **73.406(1.0x)** |
|nanoply     |     8772.179(2.9x) |      107.735(1.5x) |
|plylib      | **3045.147(1.0x)** |      315.647(4.3x) |
|rply        |     3966.512(1.3x) |      261.940(3.6x) |
|tinyply21   |     9667.221(3.2x) |    1449.753(19.7x) |
|tinyply22   |     9870.520(3.2x) |      526.407(7.2x) |
|tinyply23   |     9653.622(3.2x) |      560.677(7.6x) |
|turkply     |     4017.640(1.3x) |      624.668(8.5x) |

**Notes**:
 - miniply is the fastest library for reading the ply files. If you're only interested in reading files and use C++, it is a great choice.
 - miniply and microply do not support the writing of ply files.
 - In c, when you need decent read and write performance, msh_ply is a good choice ;). However, it's ASCII mode requires work, so if your models are mostly stored in ASCII, you might want to use other libraries.
 - microply cannot parse binary files, it only supports ASCII formats.
 - Some libraries were modified to include getter to establish whether input is binary or ASCII.
 - In ASCII mode, happly is unable to convert between __uint__ and __int__. Since some models (angel, bust_of_sappho, bust_of_angelique_dhannetaire ) contain vertex list specified as __uint__, while others use __int__, happly fails to parse the two aforementioned models and would need to be recompiled to support the specific type. Here, we simply omit these models when benchmarking happly.
 
 ### Per model I/O times:

|  |  |  |  |  |
|-:|--|--|--|--|
| ASCII | [Read Times Table](assets/ascii_read_full_table.md) | [Read Times Image](assets/ascii_read_fig.png) | [Write Times Table](assets/ascii_write_full_table.md) | [Write Times Image](assets/ascii_write_fig.png) |
| Binary | [Read Times Table](assets/binary_read_full_table.md) | [Read Times Image](assets/binary_read_fig.png) | [Write Times Table](assets/binary_write_full_table.md) | [Write Times Image](assets/binary_write_fig.png) |

Note that the images show the read time on a log scale, since the performance of different libraries is significantly different.

## LOC

Another metric we can use for deciding a library is the ease of use. Why LOC is by no means a perfect metric to measure ease of use, it does reflect how much code one needs to
type to get basic PLY I/O done. Also, note that these numbers report only simple versions of reading function without any error reporting, etc.

|  Library  |   Read LOC  | Write LOC |
|:---------:|:-----------:|:---------:|
| miniply   |    35       |  N/A      |
| microply  |    25       |  N/A      |
| mshply    |    29       |   23      |
| nanoply   |    23       |   29      |
| plylib    |    78       |   65      |
| rply      |    69       |   23      |
| happly    |  **17**     |   26      |
| tinyply   |  **17**     | **10**    |
| turkply   |    52       |   39      |

[Large Geometric Models Archvive]: https://www.cc.gatech.edu/projects/large_models/index.html
[Stanford 3D Scaning Repository]: http://graphics.stanford.edu/data/3Dscanrep/
