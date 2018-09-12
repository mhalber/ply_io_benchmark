/*
Author: Maciej Halber
Data: 04/09/18
Description: Benchmarking the read and write capabilities of tinyply by @ddiakopoulos
Setting is simple - getting positions and vertex_indices from a ply file that describes
triangular mesh.
License: Public Domain

Compilation:
g++ -I../dev -Itinyply2.1/ -O2 -std=c++11 tinyply2.1/tinyply.cpp tinyply21_test.cpp -o bin/tinyply21_test

Notes:
- No way to do non-triangle meshes?
*/

#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include "tinyply.h"



typedef struct options
{
  bool verbose;
  char* input_filename;
  char* output_filename;
} Opts;

typedef struct vec3f
{
  float x,y,z;
} Vec3f;

typedef struct tri
{
  int i1, i2, i3;
} Tri;

typedef struct triangle_mesh
{
  int n_verts;
  int n_faces;
  Vec3f* vertices;
  Tri* faces;
} TriMesh;

void
read_ply( const char* filename, TriMesh* mesh)
{
  using namespace tinyply;
  std::ifstream ss(filename, std::ios::binary);
  PlyFile file;
  file.parse_header(ss);
  
  std::shared_ptr<PlyData> verts, faces;
  verts = file.request_properties_from_element("vertex", {"x", "y", "z"});
  faces = file.request_properties_from_element("face", { "vertex_indices" });

  file.read(ss);
  {
    mesh->n_verts  = verts->count;
    mesh->n_faces  = faces->count;
    mesh->vertices = (Vec3f*)malloc( verts->buffer.size_bytes() );
    mesh->faces    = (Tri*)malloc( faces->buffer.size_bytes() );
    std::memcpy(mesh->vertices, verts->buffer.get(), verts->buffer.size_bytes() );
    std::memcpy(mesh->faces, faces->buffer.get(), faces->buffer.size_bytes() );
  }
}

void
write_ply( const char* filename, const TriMesh* mesh )
{
  using namespace tinyply;
  std::filebuf fb;
  fb.open(filename, std::ios::out | std::ios::binary);
  std::ostream outstream(&fb);
  PlyFile cube_file;
  cube_file.add_properties_to_element("vertex", { "x", "y", "z" }, 
      Type::FLOAT32, mesh->n_verts, reinterpret_cast<uint8_t*>(mesh->vertices), Type::INVALID, 0);
  cube_file.add_properties_to_element("face", { "vertex_indices" },
        Type::UINT32, mesh->n_faces, reinterpret_cast<uint8_t*>((int*)&mesh->faces[0].i1), Type::UINT8, 3);
  cube_file.write(outstream, true);
  fb.close();
}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = (char*)"test.ply";
  opts->verbose         = 0;

  msh_ap_init( &parser, "bourkeply test",
               "This program simply reads and writes an input ply file" );
  msh_ap_add_string_argument( &parser, "input_filename", NULL, "Name of a ply file to read",
                           &opts->input_filename, 1 );
  msh_ap_add_string_argument( &parser, "--output_filename", "-o", "Name of a ply file to write",
                          &opts->output_filename, 1 );
  msh_ap_add_bool_argument( &parser, "--verbose", "-v", "Print verbose information",
                        &opts->verbose, 0 );

  if( !msh_ap_parse(&parser, argc, argv) )
  {
    msh_ap_display_help( &parser );
    return 1;
  }
  return 0;
}

int
main( int argc, char** argv )
{
  uint64_t t1, t2;
  Opts opts = {0};
  TriMesh mesh = {0};

  int parse_err = parse_arguments( argc, argv, &opts );
  if( parse_err ) { return 1; }

  msh_cprintf(opts.verbose, "Reading %s ...\n", opts.input_filename );
  t1 = msh_time_now();
  read_ply( opts.input_filename, &mesh );
  t2 = msh_time_now();
  float read_time = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  t1 = msh_time_now();
  write_ply( opts.output_filename, &mesh );
  t2 = msh_time_now();
  float write_time = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );
  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d; N. Faces: %d \n", 
               mesh.n_verts, mesh.n_faces );
  int test_idx = 1024;
  msh_cprintf( opts.verbose, "Vert no. %d : %f %f %f\n",
                              test_idx, 
                              mesh.vertices[test_idx].x,
                              mesh.vertices[test_idx].y,
                              mesh.vertices[test_idx].z );
  msh_cprintf( opts.verbose, "Face no. %d : %d %d %d\n", 
                              test_idx, 
                              mesh.faces[test_idx].i1,
                              mesh.faces[test_idx].i2,
                              mesh.faces[test_idx].i3 );

  return 0;
}