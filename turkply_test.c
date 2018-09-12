/*
Author: Maciej Halber
Data: 04/09/18
Description: Benchmarking the read and write capabilities of turkply, a ply file
reader found on jburkardt's site. It lists greg turk as an author.
Setting is simple - getting positions and vertex_indices from a ply file that describes
triangular mesh.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Iturkply/ -O2 -std=c11 turkply/ply_io.c turkply_test.c -o bin/turkply_test

Notes:
- turk's ply annoingly has function called conflicted with my functions.
- turk's ply does not deal with the endianness correctly
- turk's ply used drand48() which is posix function, not available on Windows.
  Replaced it with (float)rand()/(float)(RAND_MAX)

NOTE: The examples are based on Greg Turk code from 94
*/

#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include "ply_io.h"

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
  {"x", Float32, Float32, offsetof(Vec3f,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(Vec3f,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(Vec3f,z), 0, 0, 0, 0},
};


PlyProperty face_props[] = {
  {"vertex_indices", Int32, Int32, offsetof(Face,vertex_indices), 1, Uint8, Uint8, offsetof(Face,count)},
};

void
read_ply_file( const char* filename, TriMesh* mesh)
{
  int elem_count;
  char *elem_name;
  int i;
  int j;

  FILE *fp = fopen(filename, "rb");
  if (!fp) { return; }
  PlyFile *in_ply = read_ply (fp );
  
  for( i = 0; i < in_ply->num_elem_types; i++) 
  {
    elem_name = setup_element_read_ply(in_ply, i, &elem_count);
    
    if( !strcmp("vertex", elem_name) ) 
    {
      mesh->vertices = (Vec3f*)malloc( sizeof(Vec3f) * elem_count);
      mesh->n_verts  = elem_count;

      setup_property_ply( in_ply, &vert_props[0] );
      setup_property_ply( in_ply, &vert_props[1] );
      setup_property_ply( in_ply, &vert_props[2] );

      for (j = 0; j < elem_count; j++) {
        get_element_ply( in_ply, (void *)&mesh->vertices[j] );
      }
    }

    if (!strcmp("face", elem_name)) 
    {
      mesh->n_faces = elem_count; 
      mesh->faces   = (Face*)malloc( sizeof(Face) * elem_count );

      setup_property_ply( in_ply, &face_props[0] );

      for (j = 0; j < elem_count; j++) {
        get_element_ply( in_ply, (void *)&mesh->faces[j] );
      }
    }
  }

  close_ply (in_ply);
  free_ply (in_ply);
}

void
write_ply_file( char* filename, const TriMesh* mesh )
{
  int i;
  int num_elem_types;
  PlyFile *ply;
  FILE* output = NULL;
  output = fopen( filename, "w" );
  if( output ==NULL ) return;

  ply = write_ply( output, 2, elem_names, PLY_ASCII );
/*
  Describe what properties go into the vertex elements.
*/
  describe_element_ply( ply, "vertex", mesh->n_verts );
  describe_property_ply( ply, &vert_props[0] );
  describe_property_ply( ply, &vert_props[1] );
  describe_property_ply( ply, &vert_props[2] );

/*
  Describe what properties go into the face elements.
*/
  describe_element_ply( ply, "face", mesh->n_faces );
  describe_property_ply( ply, &face_props[0] );

  header_complete_ply( ply );
/*
  Set up and write the vertex elements.
*/
  put_element_setup_ply( ply, "vertex" );
  for (i = 0; i < mesh->n_verts; i++)
  {
    put_element_ply( ply, (void *) &mesh->vertices[i] );
  }
/*
  Set up and write the face elements.
*/
  put_element_setup_ply( ply, "face" );
  for (i = 0; i < mesh->n_faces; i++)
  {
    put_element_ply( ply, (void *) &mesh->faces[i] );
  }

  close_ply( ply );
  free_ply( ply );
  fclose(output);
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
  read_ply_file( opts.input_filename, &mesh );
  t2 = msh_time_now();
  float read_time = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  t1 = msh_time_now();
  write_ply_file( opts.output_filename, &mesh );
  t2 = msh_time_now();
  float write_time = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );

  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );
  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d;N. Faces: %d \n", 
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