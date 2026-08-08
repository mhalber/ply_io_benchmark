/*
Author: Maciej Halber
Date: 12/01/24
Description: Benchmarking of the read and write capabilities of plywoot by Ton
van den Heuvel Task is to get positions and vertex_indices from a ply file that
describe triangular mesh and write that mesh back to storage. License: Public
Domain

Compilation:
g++ -I<path_to_msh> -Iplywoot/ -O2 -std=c++17 plywoot_test.cpp -o
bin/plywoot_test

*/
#include "plywoot/plywoot.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh/msh_argparse.h"
#include "msh/msh_std.h"
#include "base_test.h"

bool read_ply(const char *filename, TriMesh *mesh, bool *is_binary) {
  std::ifstream ifs{std::string(filename)};
  if (!ifs) {
    return false;
  }

  std::vector<Tri> faces;
  std::vector<Vec3f> vertices;

  plywoot::IStream plyIn{ifs};
  while (plyIn.hasElement()) {
    const plywoot::PlyElement element{plyIn.element()};
    if (element.name() == "vertex") {
      using VertexLayout =
          plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;
      vertices = plyIn.readElement<Vec3f, VertexLayout>();
    } else if (element.name() == "face") {
      using TriangleLayout =
          plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;
      faces = plyIn.readElement<Tri, TriangleLayout>();
    } else {
      plyIn.skipElement();
    }
  }
  mesh->n_verts = static_cast<int32_t>(vertices.size());
  mesh->vertices = (Vec3f*)malloc(mesh->n_verts * sizeof(Vec3f));
  mesh->n_faces = static_cast<int32_t>(faces.size());
  mesh->faces = (Tri*)malloc(mesh->n_faces * sizeof(Tri));
  memcpy(mesh->vertices, vertices.data(), mesh->n_verts * sizeof(Vec3f));
  memcpy(mesh->faces, faces.data(), mesh->n_faces * sizeof(Tri));
  *is_binary = (plyIn.format() != plywoot::PlyFormat::Ascii);
  return true;
}

void write_ply(const char *filename, TriMesh *mesh, bool is_binary) {
  plywoot::OStream plyos{is_binary ? plywoot::PlyFormat::BinaryLittleEndian
                                   : plywoot::PlyFormat::Ascii};

  const plywoot::PlyProperty x{"x", plywoot::PlyDataType::Float};
  const plywoot::PlyProperty y{"y", plywoot::PlyDataType::Float};
  const plywoot::PlyProperty z{"z", plywoot::PlyDataType::Float};
  const plywoot::PlyElement vertexElement{
      "vertex", static_cast<size_t>(mesh->n_verts), {x, y, z}};

  const plywoot::PlyProperty faceIndices{
      "vertex_indices", plywoot::PlyDataType::Int, plywoot::PlyDataType::UChar};
  const plywoot::PlyElement faceElement{
      "face", static_cast<size_t>(mesh->n_faces), {faceIndices}};

  using TriangleLayout =
      plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;
  using VertexLayout =
      plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;


  std::vector<Vec3f> vertices;
  std::vector<Tri> faces;
  vertices.resize(mesh->n_verts);
  faces.resize(mesh->n_faces);
  memcpy(vertices.data(), mesh->vertices, mesh->n_verts * sizeof(Vec3f));
  memcpy(faces.data(), mesh->faces, mesh->n_faces * sizeof(Tri));

  plyos.add(vertexElement, VertexLayout{vertices});
  plyos.add(faceElement, TriangleLayout{faces});

  std::ofstream ofs{ std::string(filename) };
  plyos.write(ofs);
}

int main(int argc, char **argv) {
  bool is_able_to_write_ply = true;
  return run_test("plywoot_test", is_able_to_write_ply, argc, argv);
}
