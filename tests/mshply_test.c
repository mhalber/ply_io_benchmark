/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of mshply
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Imshply/ -O2 -std=c11 mshply_test.c -o bin/mshply_test

*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "msh/msh_ply.h"
#include "base_test.h"

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

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = true;
  return run_test("mshply_test", is_able_to_write_ply, argc, argv );
}
