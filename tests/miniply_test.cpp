/*
Author: Maciej Halber
Date: 02/02/2020
Description: Benchmarking the read and write capabilities of miniply by Vilya Harvey (@vilyah)
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Iminiply/ -O2 -std=c++11 miniply/miniply.cpp miniply_test.cpp -o bin/miniply_test

Notes:
- miniply is super fast in general, but it especially excels in parsing ascii files.
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include "miniply/miniply.h"
#include "base_test.h"

// This is modified code from Vilya Harvey's ply-parsing-perf
bool
read_ply( const char* filename, TriMesh* mesh, bool *is_binary )
{
  int32_t verts_per_face = 3;

  miniply::PLYReader reader(filename);
  if (!reader.valid()) {
    return false;
  }

  std::vector<uint32_t> listIdxs;
  miniply::PLYElement* facesElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
  if (facesElem != nullptr)
  {
    listIdxs.resize(verts_per_face);
    facesElem->convert_list_to_fixed_size( facesElem->find_property("vertex_indices"), verts_per_face, listIdxs.data());
  }

  bool gotVerts = false;
  bool gotFaces = false;
  while (reader.has_element() && (!gotVerts || !gotFaces))
  {
    if (!gotVerts && reader.element_is(miniply::kPLYVertexElement))
    {
      if (!reader.load_element()) { break; }
      uint32_t propIdxs[3];
      if (!reader.find_pos(propIdxs)) { break; }
      mesh->n_verts = reader.num_rows();
      mesh->vertices = new Vec3f[mesh->n_verts];
      reader.extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, mesh->vertices );
      gotVerts = true;
    }
    else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement))
    {
      if (!reader.load_element()) { break; }
      mesh->n_faces = reader.num_rows();
      mesh->faces = new Tri[mesh->n_faces];
      reader.extract_properties(listIdxs.data(), verts_per_face, miniply::PLYPropertyType::Int, mesh->faces );
      gotFaces = true;
    }
    reader.next_element();
  }

  *is_binary = (reader.file_type() != miniply::PLYFileType::ASCII );

  return true;
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
}

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = false;
  return run_test("miniply_test", is_able_to_write_ply, argc, argv );
}