/*
Author: Maciej Halber
Data: 04/09/18
Description: Benchmarking the read and write capabilities of rply by Diego Nehab
Setting is simple - getting positions and vertex_indices from a ply file that describes
triangular mesh.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Irply/ -O2 -std=c11 rply/rply.c rply_test.c -o bin/rply_test

Notes:
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

static int read_vertex_cb(p_ply_argument argument) {
  void *pdata;
  long idata;
  ply_get_argument_user_data(argument, &pdata, &idata);
  TriMesh* triangle_mesh = (TriMesh*)pdata;
  int val_idx = idata;
  float value = ply_get_argument_value(argument);

  switch(val_idx)
  {
    case 0:
      triangle_mesh->vertices[triangle_mesh->n_verts].x = value; break;
    case 1:
      triangle_mesh->vertices[triangle_mesh->n_verts].y = value; break;
    case 2:
      triangle_mesh->vertices[triangle_mesh->n_verts++].z = value; break;
  }
  return 1;
}

static int read_face_cb(p_ply_argument argument) {
  void *pdata;
  ply_get_argument_user_data(argument, &pdata, NULL);
  TriMesh* triangle_mesh = (TriMesh*)pdata;
  float value = ply_get_argument_value(argument);

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

void
read_ply( const char* filename, TriMesh* mesh)
{
  p_ply ply = ply_open(filename, NULL, 0, NULL);
  if (!ply) return;
  if (!ply_read_header(ply)) return;
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
  if (!ply_read(ply)) return;
  ply_close(ply);
  return;
}


//NOTE: This is based on example found in wjakob instant-meshes implementation
void
write_ply( const char* filename, const TriMesh* mesh )
{
  p_ply ply = ply_create(filename, PLY_LITTLE_ENDIAN, NULL, 0, NULL);
  if (!ply) return;
  ply_add_element( ply, "vertex", mesh->n_verts );
  ply_add_scalar_property( ply, "x", PLY_FLOAT );
  ply_add_scalar_property( ply, "y", PLY_FLOAT );
  ply_add_scalar_property( ply, "z", PLY_FLOAT );


  ply_add_element( ply, "face", mesh->n_faces );
  ply_add_list_property( ply, "vertex_indices", PLY_UINT8, PLY_INT );

  ply_write_header( ply );

  for( uint32_t i = 0; i < mesh->n_verts; ++i )
  {
    ply_write( ply, mesh->vertices[i].x );
    ply_write( ply, mesh->vertices[i].y );
    ply_write( ply, mesh->vertices[i].z );
  }

  for( uint32_t i = 0; i < mesh->n_faces; ++i ) 
  {
    ply_write( ply, 3 );
    ply_write( ply, mesh->faces[i].i1 );
    ply_write( ply, mesh->faces[i].i2 );
    ply_write( ply, mesh->faces[i].i3 );
  }

  ply_close(ply);
}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = "test.ply";
  opts->verbose         = 0;

  msh_ap_init( &parser, "rply test",
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
  msh_cprintf( opts.verbose, "Writing done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose, "N. Verts : %d ;N. Faces: %d \n", 
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