# Ply File I/O Benchmark

This repository contains a simple benchmark comparing various libraries for input/output of ply files.

For this benchmark we use models from the [Stanford 3D Scaning Repository](http://graphics.stanfgunziord.edu/data/3Dscanrep/).
In addition to models provided in repository, we have added cube and subdivided suzanne models as a baseline. We have also added little endian variation of each model as well as ascii variation of armadillo. Variations were created
by exporting meshes using Meshlab.

The reading task is simply opening any of the input file and extracting the minimal __triangle__ mesh
information from it, that is vertex position buffer, face indices buffer, as well as counts of
each element.

The writing task is to write out the read file back as a little endian binary file.

**Model stats:**

|     Model Name    | N. Vertices |  N. Tris |
|:-----------------:|:-----------:|:--------:|
| armadillo         |      172974 |   345944 |
| bunny             |       35947 |    69451 |
| cube              |           8 |       12 |
| dragon            |      437645 |   871414 |
| happy_buddha      |      543652 |  1087716 |
| lucy              |    14027872 | 28055742 |
| suzanne           |        7958 |    15744 |
| xyzrgb_dragon     |     3609600 |  7219045 |
| xyzrgb_statuette  |     4999996 | 10000000 |

## Libraries

Below is a list of libraries used in this benchmark:

- [turkply](https://people.sc.fsu.edu/~jburkardt/c_src/ply_io/ply_io.html) - original PLY library by Greg Turk 

- [bourkeply](http://paulbourke.net/dataformats/ply/) - as far as I can tell it is a Paul Bourke modification of original Turk's code 
 
- [rply](http://w3.impa.br/~diego/software/rply/) - a ply io library by Diego Nehab

- [mshply](https://github.com/mhalber/msh) - a single-header c ply io library by myself

- [nanoply](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/nanoply) - a single-header c++ ply io library taken from vcglib

- [plylib](https://github.com/cnr-isti-vclab/vcglib/tree/master/wrap/ply) - a ply io library taken from vcglib (meshlab ply io(?))

- [tinyply](https://github.com/ddiakopoulos/tinyply) - a ply io library by Dimitri Diakopoulos

For an example of usage of each library, as well as some additional comments about each of the libraries please check the *_test.c(pp)
files.

## Timings

All tests were performed using MSI GS5 laptop with i7-8750H, 16GB of ram and SATA SSD.

All programs were compiled gcc 7.3, with -O3 flag.

Each program was run 10 times for each model and numbers reported below are average timings in milliseconds.
First number is the read time and second number is the write time. Best numbers for model - in bold.

  |           |            armadillo |      armadillo_ascii |         armadillo_le |                bunny |             bunny_le |                 cube |               dragon |            dragon_le |         happy_buddha |      happy_buddha_le |                 lucy |              lucy_le |              suzanne |           suzanne_le |        xyzrgb_dragon |     xyzrgb_dragon_le |     xyzrgb_statuette |  xyzrgb_statuette_le |
  |----------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|---------------------:|
  |bourkeply  |    71.765/39.886     |   259.007/85.883     |    52.704/40.678     |    69.335/21.853     |    20.954/22.856     |     0.135/1.266      |   648.571/220.571    |   133.989/98.990     |   812.189/273.462    |   165.651/122.445    |  4277.068/3143.220   |  4278.043/3158.786   |    11.312/**8.931**  |     2.616/10.352     |  1100.573/810.055    |  1100.025/819.230    |  1524.393/1132.947   |  1524.664/1138.397   |
  |mshply     |  **11.451/11.492**   | **234.082/11.066**   |   **5.964/11.527**   |  **63.710/6.785**    |   **1.629/6.755**    |     0.142/1.249      |  **594.876/25.372**  |  **14.989/26.252**   | **749.520/26.476**   |  **18.572/26.853**   | **910.077/1851.221** | **508.121/1859.988** |**10.183**/9.067      |    **0.373/3.716**   | **234.787/425.155**  | **129.942/423.151**  | **326.463/611.892**  | **179.641/615.597**  |
  |nanoply    |    17.954/611.240    |  1486.275/609.538    |    13.023/609.672    |   498.434/132.417    |     5.077/132.529    |     0.371/1.310      |  3798.315/1623.651   |    28.054/1631.544   |  4736.871/2037.694   |    34.057/2041.885   |  1073.746/52459.071  |   808.868/52532.779  |    68.980/43.704     |     3.247/35.880     |   279.857/13113.569  |   210.259/13123.615  |   385.211/18445.489  |   293.047/18555.088  |
  |plylib     |    82.801/50.227     |   919.134/48.172     |    71.453/51.234     |   239.788/14.388     |    20.353/14.074     |     0.182/2.967      |  2394.411/130.261    |   179.529/131.074    |  3000.221/161.200    |   224.353/156.239    |  5832.562/4067.041   |  5791.145/4035.811   |    43.836/10.156     |     3.319/5.360      |  1498.227/1066.351   |  1489.111/1064.513   |  2075.827/1460.394   |  2063.597/1463.902   |
  |rply       |    40.029/33.632     |   447.053/34.822     |    34.851/35.573     |   132.162/11.336     |     8.372/11.520     |     0.140/1.101      |  1130.187/91.968     |    87.864/92.282     |  1404.508/113.376    |   109.690/115.028    |  3058.011/2762.030   |  2825.575/2779.015   |    20.456/9.227      |     1.668/4.815      |   789.177/712.001    |   727.750/723.889    |  1090.397/1013.406   |  1010.654/1014.885   |
  |tinyply    |   421.095/163.183    |  1779.817/163.930    |   381.058/166.200    |   394.621/37.421     |   101.955/36.745     |     0.539/**1.024**  |  4521.769/416.015    |   967.929/420.629    |  5662.831/522.357    |  1215.175/533.842    | 30577.931/13676.833  | 30798.051/13811.017  |    85.670/17.344     |    19.383/11.819     |  8117.501/3623.996   |  7941.298/3554.399   | 11084.204/4971.100   | 10984.934/4918.387   |
  |turkply    |   103.106/83.552     |   255.376/81.390     |    91.135/82.043     |    68.419/20.990     |    24.620/20.672     |   **0.132**/1.485    |   642.290/214.018    |   228.870/212.002    |   804.640/259.995    |   285.496/258.669    |  7419.417/6685.876   |  7405.573/6543.948   |    11.205/12.545     |     4.239/7.170      |  1917.335/1746.897   |  1888.970/1709.805   |  2664.121/2401.271   |  2704.476/2450.259   |

## LOC

Another metric we can use for deciding a library is ease of use. Why LOC is by no
means a perfect metric to measure ease of use, it does reflec how much code one needs to
type to get basic ply reading done. Also note that these numbers report only simple versions
of read function without any error reporting etc.

|  Library  |   Read LOC  | Write LOC |
|:---------:|:-----------:|:---------:|
| bourkeply |    55       |   33      |
| mshply    |    24       |   24      |
| nanoply   |    23       |   29      |
| plylib    |    78       |   1(65)*  |
| rply      |    69       |   23      |
| tinyply   |  **17**     | **10**    |
| turkply   |    52       |   39      |

\* - plylib does not support writing files explicitly. I have simply copied and simplified **Save** function from
[vcglib source](https://github.com/cnr-isti-vclab/vcglib/blob/master/wrap/io_trimesh/export_ply.h). Number in
parenthesis is the number of lines in that simplified function.

## Notes

1. The speed of msh_ply for triangular meshes comes from the list size hint - if provided program can assume that all faces have constant number of vertices and can just parse all vertex indices data in one go. I'll try to provide benchmark showing how not setting that variables affects the read result.

2. If you have any issues with methodology used in this benchmark or would like to see additional libraries / models tested, please create an issue. I'd be very happy to improve this benchmark and make it more informative
