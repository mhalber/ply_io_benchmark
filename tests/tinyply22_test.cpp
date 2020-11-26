/*
Author: Maciej Halber
Date: 08/12/18
Description: Benchmarking the read and write capabilities of tinyply by @ddiakopoulos
Task is to get positions and vertex_indices from a ply file that describe
triangular mesh and write that mesh back to hard drive.
License: Public Domain

Compilation:
g++ -I<path_to_msh> -Itinyply/ -O2 -std=c++11 tinyply22_test.cpp -o bin/tinyply22_test

*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define TINYPLY_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_argparse.h"

#include <thread>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include "tinyply22/tinyply.h"
#include "base_test.h"

bool
read_ply( const char* filename, TriMesh* mesh, bool *is_binary)
{
  using namespace tinyply;
  std::ifstream ss(filename, std::ios::binary);
  PlyFile file;
  file.parse_header(ss);
  
  std::shared_ptr<PlyData> verts, faces;
  verts = file.request_properties_from_element("vertex", {"x", "y", "z"});
  faces = file.request_properties_from_element("face", { "vertex_indices" }, 3);

  file.read(ss);
  {
    *is_binary = file.is_binary_file();
    mesh->n_verts  = (int32_t)verts->count;
    mesh->n_faces  = (int32_t)faces->count;
    mesh->vertices = (Vec3f*)malloc( verts->buffer.size_bytes() );
    mesh->faces    = (Tri*)malloc( faces->buffer.size_bytes() );
    std::memcpy(mesh->vertices, verts->buffer.get(), verts->buffer.size_bytes() );
    std::memcpy(mesh->faces, faces->buffer.get(), faces->buffer.size_bytes() );
  }
  return true;
}

void
write_ply( const char* filename, TriMesh* mesh, bool is_binary )
{
  using namespace tinyply;
  std::filebuf fb;
  std::ios_base::openmode flags = std::ios::out;
  if( is_binary ) { flags |= std::ios::binary; }
  fb.open(filename, flags);
  std::ostream outstream(&fb);
  PlyFile out_file;
  out_file.add_properties_to_element("vertex", { "x", "y", "z" }, 
      Type::FLOAT32, mesh->n_verts, reinterpret_cast<uint8_t*>(mesh->vertices), Type::INVALID, 0);
  out_file.add_properties_to_element("face", { "vertex_indices" },
        Type::UINT32, mesh->n_faces, reinterpret_cast<uint8_t*>((int*)&mesh->faces[0].i1), Type::UINT8, 3);
  out_file.write(outstream, is_binary);
  fb.close();
}

int
main( int argc, char** argv )
{
  bool is_able_to_write_ply = true;
  return run_test("tinyply22_test", is_able_to_write_ply, argc, argv );
}