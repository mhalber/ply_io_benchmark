/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of rply by Diego Nehab
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Irply/ -O2 -std=c11 rply/rply.c rply_test.c -o bin/rply_test

Comments:
- User needs to generate both requested types and approperiate callbacks to read the data.
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "rply.h"
#include "rplyfile.h"
#include "base_test.h"


static int read_vertex_cb(p_ply_argument argument) {
  void *pdata;
  long idata;
  ply_get_argument_user_data(argument, &pdata, &idata);
  TriMesh* triangle_mesh = (TriMesh*)pdata;
  int val_idx = idata;
  double value = ply_get_argument_value(argument);

  switch(val_idx)
  {
    case 0:
      triangle_mesh->vertices[triangle_mesh->n_verts].x = (float)value; break;
    case 1:
      triangle_mesh->vertices[triangle_mesh->n_verts].y = (float)value; break;
    case 2:
      triangle_mesh->vertices[triangle_mesh->n_verts++].z = (float)value; break;
  }
  return 1;
}

static int read_face_cb(p_ply_argument argument) {
  void *pdata;
  ply_get_argument_user_data(argument, &pdata, NULL);
  TriMesh* triangle_mesh = (TriMesh*)pdata;
  double value = ply_get_argument_value(argument);

  long length, val_idx;
  ply_get_argument_property(argument, NULL, &length, &val_idx);
  switch (val_idx) {
    case 0:
      triangle_mesh->faces[triangle_mesh->n_faces].i1 = (int)value; break;
    case 1:
      triangle_mesh->faces[triangle_mesh->n_faces].i2 = (int)value; break;
    case 2:
      triangle_mesh->faces[triangle_mesh->n_faces++].i3 = (int)value; break;
    default:
        break;
    }
    return 1;
}

bool
read_ply( const char* filename, TriMesh* mesh, bool* is_binary )
{
  p_ply ply = ply_open(filename, NULL, 0, NULL);
  if (!ply) return false;
  if (!ply_read_header(ply)) return false;
  p_ply_element element = NULL;
  while( (element = ply_get_next_element(ply, element)) ) 
  {
    const char* name = NULL;
    long n_instances = -1;
    ply_get_element_info(element, &name, &n_instances);
    if(!strcmp(name, "vertex")) 
    { 
      mesh->n_verts = n_instances; 
      mesh->vertices = (Vec3f*)malloc(mesh->n_verts*sizeof(Vec3f)); 
    }
    if(!strcmp(name, "face"))
    { 
      mesh->n_faces = n_instances; 
      mesh->faces = (Tri*)malloc(mesh->n_faces*sizeof(Tri));
    }
  }
  mesh->n_verts = 0;
  mesh->n_faces = 0;
  ply_set_read_cb(ply, "vertex", "x", read_vertex_cb, mesh, 0);
  ply_set_read_cb(ply, "vertex", "y", read_vertex_cb, mesh, 1);
  ply_set_read_cb(ply, "vertex", "z", read_vertex_cb, mesh, 2);
  ply_set_read_cb(ply, "face", "vertex_indices", read_face_cb, mesh, 0);
  if (!ply_read(ply)) return false;
  *is_binary = (ply_get_storage_mode(ply) != PLY_ASCII); 
  ply_close(ply);
  return true;
}

//NOTE: This is based on example found in wjakob instant-meshes implementation
void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
  e_ply_storage_mode mode = is_binary ? PLY_LITTLE_ENDIAN : PLY_ASCII;
  p_ply ply = ply_create(filename, mode, NULL, 0, NULL);
  if (!ply) return;
  ply_add_element( ply, "vertex", mesh->n_verts );
  ply_add_scalar_property( ply, "x", PLY_FLOAT );
  ply_add_scalar_property( ply, "y", PLY_FLOAT );
  ply_add_scalar_property( ply, "z", PLY_FLOAT );


  ply_add_element( ply, "face", mesh->n_faces );
  ply_add_list_property( ply, "vertex_indices", PLY_UINT8, PLY_INT );

  ply_write_header( ply );

  for( int32_t i = 0; i < mesh->n_verts; ++i )
  {
    ply_write( ply, mesh->vertices[i].x );
    ply_write( ply, mesh->vertices[i].y );
    ply_write( ply, mesh->vertices[i].z );
  }

  for( int32_t i = 0; i < mesh->n_faces; ++i ) 
  {
    ply_write( ply, 3 );
    ply_write( ply, mesh->faces[i].i1 );
    ply_write( ply, mesh->faces[i].i2 );
    ply_write( ply, mesh->faces[i].i3 );
  }

  ply_close(ply);
}


int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = true;
  return run_test("rply_test", is_able_to_write_ply, argc, argv );
}