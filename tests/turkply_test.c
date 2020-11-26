/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of turkply, which is the original code for ply file parsing by Greg Turk, with name changed to differentiate it from other lubraries..
Setting is simple - getting positions and vertex_indices from a ply file that describes
triangular mesh.
License: Public Domain

Compilation:
gcc -I<path_to_msh> -Iturkply/ -O2 -std=c11 turkply/ply_io.c turkply_test.c -o bin/turkply_test

Notes:
- turkply does not seem deal with the endianness correctly
- turkply used drand48() which is posix function, not available on Windows.
  Replaced it with (float)rand()/(float)(RAND_MAX)
- turkply forces a specific representation of the mesh's face, hence I cannot reuse base_test.h
- turkply generates a lot of warnings on MSVC, I suppresed them, but the library should be ideally fixed.
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
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

PlyFile *in_ply;
PlyFile *out_ply;

void
read_ply_file( const char* filename, TriMesh* mesh, bool *is_binary )
{
  int elem_count;
  char *elem_name;
  int i;
  int j;

  FILE *fp = fopen(filename, "rb");
  if (!fp) { return; }
  in_ply = read_ply (fp );
  
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
  *is_binary = (in_ply->file_type != PLY_ASCII);
  close_ply (in_ply);
}

void
write_ply_file( char* filename, const TriMesh* mesh, bool is_binary )
{
  int i;
  int num_elem_types;
  FILE* output = NULL;
  output = fopen( filename, "wb" );
  if( output ==NULL ) return;
  int file_type = is_binary ? PLY_BINARY_LE: PLY_ASCII;

  out_ply = write_ply( output, 2, elem_names, file_type );
/*
  Describe what properties go into the vertex elements.
*/
  describe_element_ply( out_ply, "vertex", mesh->n_verts );
  describe_property_ply( out_ply, &vert_props[0] );
  describe_property_ply( out_ply, &vert_props[1] );
  describe_property_ply( out_ply, &vert_props[2] );

/*
  Describe what properties go into the face elements.
*/
  describe_element_ply( out_ply, "face", mesh->n_faces );
  describe_property_ply( out_ply, &face_props[0] );

  header_complete_ply( out_ply );
/*
  Set up and write the vertex elements.
*/
  put_element_setup_ply( out_ply, "vertex" );
  for (i = 0; i < mesh->n_verts; i++)
  {
    put_element_ply( out_ply, (void *) &mesh->vertices[i] );
  }
/*
  Set up and write the face elements.
*/
  put_element_setup_ply( out_ply, "face" );
  for (i = 0; i < mesh->n_faces; i++)
  {
    put_element_ply( out_ply, (void *) &mesh->faces[i] );
  }

  close_ply( out_ply );
  fclose(output);
}


int 
parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = NULL;
  opts->verbose         = 0;

  msh_ap_init( &parser, "turkply test",
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

  bool is_binary = false;
  msh_cprintf( opts.verbose, "Reading %s ...\n", opts.input_filename );
  t1 = msh_time_now();
  read_ply_file( opts.input_filename, &mesh, &is_binary );
  t2 = msh_time_now();
  double read_time = msh_time_diff_ms( t2, t1 );

  double write_time = -1.0f;
  if( opts.output_filename )
  {
    t1 = msh_time_now();
    write_ply_file( opts.output_filename, &mesh, is_binary ); 
    t2 = msh_time_now();
    write_time = msh_time_diff_ms( t2, t1 );
  }
  
  msh_cprintf( !opts.verbose, "%f %f\n", read_time, write_time );

  msh_cprintf( opts.verbose, "Reading done in %lf ms\n", read_time );
  msh_cprintf( opts.verbose && opts.output_filename, "Writing done in %lf ms\n", write_time );
  msh_cprintf( opts.verbose, "N. Verts : %d; N. Faces: %d\n", mesh.n_verts, mesh.n_faces );

  free_ply( out_ply );
  free_ply( in_ply );

  return 0;
}