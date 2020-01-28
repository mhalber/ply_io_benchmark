/*
Author: Maciej Halber
Data: 08/12/18
Description: Benchmarking the read and write capabilities of tinyply by @ddiakopoulos
Setting is simple - getting positions and vertex_indices from a ply file that describes
triangular mesh.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Iminiply/ -O2 -std=c++11 miniply/miniply.cpp miniply_test.cpp -o bin/miniply_test

Notes:
- No way to do non-triangle meshes?
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include "miniply.h"



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


// This is modified code from Vilya Harper's ply-parsing-perf
void
read_ply( const char* filename, TriMesh* mesh)
{
  int32_t verts_per_face = 3;

  miniply::PLYReader reader(filename);
  if (!reader.valid()) {
    return;
  }

  std::vector<uint32_t> listIdxs;
  miniply::PLYElement* facesElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
  if (facesElem != nullptr)
  {
    listIdxs.resize(verts_per_face);
    facesElem->convert_list_to_fixed_size( facesElem->find_property("vertex_indices"), verts_per_face, listIdxs.data());
  }

  bool gotVerts = false;
  bool gotFaces = false;
  while (reader.has_element() && (!gotVerts || !gotFaces))
  {
    if (!gotVerts && reader.element_is(miniply::kPLYVertexElement))
    {
      if (!reader.load_element()) { break; }
      uint32_t propIdxs[3];
      if (!reader.find_pos(propIdxs)) { break; }
      mesh->n_verts = reader.num_rows();
      mesh->vertices = new Vec3f[mesh->n_verts];
      reader.extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, mesh->vertices );
      gotVerts = true;
    }
    else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement))
    {
      if (!reader.load_element()) { break; }
      mesh->n_faces = reader.num_rows();
      mesh->faces = new Tri[mesh->n_faces];
      reader.extract_properties(listIdxs.data(), verts_per_face, miniply::PLYPropertyType::Int, mesh->faces );
      gotFaces = true;
    }
    reader.next_element();
  }
}

void
write_ply( const char* filename, const TriMesh* mesh )
{

}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = (char*)"test.ply";
  opts->verbose         = 0;

  msh_ap_init( &parser, "miniply test",
               "This program simply reads and writes an input ply file" );
  msh_ap_add_string_argument( &parser, "input_filename", NULL, "Name of a ply file to read",
                           &opts->input_filename, 1 );
  msh_ap_add_string_argument( &parser, "--output_filename", "-o", "Name of a ply file to write",
                          &opts->output_filename, 1 );
  msh_ap_add_bool_argument( &parser, "--verbose", "-v", "Print verbose information",
                        &opts->verbose, 0 );

  if( !msh_ap_parse(&parser, argc, argv) )
  {
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
  float read_time = msh_time_diff_ms( t2, t1 );
 
  float write_time = -1.0f;
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