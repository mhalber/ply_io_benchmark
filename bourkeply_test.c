/*
Author: Maciej Halber
Data: 04/09/18
Description: Benchmarking the read and write capabilities of bourkeply, a ply file
reader found on Paul Bourke's site. It also lists greg turk as an author but features different
api that turkply.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Ibourkeply/ -O3 -std=c11 bourkeply/plyfile.c bourkeply_test.c -o bin/bourkeply_test

Notes:
- bourke's ply seem to have issues with writing binary files? (Bug on my side? - possible, but ascii output is fine)
- bourke's ply seems to enforce the internal mesh structures' face layout to follow the one of ply format
- Is each face list a separate malloc(?)
- Needed to edit the plyfile.c slightly:
  - Added type specifier to ply_get_element(...)
  - made my_alloc(...) non-static to follow declaration in ply.h
  - made filename passed to read function take a const char* instead of char*
  - Auxiliary data idea could be utilized to get the tris.
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include "ply.h"

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

typedef struct face
{
  unsigned char count;
  int* vertex_indices;
} Face;

typedef struct triangle_mesh
{
  int n_verts;
  int n_faces;
  Vec3f* vertices;
  Face* faces;
} TriMesh;

char *elem_names[] = { 
  "vertex", "face"
};

PlyProperty vert_props[] = {
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vec3f,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vec3f,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vec3f,z), 0, 0, 0, 0},
};


PlyProperty face_props[] = {
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,vertex_indices), 1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,count)},
};

void
read_ply( const char* filename, TriMesh* mesh)
{
  int i,j,k;
  PlyFile *ply;
  int nelems;
  char **elist;
  int file_type;
  float version;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  
  ply = ply_open_for_reading(filename, &nelems, &elist, &file_type, &version);
  for (i = 0; i < nelems; i++) {
    elem_name = elist[i];
    plist = ply_get_element_description( ply, elem_name, &num_elems, &nprops );
    if( !strcmp("vertex", elem_name) ) {

      mesh->n_verts  = num_elems; 
      mesh->vertices = (Vec3f*)malloc( sizeof(Vec3f) * num_elems );
      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);

      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *)&mesh->vertices[j]);
      }
    }

    if( !strcmp("face", elem_name) ) {

      mesh->n_faces = num_elems; 
      mesh->faces   = (Face*)malloc( sizeof(Face) * num_elems);
      ply_get_property (ply, elem_name, &face_props[0]);

      for (j = 0; j < num_elems; j++) {
        ply_get_element (ply, (void *)&mesh->faces[j]);
      }
    }
  }

  ply_close (ply);
}

void
write_ply( char* filename, const TriMesh* mesh )
{
  int i,j;
  PlyFile *ply;
  int nelems;
  char **elist;
  int file_type;
  float version;

  /* open either a binary PLY file for writing */
  ply = ply_open_for_writing(filename, 2, elem_names, PLY_BINARY_LE, &version);

  /* describe what properties go into the vertex and face elements */
  ply_element_count (ply, "vertex", mesh->n_verts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);

  ply_element_count (ply, "face", mesh->n_faces);
  ply_describe_property (ply, "face", &face_props[0]);

  /* we have described exactly what we will put in the file, so */
  /* we are now done with the header info */
  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < mesh->n_verts; i++)
    ply_put_element (ply, (void *) &mesh->vertices[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < mesh->n_faces; i++)
    ply_put_element (ply, (void *) &mesh->faces[i]);

  /* close the PLY file */
  ply_close (ply);
}

int 
parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = "test.ply";
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
  float read_time = msh_time_diff_ms( t2, t1 );
  t1 = msh_time_now();
  write_ply( opts.output_filename, &mesh );
  t2 = msh_time_now();
  float write_time = msh_time_diff_ms( t2, t1 );
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
                              mesh.faces[test_idx].vertex_indices[0],
                              mesh.faces[test_idx].vertex_indices[1],
                              mesh.faces[test_idx].vertex_indices[2] );

  return 0;
}