/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of mshply
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
clang -I<path_to_msh> -Imshply/ -O2 -std=c11 mshply_test.c -o bin/mshply_test

*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "msh/msh_ply.h"

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
  size_t n_verts;
  size_t n_faces;
  Vec3f* vertices;
  Tri* faces;
} TriMesh;

const char* positions_names[] = { "x", "y", "z" };
const char* vertex_indices_names[] = { "vertex_indices" };
msh_ply_desc_t vertex_desc = { .element_name = "vertex",
                               .property_names = positions_names,
                               .num_properties = 3,
                               .data_type = MSH_PLY_FLOAT};
msh_ply_desc_t face_desc = { .element_name = "face",
                             .property_names = vertex_indices_names,
                             .num_properties = 1,
                             .data_type = MSH_PLY_INT32,
                             .list_type = MSH_PLY_UINT8,
                             .list_size_hint = 3 };

bool
read_ply( const char* filename, TriMesh* mesh, bool *is_binary )
{
  vertex_desc.data       = &mesh->vertices;
  vertex_desc.data_count = &mesh->n_verts;
  face_desc.data         = &mesh->faces;
  face_desc.data_count   = &mesh->n_faces;
  msh_ply_t* pf = msh_ply_open( filename, "rb");
  if( pf )
  {
    msh_ply_add_descriptor( pf, &vertex_desc );
    msh_ply_add_descriptor( pf, &face_desc );
    msh_ply_read( pf );
    *is_binary = (pf->format != MSH_PLY_ASCII);
    msh_ply_close( pf );
    return true;
  }
  else
  {
    msh_ply_close( pf );
    return false;
  }
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
  vertex_desc.data       = &mesh->vertices;
  vertex_desc.data_count = &mesh->n_verts;
  face_desc.data         = &mesh->faces;
  face_desc.data_count   = &mesh->n_faces;
  const char* write_format = is_binary ? "wb" : "w";
  msh_ply_t* pf = msh_ply_open( filename, write_format);
  if( pf )
  {
    msh_ply_add_descriptor( pf, &vertex_desc );
    msh_ply_add_descriptor( pf, &face_desc );
    msh_ply_write(pf);
  }
  msh_ply_close(pf);
}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = NULL;
  opts->verbose         = 0;

  msh_ap_init( &parser, "mshply test",
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

  bool is_binary = false;
  msh_cprintf( opts.verbose, "Reading %s ...\n", opts.input_filename );
  t1 = msh_time_now();
  read_ply( opts.input_filename, &mesh, &is_binary );
  t2 = msh_time_now();
  double read_time = msh_time_diff_ms( t2, t1 );
  double write_time = -1.0f;
  if( opts.output_filename )
  {
    t1 = msh_time_now();
    write_ply( opts.output_filename, &mesh, is_binary ); 
    t2 = msh_time_now();
    write_time = msh_time_diff_ms( t2, t1 );
  }
  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );
  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose && opts.output_filename, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d; N. Faces: %d\n", mesh.n_verts, mesh.n_faces );


  return 0;
}