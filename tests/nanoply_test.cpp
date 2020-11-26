/*
Author: Maciej Halber
Date: 04/09/18
Description: Benchmarking the read and write capabilities of nanoply from vcglib @cnr-isti-vclab
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Inanoply/ -O2 -std=c++11 nanoply_test.cpp -o bin/nanoply_test

Notes: 
- There appears to be no way to do non-triangle meshes?
- I had modify name property list variable for vertiex indices to say vertex_indices instead of vertex_index
   Line 249 of original nanoply. Not sure if I am using library wrong..
*/

#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include "nanoply/nanoply.hpp"

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
  // Get file info
  nanoply::Info info( filename );
  *is_binary = info.binary;

  // Prepare the mesh contents
  mesh->n_verts = (int32_t)info.GetVertexCount();
  mesh->n_faces = (int32_t)info.GetFaceCount();
  mesh->vertices = (Vec3f*)malloc( mesh->n_verts * sizeof(Vec3f) );
  mesh->faces    = (Tri*)malloc( mesh->n_faces * sizeof(Tri) );

  // Create the vertex and face properties descriptor
  nanoply::ElementDescriptor vertex(nanoply::NNP_VERTEX_ELEM);
  nanoply::ElementDescriptor face(nanoply::NNP_FACE_ELEM);
  vertex.dataDescriptor.push_back(new nanoply::DataDescriptor<Vec3f, 3, float>(
                                    nanoply::NNP_PXYZ, (void*)mesh->vertices));
  face.dataDescriptor.push_back(new nanoply::DataDescriptor<Tri, 3, int>(
                                 nanoply::NNP_FACE_VERTEX_LIST, (void*)mesh->faces));
  
  // Create the mesh descriptor
  std::vector<nanoply::ElementDescriptor*> meshDescr;
  meshDescr.push_back(&vertex);
  meshDescr.push_back(&face);

  // Open the file and save the element data according the relative element descriptor
  OpenModel(info, meshDescr);

  // Cleanup
  for (int i = 0; i < vertex.dataDescriptor.size(); i++)
    delete vertex.dataDescriptor[i];
  for (int i = 0; i < face.dataDescriptor.size(); i++)
    delete face.dataDescriptor[i];
  return true;
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
 //Create the vector of vertex properties to save in the file
  std::vector<nanoply::PlyProperty> vertexProp;
  vertexProp.push_back(nanoply::PlyProperty(nanoply::NNP_FLOAT32, nanoply::NNP_PXYZ));

  //Create the vector of face properties to save in the file
  std::vector<nanoply::PlyProperty> faceProp;
  faceProp.push_back(nanoply::PlyProperty(nanoply::NNP_LIST_UINT8_UINT32, nanoply::NNP_FACE_VERTEX_LIST));

  //Create the PlyElement
  nanoply::PlyElement vertexElem(nanoply::NNP_VERTEX_ELEM, vertexProp, mesh->n_verts);
  nanoply::PlyElement faceElem(nanoply::NNP_FACE_ELEM, faceProp, mesh->n_faces);

  //Create the Info object with the data to save in the header
  nanoply::Info infoSave;
  infoSave.filename = filename;
  infoSave.binary = is_binary;
  infoSave.AddPlyElement(vertexElem);
  infoSave.AddPlyElement(faceElem);

  //Create the vertex properties descriptor (what ply property and where the data is stored)
  nanoply::ElementDescriptor vertex(nanoply::NNP_VERTEX_ELEM);
  if (mesh->n_verts > 0)
  {
    vertex.dataDescriptor.push_back(new nanoply::DataDescriptor<Vec3f, 3, float>(nanoply::NNP_PXYZ, (mesh->vertices)));
  }

  //Create the face properties descriptor (what ply property and where the data is stored)
  nanoply::ElementDescriptor face(nanoply::NNP_FACE_ELEM);
  if (mesh->n_faces > 0)
  {
    face.dataDescriptor.push_back(new nanoply::DataDescriptor<Tri, 3, int>(nanoply::NNP_FACE_VERTEX_LIST, (mesh->faces)));
  }
  //Create the mesh descriptor
  std::vector<nanoply::ElementDescriptor*> meshDescr;
  meshDescr.push_back(&vertex);
  meshDescr.push_back(&face);

  //Save the file
  bool result = nanoply::SaveModel(infoSave.filename, meshDescr, infoSave);

  for (int i = 0; i < vertex.dataDescriptor.size(); i++)
    delete vertex.dataDescriptor[i];
  for (int i = 0; i < face.dataDescriptor.size(); i++)
    delete face.dataDescriptor[i];
}

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = true;
  return run_test("nanoly_test", is_able_to_write_ply, argc, argv );
}