# Ply File I/O Benchmark

This repository contains code for comparison of various libraries for input and output of PLY files. [PLY file format](https://en.wikipedia.org/wiki/PLY_(file_format)) has been developed at Stanford University by [Greg Turk](https://www.cc.gatech.edu/~turk/) as a part of the [real-world object digitization project](http://graphics.stanford.edu/data/3Dscanrep/) undertaken by Stanford University in mid-90s.

## Task
The task is to read and write a basic triangle mesh stored in a PLY file into memory. The triangle mesh is represented as a list of vertices and a list of indices into that list, indicating which triplets of vertices make a triangle. 

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

The meshes we are testing were processed to contain only the position attribute and only triangles. As such this task aims to measure the speed at which each library is able to exchange simple triangular mesh data. Each of the meshes is tested using both binary and text(ASCII) formats, as supported by PLY file format. 

### Mesh size vs. number of meshes

This benchmark focuses on rather large meshes (15k - 28 million triangles). It might be reflective of your use-case, where you need to load in heavy meshes often. If your task is to read a lot of smaller .ply files, then this benchmark might not be reflective of your situation.

For an alternative task, where a large number of smaller meshes is parsed, and where meshes might have more varied per-vertex attribute list, please see [Vilya Harvey's ply-parsing-perf](https://github.com/vilya/ply-parsing-perf).  

### Known list size
Given that in our task we process only triangular meshes, it would be good to let the application know this information. 
Some libraries (see below) allow passing the expected size of list properties, leading to non-negligible speed-up in parsing. As such, where applicable, this feature has been enabled.

### Test Models

The table below lists models used for this benchmark, along with the source.

|     Model Name                | N. Vertices |  N. Tris | Source |
|:-----------------------------:|:-----------:|:--------:|:------:|
| suzanne                       |        7958 |    15744 | Blender
| scannet_scene0402_00          |       93834 |   177518 | [Scannet](http://www.scan-net.org/)
| angel                         |      237018 |   474048 | [Large Geometric Models Archvive]
| blade                         |      882954 |  1765388 | [Large Geometric Models Archvive]
| hand                          |      327323 |   654666 | [Large Geometric Models Archvive]
| horse                         |      48485  |    96966 | [Large Geometric Models Archvive]
| armadillo                     |      172974 |   345944 | [Stanford 3D Scaning Repository]
| bunny                         |       35947 |    69451 | [Stanford 3D Scaning Repository]
| dragon                        |      437645 |   871414 | [Stanford 3D Scaning Repository]
| happy_buddha                  |      543652 |  1087716 | [Stanford 3D Scaning Repository]
| lucy                          |    14027872 | 28055742 | [Stanford 3D Scaning Repository]
| xyzrgb_dragon                 |     3609600 |  7219045 | [Stanford 3D Scaning Repository]
| xyzrgb_statuette              |     4999996 | 10000000 | [Stanford 3D Scaning Repository]
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
| [miniply](https://github.com/vilya/miniply) |  [Vilya Harvey](https://github.com/vilya) | c++ | :heavy_check_mark: |
| [nanoply](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/nanoply) | [vcglib](https://github.com/cnr-isti-vclab/vcglib) | c++ | :x: |
| [plylib](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/ply) | [vcglib](https://github.com/cnr-isti-vclab/vcglib)  | c++ |  :x: | PLY reading/writing used by Meshlab(?)
| [tinyply](https://github.com/ddiakopoulos/tinyply) | [Dimitri Diakopoulos](https://github.com/ddiakopoulos) | c++ |  :heavy_check_mark: | This benchmark includes versions 2.1, 2.2 and 2.3 of this library. 

For the usage examples, as well as some additional comments about each of the libraries please check the *_test.c(pp) files.

## Results

Below we present results for parsing PLY files storing data in both ASCII and binary format (little-endian). Times are given in milliseconds. Highlighted numbers indicate the best method in each category.
As noted before, where applicable, a known list size is passed to the library.

The benchmark was compiled using gcc 8.3, with -O3 level of optimization.

To run the test, we run a separate program for each file that attempts to read and write each of the models, and reports time taken to do so. Program for each method is run 10 times and results are averaged. Then we use these averaged results to compute the total average times below.

* Disclaimer: I am the author of msh_ply library. If you see any deficiencies in code for other libraries, don't hesitate to let me know - I hope to make this benchmark as fair as possible. *

### Average Read Times

|Method     |           ASCII    |           Binary   |
|----------:|-------------------:|-------------------:|
|happly     |   25161.368(82.9x) |    2346.514(41.8x) |
|miniply    |  **303.537(1.0x)** |       72.487(1.3x) |
|mshply     |     2207.052(7.3x) |   **56.118(1.0x)** |
|nanoply    |   14205.223(46.8x) |       94.209(1.7x) |
|plylib     |    9268.076(30.5x) |     643.082(11.5x) |
|rply       |    4044.029(13.3x) |      304.144(5.4x) |
|tinyply21  |   16110.044(53.1x) |    1850.766(33.0x) |
|tinyply22  |   13861.185(45.7x) |      430.968(7.7x) |
|tinyply23  |   13845.205(45.6x) |      431.243(7.7x) |
|turkply    |     2424.963(8.0x) |     856.306(15.3x) |


### Average Write Times

|Method     |           ASCII    |           Binary  |
|----------:|-------------------:|------------------:|
|happly     |      7605.653(1.9x) |  1950.618(25.3x) |
|mshply     |  **3965.875(1.0x)** | **76.999(1.0x)** |
|nanoply    |      5315.708(1.3x) |    111.541(1.4x) |
|plylib     |      4704.380(1.2x) |    381.841(5.0x) |
|rply       |      4040.981(1.0x) |    250.154(3.2x) |
|tinyply21  |      5934.833(1.5x) |  1231.946(16.0x) |
|tinyply22  |      5953.786(1.5x) |    682.946(8.9x) |
|tinyply23  |      5522.171(1.4x) |    687.388(8.9x) |
|turkply    |  **3961.067(1.0x)** |    694.024(9.0x) |

Notes:
 - miniply is almost an order of magnitude faster than alternatives for ASCII files while remaining competitive for binary task
 - miniply does not support the writing of ply files.
 - some libraries were modified to include getter to establish whether input is binary or ASCII.
 - In binary mode, happly is unable to convert between __uint__ and __int__. Since some models (bust_of_sappho,bust_of_angelique_dhannetaire ) contain vertex list specified as __uint__, while others use __int__, happly fails to parse the two aforementioned models and would need to be recompiled to support the specific type.
 
 ### Per model I/O times:

|  |  |  |  |  |
|-:|--|--|--|--|
| ASCII | [Read Times Table](assets/ascii_read_times.md) | [Read Times Image](assets/ascii_read.png) | [Write Times Table](assets/ascii_write_times.md) | [Write Times Image](assets/ascii_write.png) |
| Binary | [Read Times Table](assets/little_endian_read_times.md) | [Read Times Image](assets/little_endian_read.png) | [Write Times Table](assets/little_endian_write_times.md) | [Write Times Image](assets/little_endian_write.png) |

Note that the images show the read time on a log scale, since the performance of different libraries is significantly different.

## LOC

Another metric we can use for deciding a library is the ease of use. Why LOC is by no means a perfect metric to measure ease of use, it does reflect how much code one needs to
type to get basic PLY I/O done. Also, note that these numbers report only simple versions of reading function without any error reporting, etc.

|  Library  |   Read LOC  | Write LOC |
|:---------:|:-----------:|:---------:|
| miniply   |    35       |  N/A      |
| mshply    |    29       |   23      |
| nanoply   |    23       |   29      |
| plylib    |    78       |   65*     |
| rply      |    69       |   23      |
| happly    |  **17**     |   26      |
| tinyply   |  **17**     | **10**    |
| turkply   |    52       |   39      |

[Large Geometric Models Archvive]: https://www.cc.gatech.edu/projects/large_models/index.html
[Stanford 3D Scaning Repository]: http://graphics.stanford.edu/data/3Dscanrep/
