/*
Author: Maciej Halber
Data: 04/09/18
Description: 
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Ihapply/ -O2 -std=c++11 happly_test.cpp -o bin/happly_test

Notes:
*/
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include "happly.h"

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

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
  happly::PLYData plyIn( filename, false );
  std::vector<float> x_pos   = plyIn.getElement("vertex").getProperty<float>("x");
  std::vector<float> y_pos   = plyIn.getElement("vertex").getProperty<float>("y");
  std::vector<float> z_pos   = plyIn.getElement("vertex").getProperty<float>("z");
  std::vector<std::vector<int>> face_ind = plyIn.getElement("face").getListProperty<int>("vertex_indices");

  mesh->n_verts  = (int)x_pos.size();
  mesh->n_faces  = (int)face_ind.size();
  mesh->vertices = (Vec3f*)malloc( mesh->n_verts * sizeof(Vec3f));
  mesh->faces    = (Tri*)malloc( mesh->n_faces * sizeof(Tri));
  for( int i = 0; i < mesh->n_verts; ++i )
  {
    mesh->vertices[i] = (Vec3f){ x_pos[i], y_pos[i], z_pos[i] };
  }
  for( int i = 0; i < mesh->n_faces; ++i )
  {
     mesh->faces[i] = (Tri){ face_ind[i][0], face_ind[i][1], face_ind[i][2] };
  }
}

void
write_ply( const char* filename, const TriMesh* mesh )
{
  // Create an empty object
  happly::PLYData plyOut;

  plyOut.addElement("vertex", mesh->n_verts);
  plyOut.addElement("face", mesh->n_faces);

  std::vector<float> xPos(mesh->n_verts);
  std::vector<float> yPos(mesh->n_verts);
  std::vector<float> zPos(mesh->n_verts);
  for( int i = 0; i < mesh->n_verts; i++) 
  {
    xPos[i] = mesh->vertices[i].x;
    yPos[i] = mesh->vertices[i].y;
    zPos[i] = mesh->vertices[i].z;
  }

  plyOut.getElement("vertex").addProperty<float>("x", xPos);
  plyOut.getElement("vertex").addProperty<float>("y", yPos);
  plyOut.getElement("vertex").addProperty<float>("z", zPos);

  std::vector<std::vector<int>> intInds;
  for( int i = 0; i < mesh->n_faces; ++i )
  {
    std::vector<int> thisInds(3);
    thisInds[0] = mesh->faces[i].i1;
    thisInds[1] = mesh->faces[i].i2;
    thisInds[2] = mesh->faces[i].i3;
    intInds.push_back(thisInds);
  }

  // Store
  plyOut.getElement("face").addListProperty<int>("vertex_indices", intInds);

  plyOut.write(filename, happly::DataFormat::Binary);
}

int parse_arguments( int argc, char**argv, Opts* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->output_filename = (char*)"test.ply";
  opts->verbose         = 0;

  msh_ap_init( &parser, "nanoply test",
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
                              mesh.faces[test_idx].i1,
                              mesh.faces[test_idx].i2,
                              mesh.faces[test_idx].i3 );

  return 0;
}