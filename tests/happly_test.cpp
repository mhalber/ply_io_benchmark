/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking of the read and write capabilities of happly by Nicolas Sharp (@nmwsharp)
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to storage.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Ihapply/ -O2 -std=c++11 happly_test.cpp -o bin/happly_test

*/
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include "happly/happly.h"

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "base_test.h"

bool
read_ply( const char* filename, TriMesh* mesh, bool *is_binary )
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
    Vec3f vertex = { x_pos[i], y_pos[i], z_pos[i] };
    mesh->vertices[i] = vertex;
  }
  for( int i = 0; i < mesh->n_faces; ++i )
  {
    Tri triangle = { face_ind[i][0], face_ind[i][1], face_ind[i][2] };
    mesh->faces[i] = triangle;
  }
  *is_binary = (plyIn.getInputDataFormat() != happly::DataFormat::ASCII);
  return true;
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
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

  happly::DataFormat output_format = is_binary ? happly::DataFormat::Binary : happly::DataFormat::ASCII;

  plyOut.write(filename, output_format);
}

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = true;
  return run_test("happly_test", is_able_to_write_ply, argc, argv );
}